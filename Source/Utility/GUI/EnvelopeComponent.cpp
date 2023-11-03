#include "EnvelopeComponent.h"

EnvelopeComponent::EnvelopeComponent ()
{
    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::background_png, BinaryData::background_pngSize);
    offset = anchorSize / 2;
}

void EnvelopeComponent::connectAnchorNeighbors (int index)
{
    if (index == 0)
    {
        // this was a prepend
        if (index < (envelopeAnchors.size () - 1))
        {
            envelopeAnchors[index]->anchorToRight = envelopeAnchors[index + 1];
            envelopeAnchors[index + 1]->anchorToLeft = envelopeAnchors[index];
        }
    }
    else if ((index < 0) || (index > (envelopeAnchors.size () - 2)))
    {
        // this was an append
        index = envelopeAnchors.size () - 1;

        if (index > 0)
        {
            envelopeAnchors[index]->anchorToLeft = envelopeAnchors[index - 1];
            envelopeAnchors[index - 1]->anchorToRight = envelopeAnchors[index];
        }
    }
    else
    {
        // this is an insert
        if (index > 0)
        {
            envelopeAnchors[index]->anchorToLeft = envelopeAnchors[index - 1];
            envelopeAnchors[index - 1]->anchorToRight = envelopeAnchors[index];
        }
        if (index < (envelopeAnchors.size () - 1))
        {
            envelopeAnchors[index]->anchorToRight = envelopeAnchors[index + 1];
            envelopeAnchors[index + 1]->anchorToLeft = envelopeAnchors[index];
        }
    }
}

// inserts an anchor at the position indicated by the index, with -1 indicating the end (ie. an append)
void EnvelopeComponent::addAnchor (EnvelopeAnchorComponent* anchor, int index)
{
    anchor->setMaxTime (envelopeDisplayAreaWidth);
    anchor->setMaxAmplitude (envelopeDisplayAreaHeight);
    envelopeAnchors.insert (index, anchor);
    connectAnchorNeighbors (index);

    repaint ();
}

void EnvelopeComponent::disconnectAnchorNeighbors (int index)
{
    if (index == 0)
    {
        // deleted the first anchor
        if (envelopeAnchors.size () > 0)
            envelopeAnchors[0]->anchorToLeft = nullptr;
    }
    else if (index == envelopeAnchors.size ())
    {
        // deleted the last anchor
        envelopeAnchors[envelopeAnchors.size () - 1]->anchorToRight = nullptr;
    }
    else
    {
        envelopeAnchors[index]->anchorToLeft = envelopeAnchors[index - 1];
        envelopeAnchors[index - 1]->anchorToRight = envelopeAnchors[index];
    }
}

// deletes an anchor at the position indicated by the index, causing all anchors to the right of the index to shift one left
EnvelopeAnchorComponent* EnvelopeComponent::deleteAnchor (int index)
{
    if ((index < 0) || (index >= envelopeAnchors.size ()))
        return nullptr;

    const auto anchorThatWasDeleted = envelopeAnchors[index];
    envelopeAnchors.remove (index);
    disconnectAnchorNeighbors (index);

    repaint ();
    return anchorThatWasDeleted;
}

void EnvelopeComponent::initAnchors (long numberOfEnvelopeAnchors)
{
    for (auto curEnvelopAnchor { 0 }; curEnvelopAnchor < numberOfEnvelopeAnchors; ++curEnvelopAnchor)
        addAnchor (new EnvelopeAnchorComponent);
}

void EnvelopeComponent::setAnchorXPercent (long anchorIndex, double xPercent)
{
    if (anchorIndex < envelopeAnchors.size ())
    {
        envelopeAnchors[anchorIndex]->xAxis.percentage = xPercent;
        envelopeAnchors[anchorIndex]->recalculateX (borderWidth, envelopeDisplayAreaWidth);
    }
}

void EnvelopeComponent::setAnchorYPercent (long anchorIndex, double yPercent)
{
    if (anchorIndex < envelopeAnchors.size ())
    {
        envelopeAnchors[anchorIndex]->yAxis.percentage = yPercent;
        envelopeAnchors[anchorIndex]->recalculateY (borderWidth, envelopeDisplayAreaHeight);
    }
}

