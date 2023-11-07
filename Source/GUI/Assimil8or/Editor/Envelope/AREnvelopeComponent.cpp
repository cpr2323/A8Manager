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
            g.drawEllipse (juce::Rectangle<float> (kOffset + anchor.getX () - curRadius, kOffset + anchor.getY () - curRadius, curSize, curSize), lineWidth);
            alpha -= alphaStep;
        }
    };

    g.drawLine (kOffset + startAnchor.getX (), kOffset + startAnchor.getY (), kOffset + attackAnchor.getX (), kOffset + attackAnchor.getY ());
    g.drawLine (kOffset + attackAnchor.getX (), kOffset + attackAnchor.getY (), kOffset + releaseAnchor.getX (), kOffset + releaseAnchor.getY ());
    juce::Colour color { juce::Colours::black};
    g.fillEllipse (juce::Rectangle<float> (kOffset + startAnchor.getX () - (kAnchorSize / 2.0), kOffset + startAnchor.getY () - (kAnchorSize / 2.0), kAnchorSize, kAnchorSize));
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
    auto isMouseOverAnchor = [&e] (EnvelopeAnchor& anchor) -> bool
    {
        return juce::Rectangle<int> (anchor.getX (), anchor.getY (), kAnchorSize, kAnchorSize).contains (e.x, e.y);
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
                    dragStartAnchorX = curActiveAnchor->getX ();
            }
        };
        setActive (false);
        curActiveAnchor = mouseOverAnchor;
        setActive (true);

        repaint ();
    }
}

void AREnvelopeComponent::mouseDown (const juce::MouseEvent& e)
{
    if (curActiveAnchor == nullptr)
        return;
}

void AREnvelopeComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (curActiveAnchor == nullptr)
        return;

    const auto newX { dragStartAnchorX + e.getDistanceFromDragStartX () };
    const auto newTime { newX / editorWidth };
    if (curActiveAnchor == &attackAnchor)
    {
        if (newTime - startAnchor.getTime () != attackAnchor.getTime ())
        {
            const auto newAttackTime { std::fmin (std::fmax ((float) newTime - startAnchor.getTime (), 0.0), std::fmin (attackAnchor.getMaxTime (), 1.0 - releaseAnchor.getTime ()))};
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
