#include "AREnvelopeComponent.h"

const auto kAnchorSize { 8.0 };
const auto kOffset { kAnchorSize / 2.0 };

AREnvelopeComponent::AREnvelopeComponent ()
{
    arEnvelopeProperties.wrap ({}, AREnvelopeProperties::WrapperType::owner, AREnvelopeProperties::EnableCallbacks::yes);
    arEnvelopeProperties.onAttackPercentChanged = [this] (double attackPercent) { attackPercentChanged (attackPercent); };
    arEnvelopeProperties.onReleasePercentChanged = [this] (double releasePercent) { releasePercentChanged (releasePercent); };

    attackAnchor.setAmplitude (1.0);
    attackAnchor.setMaxTime (0.5);
    releaseAnchor.setAmplitude (0.0);
    releaseAnchor.setMaxTime (0.5);
}

void AREnvelopeComponent::attackPercentChanged (double attackPercent)
{
    attackAnchor.setTime (attackPercent);
    recalcAnchorPositions ();
    repaint ();
}

void AREnvelopeComponent::releasePercentChanged (double releasePercent)
{
    releaseAnchor.setTime (releasePercent);
    recalcAnchorPositions ();
    repaint ();
}

void AREnvelopeComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey.darker (0.3f));
    g.fillRect (getLocalBounds ());

    g.setColour (juce::Colours::black);
    auto drawAnchor = [this, &g] (EnvelopeAnchor& anchor)
    {
        juce::Colour color { anchor.getActive () ? juce::Colours::white : juce::Colours::grey };
        const auto startSize { 1.0 };
        const auto totalSize { kAnchorSize };
        const auto lineWidth { 1.0f };
        const auto endSize { startSize + totalSize };
        const auto alphaStep { 1.0f / (totalSize / lineWidth) };
        auto alpha { 1.0f };
        for (auto curSize { startSize }; curSize < endSize; curSize += lineWidth)
        {
            const auto curRadius { curSize / 2.0f };
            g.setColour (color.withAlpha (alpha));
            g.drawEllipse (juce::Rectangle<float> (static_cast<float> (kOffset + anchor.getX () - curRadius),
                                                   static_cast<float> (kOffset + anchor.getY () - curRadius),
                                                   static_cast<float> (curSize),
                                                   static_cast<float> (curSize)), lineWidth);
            alpha -= alphaStep;
        }
        g.setColour (juce::Colours::black);
        g.drawEllipse (juce::Rectangle<float> (static_cast<float> (kOffset + anchor.getX () - endSize / 2.0f),
                                               static_cast<float> (kOffset + anchor.getY () - endSize / 2.0f),
                                               static_cast<float> (endSize),
                                               static_cast<float> (endSize)), lineWidth);
        };

    g.drawLine (static_cast<float> (kOffset + startAnchor.getX ()),
                static_cast<float> (kOffset + startAnchor.getY ()),
                static_cast<float> (kOffset + attackAnchor.getX ()),
                static_cast<float> (kOffset + attackAnchor.getY ()));
    g.drawLine (static_cast<float> (kOffset + attackAnchor.getX ()),
                static_cast<float> (kOffset + attackAnchor.getY ()),
                static_cast<float> (kOffset + releaseAnchor.getX ()),
                static_cast<float> (kOffset + releaseAnchor.getY ()));
    g.fillEllipse (juce::Rectangle<float> (static_cast<float> (kOffset + startAnchor.getX () - (kAnchorSize / 2.0)),
                                           static_cast<float> (kOffset + startAnchor.getY () - (kAnchorSize / 2.0)),
                                           static_cast<float> (kAnchorSize),
                                           static_cast<float> (kAnchorSize)));
    drawAnchor (attackAnchor);
    drawAnchor (releaseAnchor);

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}

void AREnvelopeComponent::recalcAnchorPositions ()
{
    startAnchor.setX (0);
    startAnchor.setY (editorHeight);

    attackAnchor.setX (editorWidth * attackAnchor.getTime ());
    attackAnchor.setY (editorHeight - (editorHeight * attackAnchor.getAmplitude ()));

    releaseAnchor.setX (attackAnchor.getX () + (editorWidth * releaseAnchor.getTime ()));
    releaseAnchor.setY (editorHeight - (editorHeight * releaseAnchor.getAmplitude ()));
}

void AREnvelopeComponent::resized ()
{
    editorWidth = getWidth () - kAnchorSize;
    editorHeight = getHeight () - kAnchorSize;
    recalcAnchorPositions ();
}

void AREnvelopeComponent::mouseMove (const juce::MouseEvent& e)
{
    if (! isEnabled ())
        return;
    auto isMouseOverAnchor = [&e] (EnvelopeAnchor& anchor) -> bool
    {
        return juce::Rectangle<int> (static_cast<int> (anchor.getX ()),
                                     static_cast<int> (anchor.getY ()),
                                     static_cast<int> (kAnchorSize),
                                     static_cast<int> (kAnchorSize)).contains (e.x, e.y);
    };
    EnvelopeAnchor* mouseOverAnchor { nullptr };
    if (isMouseOverAnchor (attackAnchor))
        mouseOverAnchor = &attackAnchor;
    else if (isMouseOverAnchor (releaseAnchor))
        mouseOverAnchor = &releaseAnchor;

    if (mouseOverAnchor != curActiveAnchor)
    {
        auto setActive = [this] (bool isActive)
        {
            if (curActiveAnchor != nullptr)
            {
                curActiveAnchor->setActive (isActive);
                if (isActive == false)
                    dragStartAnchorX = 0;
                else
                    dragStartAnchorX = static_cast<int> (curActiveAnchor->getX ());
            }
        };
        setActive (false);
        curActiveAnchor = mouseOverAnchor;
        setActive (true);

        repaint ();
    }
}

void AREnvelopeComponent::mouseExit (const juce::MouseEvent&)
{
    if (! isEnabled ())
        return;
    if (curActiveAnchor != nullptr)
    {
        curActiveAnchor->setActive (false);
        curActiveAnchor = nullptr;
        repaint ();
    }
}

void AREnvelopeComponent::mouseDown ([[maybe_unused]] const juce::MouseEvent& e)
{
    if (! isEnabled ())
        return;
    if (curActiveAnchor == nullptr)
        return;
}

void AREnvelopeComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (! isEnabled ())
        return;
    if (curActiveAnchor == nullptr)
        return;

    const auto newX { dragStartAnchorX + e.getDistanceFromDragStartX () };
    const auto newTime { newX / editorWidth };
    if (curActiveAnchor == &attackAnchor)
    {
        if (newTime - startAnchor.getTime () != attackAnchor.getTime ())
        {
            const auto newAttackTime { std::fmin (std::fmax ((float) newTime - startAnchor.getTime (), 0.0), std::fmin (attackAnchor.getMaxTime (), 1.0 - releaseAnchor.getTime ())) };
            attackAnchor.setTime (newAttackTime);
            arEnvelopeProperties.setAttackPercent (newAttackTime, false);
        }
    }
    else
    {
        if (newTime - attackAnchor.getTime () != releaseAnchor.getTime ())
        {
            const auto newReleaseTime { std::fmin (std::fmax ((float) newTime - attackAnchor.getTime (), 0.0), std::fmin (releaseAnchor.getMaxTime (), 1.0 - attackAnchor.getTime ())) };
            releaseAnchor.setTime (newReleaseTime);
            arEnvelopeProperties.setReleasePercent (newReleaseTime, false);
        }
    }
    recalcAnchorPositions ();
    repaint ();
}