double EnvelopeComponent::getAnchorXPercent (long anchorIndex)
{
    if (anchorIndex >= envelopeAnchors.size ())
        return 0.0;
    return envelopeAnchors[anchorIndex]->xAxis.percentage;
}

double EnvelopeComponent::getAnchorYPercent (long anchorIndex)
{
    if (anchorIndex >= envelopeAnchors.size () )
        return 0.0;
    return envelopeAnchors[anchorIndex]->yAxis.percentage;
}

// since anchor positions are stored in 'percentages', we need to recalculate x/y positions if either the position of an anchor changes, or the dimensions of the containing window changes
void EnvelopeComponent::recalculateCoordinates ()
{
    for (auto curAnchor : envelopeAnchors)
    {
        curAnchor->recalculateX (borderWidth, envelopeDisplayAreaWidth);
        curAnchor->recalculateY (borderWidth, envelopeDisplayAreaHeight);
    }
}

void EnvelopeComponent::rightMouseClickOnAnchor (const juce::MouseEvent &/*e*/)
{
    auto selectedAnchor { curActiveAnchor };

    curActiveAnchor = nullptr;
    draggingAnchor = nullptr;

    juce::PopupMenu m;
    m.addItem ("Properties...", true, false, [this, selectedAnchor] ()
    {
        if (selectedAnchor != nullptr)
            selectedAnchor->setActive (false);
    });
    m.addItem ("Delete...", true, false, [this, selectedAnchor] ()
    {
        bool deleted {false};
        for (auto curAnchorIndex { 0 }; curAnchorIndex < envelopeAnchors.size (); ++curAnchorIndex)
        {
            if (envelopeAnchors [curAnchorIndex] == selectedAnchor)
            {
                deleteAnchor (curAnchorIndex);
                deleted = true;
                break;
            }
        }
        if (deleted == true && selectedAnchor != nullptr)
            selectedAnchor->setActive (false);
    });
    m.showMenuAsync ({}, [this] (int) {});
}

void EnvelopeComponent::rightMouseClickOnBackground (const juce::MouseEvent &e)
{
    // if the ctrl key is being held down, then this is a request to add
    if (e.mods.isCtrlDown ())
    {
        const auto colClicked { e.x };

        // verify that the click was after the first anchor, and before the last anchor
        if ((colClicked > envelopeAnchors[0]->curTime) && (colClicked < envelopeAnchors[envelopeAnchors.size () - 1]->curTime))
        {
            // find out at which index we should add this new anchor
            for (auto curEnvelopeAnchorIndex { 1 }; curEnvelopeAnchorIndex < envelopeAnchors.size () - 1; ++curEnvelopeAnchorIndex)
            {
                if (colClicked < envelopeAnchors[curEnvelopeAnchorIndex]->curTime )
                {
                    auto newEnvelopeAnchor { new EnvelopeAnchorComponent };
                    newEnvelopeAnchor->curX = colClicked;
                    newEnvelopeAnchor->curY = envelopeAnchors[curEnvelopeAnchorIndex]->curAmplitude;
                    newEnvelopeAnchor->xAxis.percentage = (float) e.x / (float) envelopeDisplayAreaWidth;
                    newEnvelopeAnchor->recalculateX (borderWidth, envelopeDisplayAreaWidth);
                    newEnvelopeAnchor->yAxis.percentage = envelopeAnchors[curEnvelopeAnchorIndex]->yAxis.percentage;
                    newEnvelopeAnchor->recalculateY (borderWidth, envelopeDisplayAreaHeight);
                    addAnchor (newEnvelopeAnchor, curEnvelopeAnchorIndex );
                    break;
                }
            }
        }
    }
//     else
//     {
//         //////////////////////////
//         // test code
//         // test code toggle enabled state
//         setEnabled (! isEnabled ());
//         //////////////////////////
//     }
}

