#include "EnvelopeAnchorComponent.h"

EnvelopeAnchorComponent::EnvelopeAnchorComponent ()
{
    updateDisplayState ();
}

void EnvelopeAnchorComponent::display (juce::Graphics& g, int xOffset, int yOffset)
{
    // draw an anchor if the anchor can move at least in one direction
    if ((! xAxis.locked) || (! yAxis.locked))
    {
        drawCircle (g, (float)(curTime + xOffset), (float)(curAmplitude + yOffset), 1.0, 8.0, color);
    }
}

void EnvelopeAnchorComponent::drawCircle (juce::Graphics& g, float centerX, float centerY, float startSize, float totalSize, juce::Colour circleColor)
{
    const auto lineWidth { 1.0f };
    const auto endSize { startSize + totalSize };
    const auto alphaStep { 1.0f / (totalSize / lineWidth) };
    auto alpha { 1.0f };
    for (auto curSize { startSize }; curSize < endSize; curSize += lineWidth)
    {
        const auto curRadius { curSize / 2.0f };
        g.setColour (circleColor.withAlpha (alpha));
        g.drawEllipse (juce::Rectangle<float> (centerX - curRadius, centerY - curRadius, curSize, curSize), lineWidth);
        alpha -= alphaStep;
    }
//     const auto radius {endSize / 2.0f};
//      g.setColour (color.withAlpha (1.0f));
//      g.drawEllipse (Rectangle<float> (centerX - radius, centerY - radius, endSize, endSize), lineWidth);
}

void EnvelopeAnchorComponent::updateDisplayState ()
{
    color = juce::Colours::cyan;
    if (xAxis.locked && yAxis.locked)
        return;

    if (!enabled)
        color = juce::Colours::darkgrey;
    else if (active)
        if (selected)
            color = juce::Colours::yellow;
        else
            color = juce::Colours::white;
    else
        if (selected)
            color = juce::Colours::greenyellow;
        else
            color = juce::Colours::cyan;
} 

int EnvelopeAnchorComponent::distanceMovedX (int xMove)
{
    auto distanceMoved { xMove };

    if (xAxis.locked)
    {
        distanceMoved = 0;
    }
    else if (xMove < 0)      // moving to the left
    {
        // if this is the furthest left anchor, and the move would be to less than zero, clamp the value to -mCurX
        if (anchorToLeft == nullptr)
        {
            if (curTime + xMove < 0)
                distanceMoved = -curTime;
        }
        else if (anchorToLeft->xAxis.locked)    // anchor to left is locked, can only go as far as anchor to lefts X
        {
            distanceMoved = anchorToLeft->curTime - curTime;
        }
        else if (! anchorToLeft->isSelected ())  // anchor to left is NOT selected
        {
            // not selected, can only go as far as anchor to lefts X
            {
                if (curTime + xMove < anchorToLeft->curTime)
                    distanceMoved = anchorToLeft->curTime - curTime;
            }
        }
        else    // otherwise, it IS selected, so let's see how far it can move
        {
            distanceMoved = anchorToLeft->distanceMovedX (distanceMoved);
        }
    }
    else if (xMove > 0) // moving to the right
    {
        // if this is the furthest right anchor, and the move would be to greater than mMaxX, clamp the value to -mCurX
        if (anchorToRight == nullptr)
        {
            if (curTime + xMove > maxTime)
                distanceMoved = maxTime - curTime;
        }
        else if (anchorToRight->xAxis.locked)    // anchor to right is locked, can only go as far as anchor to rights X
        {
            distanceMoved = anchorToRight->curTime - curTime;
        }
        else if (! anchorToRight->isSelected ())
        {
            // not selected, can only go as far as anchor to rights X
            {
                if (curTime + xMove > anchorToRight->curTime)
                    distanceMoved = anchorToRight->curTime - curTime;
            }
        }
        else
        {
            distanceMoved = anchorToRight->distanceMovedX (distanceMoved);
        }
    }

    return distanceMoved;
}

int EnvelopeAnchorComponent::distanceMovedY (int yMove)
{
    auto distanceMoved { yMove };

    if (yAxis.locked)
    {
        distanceMoved = 0;
    }
    if (yMove < 0)      // moving up
    {
        if ((curAmplitude + yMove) < 0)
            distanceMoved = -curAmplitude;
    }
    else if (yMove > 0) // moving down
    {
        if ((curAmplitude + yMove) > maxAmplitude)
            distanceMoved = maxAmplitude - curAmplitude;
    }

    return distanceMoved;
}