// called when the mouse is clicked on an anchor
void EnvelopeComponent::mouseDownOnAnchor (const juce::MouseEvent &e)
{
    // shift key - add to current selection
    // ctrl key  - toggle current selection
    if (isEnabled ())
    {
        resetWhenDone = false;
        if (e.mods.isRightButtonDown ())
        {
            rightMouseClickOnAnchor (e);
        }
        else
        {
            // check modifier keys to determine selection type
            if (e.mods.isShiftDown ())
            {
                // if shift is held down, setting anchor selection

                // set current as selected
                curActiveAnchor->setSelected (true);
            }
            else if (e.mods.isCtrlDown ())
            {
                // if ctrl is held down, toggling anchor selection

                // toggle the current selection setting
                curActiveAnchor->setSelected (! curActiveAnchor->isSelected ());
            }
            else
            {
                // no modifier keys held down, this is an individual anchor selection

                // if this anchor isn't selected, then let's make sure none of the other anchors are selected
                // otherwise, if it is selected, we are possibly starting a move operation
                if (! curActiveAnchor->isSelected ())
                {
                    for (auto curAnchor: envelopeAnchors)
                        curAnchor->setSelected (false);
                }

                if (! curActiveAnchor->isSelected ())
                    resetWhenDone = true;
                curActiveAnchor->setSelected (true);
            }

            if (curActiveAnchor->isSelected ())
            {
                draggingAnchor = curActiveAnchor;
                startX = draggingAnchor->curTime;
                startY = draggingAnchor->curAmplitude;
            }
        }
        repaint ();
    }
}

// called when mouse is clicked on the background
void EnvelopeComponent::mouseDownOnBackground (const juce::MouseEvent &e)
{
    if (e.mods.isRightButtonDown ())
    {
        rightMouseClickOnBackground (e);
    }
    else
    {
        // start rectangle selection
        startRectSelectX = e.getPosition ().getX ();
        startRectSelectY = e.getPosition ().getY ();
        curRectSelectX = startRectSelectX;
        curRectSelectY = startRectSelectY;
    }

    repaint ();
}

// called when done dragging an anchor
void EnvelopeComponent::mouseUpDraggingAnchor (const juce::MouseEvent &e)
{
    // no modifier keys pressed
    if (! e.mods.isShiftDown () && ! e.mods.isCtrlDown ())
    {
        // if mouse didn't move, clear all the selected ones
        if ((startX == curActiveAnchor->curTime) && (startY == curActiveAnchor->curAmplitude))
        {
            for (auto& curAnchor : envelopeAnchors)
                curAnchor->setSelected (false);
        }
        else
        {
            // if mouse did move
            if (resetWhenDone)
            {
                auto anotherSelected { false };
                for (auto& curAnchor : envelopeAnchors)
                    if ((curActiveAnchor != curAnchor) && curAnchor->isSelected ())
                        anotherSelected = true;
                // if there weren't any other anchors selected, reset this one, otherwise leave everything selected
                if (! anotherSelected)
                    curActiveAnchor->setSelected (false);

                resetWhenDone = false;
            }
        }
        repaint ();
    }
    draggingAnchor = nullptr;
}

// called when done creating a rectangle selection
void EnvelopeComponent::mouseUpRectangleSelection (const juce::MouseEvent &e)
{
    //juce::Logger::outputDebugString ("-- doing rectangular selection --");
    auto width { curRectSelectX - startRectSelectX };
    auto height { curRectSelectY - startRectSelectY };
    auto startSelectX { startRectSelectX };
    auto startSelectY { startRectSelectY };
    if (width < 0)
    {
        startSelectX += width;
        width = abs (width);
    }
    if (height < 0)
    {
        startSelectY += height;
        height = abs (height);
    }
    auto selectionRect { juce::Rectangle<float> ((float) startSelectX, (float) startSelectY, (float) width, (float) height) };
    for (auto& curAnchor : envelopeAnchors)
    {
        if (selectionRect.contains ((float) curAnchor->curX, (float) curAnchor->curY))
        {
            // this anchor is within the selection rectangle

            // if ctrl is held, but not shift, toggle the anchor setting
            if (e.mods.isCtrlDown () && ! e.mods.isShiftDown ())
                curAnchor->setSelected (! curAnchor->isSelected ());
            else
                curAnchor->setSelected (true);
        }
        else
        {
            // this anchor is not within the selection rectangle

            // if shift is not being held, deselect any anchors outside of selection rectangle
            if (! e.mods.isShiftDown () && ! e.mods.isCtrlDown ())
                curAnchor->setSelected (false);
        }
    }

    // reset the rectangle selection settings
    startRectSelectX = -1;
    startRectSelectY = -1;
    curRectSelectX = -1;
    curRectSelectY = -1;
    repaint ();
}

// called when dragging an anchor
void EnvelopeComponent::mouseDragAnchor (const juce::MouseEvent &e)
{
    auto changed { false };

    if (! draggingAnchor->xAxis.locked)
    {
        auto minX { 0L };
        auto maxX { 0L };

        auto curAnchor { draggingAnchor };
        if (curAnchor->anchorToLeft == nullptr)
        {
            minX = borderWidth;
        }
        else
        {
            minX = curAnchor->anchorToLeft->curX;
            curAnchor = curAnchor->anchorToLeft;

            while (curAnchor->anchorToLeft != nullptr && curAnchor->isSelected ())
            {
                minX = curAnchor->anchorToLeft->curX;
                curAnchor = curAnchor->anchorToLeft;
            }
            if (curAnchor->anchorToLeft == nullptr)
            {
                minX = borderWidth;
            }
        }

        curAnchor = draggingAnchor;
        if (curAnchor->anchorToRight == nullptr)
        {
            maxX = borderWidth + envelopeDisplayAreaWidth;
        }
        else
        {
            maxX = curAnchor->anchorToRight->curX;
            curAnchor = curAnchor->anchorToRight;

            while (curAnchor->anchorToRight != nullptr && curAnchor->isSelected ())
            {
                maxX = curAnchor->anchorToRight->curX;
                curAnchor = curAnchor->anchorToRight;
            }
            if (curAnchor->anchorToRight == nullptr)
            {
                maxX = borderWidth + envelopeDisplayAreaWidth;
            }
        }

        const auto newX { std::min (std::max ((long) e.x, minX), maxX) };

        if (newX != draggingAnchor->curTime)
        {
            auto xOffset { newX - draggingAnchor->curTime };

            for (auto& curAnchor2 : envelopeAnchors)
            {
                if (curAnchor2->isSelected ())
                {
                    int newMaxXOffset = curAnchor2->distanceMovedX (xOffset);

                    if ((xOffset < 0) && (newMaxXOffset > xOffset))
                        xOffset = newMaxXOffset;
                    else if ((xOffset > 0) && (newMaxXOffset < xOffset))
                        xOffset = newMaxXOffset;
                }
            }

            // update anchor being dragged, both pixel value and percent value
            if (xOffset < 0)
            {
                for (auto& curAnchor2 : envelopeAnchors)
                {
                    if (curAnchor2->isSelected ())
                    {
                        curAnchor2->curX += xOffset;
                        curAnchor2->recalculateXPercent (borderWidth, envelopeDisplayAreaWidth);

                        auto anchorToLeft { curAnchor2->anchorToLeft };
                        while ((anchorToLeft != nullptr) && (anchorToLeft->curX > curAnchor2->curX))
                        {
                            anchorToLeft->curX = curAnchor2->curX;
                            anchorToLeft->recalculateXPercent (borderWidth, envelopeDisplayAreaWidth);

                            anchorToLeft = anchorToLeft->anchorToLeft;
                        }

                    }
                }
            }
            else
            {
                for (auto curAnchorIndex { envelopeAnchors.size () - 1 }; curAnchorIndex >= 0; --curAnchorIndex)
                {
                    if (envelopeAnchors[curAnchorIndex]->isSelected ())
                    {
                        envelopeAnchors[curAnchorIndex]->curTime += xOffset;
                        envelopeAnchors[curAnchorIndex]->recalculateXPercent (borderWidth, envelopeDisplayAreaWidth);

                        auto anchorToRight { envelopeAnchors[curAnchorIndex]->anchorToRight };
                        while ((anchorToRight != nullptr) && (anchorToRight->curX < envelopeAnchors[curAnchorIndex]->curTime))
                        {
                            anchorToRight->curX = envelopeAnchors[curAnchorIndex]->curTime;
                            anchorToRight->recalculateXPercent (borderWidth, envelopeDisplayAreaWidth);

                            anchorToRight = anchorToRight->anchorToRight;
                        }
                    }
                }
            }

            // update anchor's that have 'mFollowLeft' set, both pixel value and percent value
            auto curFollowCheckAnchor { draggingAnchor->anchorToLeft };
            while ((curFollowCheckAnchor != nullptr) && (curFollowCheckAnchor->xAxis.followLeft))
            {
                // exclude any selected anchors, as they have already been moved by the previous code
                if (! curFollowCheckAnchor->isSelected ())
                {
                    curFollowCheckAnchor->curX += xOffset;
                    curFollowCheckAnchor->recalculateXPercent (borderWidth, envelopeDisplayAreaWidth);
                }

                curFollowCheckAnchor = curFollowCheckAnchor->anchorToLeft;
            }

            // update anchor's that have 'mFollowRight' set, both pixel value and percent value
            curFollowCheckAnchor = draggingAnchor->anchorToRight;
            while ((curFollowCheckAnchor != nullptr) && (curFollowCheckAnchor->xAxis.followRight))
            {
                if (! curFollowCheckAnchor->isSelected ())
                {
                    curFollowCheckAnchor->curX += xOffset;
                    curFollowCheckAnchor->recalculateXPercent (borderWidth, envelopeDisplayAreaWidth);
                }
                curFollowCheckAnchor = curFollowCheckAnchor->anchorToRight;
            }

            changed = true;
        }
    }

    if (! draggingAnchor->yAxis.locked)
    {
        const auto newY { std::min (std::max ((long) e.y, (long) borderWidth), (long) (getHeight () - borderWidth)) };

        if (newY != draggingAnchor->curAmplitude)
        {
            auto yOffset { newY - draggingAnchor->curAmplitude };

            for (auto& curAnchor : envelopeAnchors)
            {
                if (curAnchor->isSelected ())
                {
                    const auto newMaxYOffset { curAnchor->distanceMovedY (yOffset) };

                    if ((yOffset < 0) && (newMaxYOffset > yOffset))
                        yOffset = newMaxYOffset;
                    else if ((yOffset > 0) && (newMaxYOffset < yOffset))
                        yOffset = newMaxYOffset;
                }
            }

            // update anchor being dragged, both pixel value and percent value
            for (auto& curAnchor : envelopeAnchors)
            {
                if (curAnchor->isSelected ())
                {
                    curAnchor->curY += yOffset;
                    curAnchor->recalculateYPercent (borderWidth, envelopeDisplayAreaHeight);
                }
            }

            if (draggingAnchor->yAxis.followLeft)
            {
                auto leftAnchor { draggingAnchor };

                do
                {
                    leftAnchor = leftAnchor->anchorToLeft;

                    if (! leftAnchor->isSelected () )
                    {
                        leftAnchor->curY += yOffset;
                        leftAnchor->recalculateYPercent (borderWidth, envelopeDisplayAreaHeight);
                    }

                } while (leftAnchor->yAxis.followLeft);
            }

            if (draggingAnchor->yAxis.followRight)
            {
                auto rightAnchor { draggingAnchor };

                do
                {
                    rightAnchor = rightAnchor->anchorToRight;

                    if (! rightAnchor->isSelected ())
                    {
                        rightAnchor->curY += yOffset;
                        rightAnchor->recalculateYPercent (borderWidth, envelopeDisplayAreaHeight);
                    }

                } while (rightAnchor->yAxis.followRight);
            }

            changed = true;
        }
    }

    if (changed)
    {
        // inform any users of us that something has changed
        anchorChanged ();
        repaint ();
    }
}

// called when dragging/creating a rectangle selection
void EnvelopeComponent::mouseDragRectangleSelection (const juce::MouseEvent &e)
{
    curRectSelectX = e.getPosition ().getX ();
    curRectSelectY = e.getPosition ().getY ();
    repaint ();
}

void EnvelopeComponent::mouseDown (const juce::MouseEvent &e)
{
    if (curActiveAnchor != nullptr)
        mouseDownOnAnchor (e); // if the mouse is currently over an anchor
    else
        mouseDownOnBackground (e); // handle a mouse down that is not on a anchor
}

void EnvelopeComponent::mouseUp (const juce::MouseEvent &e)
{
    if (draggingAnchor != nullptr)
        mouseUpDraggingAnchor (e); // if we are currently dragging an anchor
    else if (startRectSelectX != -1)
        mouseUpRectangleSelection (e);
}

void EnvelopeComponent::mouseDrag (const juce::MouseEvent &e)
{
    if (draggingAnchor != nullptr)
        mouseDragAnchor (e); // if we are currently dragging an anchor
    else if (startRectSelectX != -1)
        mouseDragRectangleSelection (e);
}

void EnvelopeComponent::mouseMove (const juce::MouseEvent &e)
{
    // find the anchor which is under the mouse (or nullptr)
    auto mouseOverAnchor { (EnvelopeAnchorComponent*) nullptr };
    for (auto& curAnchor : envelopeAnchors)
    {
        if (juce::Rectangle<int> (offset + curAnchor->curX - (anchorSize / 2), offset + curAnchor->curY - (anchorSize / 2), anchorSize, anchorSize).contains (e.x, e.y))
        {
            mouseOverAnchor = curAnchor;
            break;
        }
    }

    // if the anchor has changed
    if (mouseOverAnchor != curActiveAnchor)
    {
        if (curActiveAnchor != nullptr)
            curActiveAnchor->setActive (false);

        curActiveAnchor = mouseOverAnchor;

        if (curActiveAnchor != nullptr)
            curActiveAnchor->setActive (true);

        repaint ();
    }
}

void EnvelopeComponent::resized ()
{
    // calculate the actual display width, which includes subtracting
    // the border width on either side
    envelopeDisplayAreaWidth  = getWidth () - (borderWidth * 2) - anchorSize;
    envelopeDisplayAreaHeight = getHeight () - (borderWidth * 2) - anchorSize;

    for (auto& curAnchor : envelopeAnchors)
    {
        curAnchor->setMaxX (envelopeDisplayAreaWidth);
        curAnchor->setMaxY (envelopeDisplayAreaHeight);
    }
    recalculateCoordinates ();
}

void EnvelopeComponent::paintBackground (juce::Graphics& g)
{
    // fill background with color
    //g.fillAll ( backgroundColour );

//     g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.3f), 0, 0,
//         Colours::transparentBlack, getWidth (), getHeight (), false));
//     g.fillRect (0, 0, getWidth (), getHeight ());

    g.fillAll (juce::Colours::white);
    g.drawImage (backgroundImage, 0, 0, getWidth (), getHeight (), 0, 0, backgroundImage.getWidth (), backgroundImage.getHeight (), false);

    g.setColour (borderColour);
    // draw border
    g.drawRect ( 0, 0, getWidth (), getHeight (), borderWidth);
}

void EnvelopeComponent::paint (juce::Graphics& g)
{
    if ( ! isEnabled () )
        g.setOpacity (0.5f);

    paintBackground ( g );

    // draw envelope line between each two anchors (thus we only iterate thru size - 1)
    for (auto curAnchor { 0 }; curAnchor < envelopeAnchors.size () - 1; ++curAnchor)
    {
        juce::Line<float> lineSegment ((float)(offset + envelopeAnchors[curAnchor]->curTime), (float)(offset + envelopeAnchors[curAnchor]->curAmplitude), (float)(offset + envelopeAnchors[curAnchor + 1]->curTime), (float)(offset + envelopeAnchors[curAnchor + 1]->curAmplitude));

        // set color for the envelope
        g.setColour (glowColour.brighter (0.8f).withAlpha (0.4f));
        g.drawLine (lineSegment, 3.0f);

        // set color for the envelope
        g.setColour (lineColour.brighter (0.2f));
        g.drawLine (lineSegment, 1.0f);
    }

    // draw anchors
    for (auto curAnchor : envelopeAnchors)
        curAnchor->display (g, offset, offset);

    // draw rectangular selection
    if (startRectSelectX > -1)
    {
        g.setColour (juce::Colours::darkcyan);
        auto width { curRectSelectX - startRectSelectX };
        auto height { curRectSelectY - startRectSelectY };
        auto startSelectX { startRectSelectX };
        auto startSelectY { startRectSelectY };
        if (width < 0)
        {
            startSelectX += width;
            width = abs (width);
        }
        if (height < 0)
        {
            startSelectY += height;
            height = abs (height);
        }
        g.drawRect ((float) startSelectX, (float) startSelectY, (float) width, (float) height, 1.0f);
    }
}
