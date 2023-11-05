#include "AREnvelopeComponent.h"

const auto kAnchorSize { 8.0 };
const auto kOffset { kAnchorSize / 2.0 };

AREnvelopeComponent::AREnvelopeComponent ()
{
    arEnvelopeProperties.wrap ({}, AREnvelopeProperties::WrapperType::owner, AREnvelopeProperties::EnableCallbacks::yes);
    arEnvelopeProperties.onAttackPercentChanged = [this] (double attackPercent) { attackPercentChanged (attackPercent); };
    arEnvelopeProperties.onReleasePercentChanged = [this] (double releasePercent) { releasePercentChanged (releasePercent); };

    attackAnchor.setAmplitude (1.0);

}

void AREnvelopeComponent::attackPercentChanged (double attackPercent)
{
    attackAnchor.setTime (attackPercent);
    attackAnchor.setX (getWidth () * attackAnchor.getTime ());
    releaseAnchor.setX (attackAnchor.getX () + getWidth () * releaseAnchor.getTime ());
    repaint ();
}

void AREnvelopeComponent::releasePercentChanged (double releasePercent)
{
    releaseAnchor.setTime (releasePercent);
    releaseAnchor.setX (attackAnchor.getX () + getWidth () * releaseAnchor.getTime ());
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
            g.drawEllipse (juce::Rectangle<float> (anchor.getX () - curRadius, anchor.getY () - curRadius, curSize, curSize), lineWidth);
            alpha -= alphaStep;
        }
    };

    g.drawLine (startAnchor.getX (), startAnchor.getY (), attackAnchor.getX (), attackAnchor.getY ());
    g.drawLine (attackAnchor.getX (), attackAnchor.getY (), releaseAnchor.getX (), releaseAnchor.getY ());
    juce::Colour color { juce::Colours::black};
    g.fillEllipse (juce::Rectangle<float> (startAnchor.getX () - (kAnchorSize / 2.0), startAnchor.getY () - (kAnchorSize / 2.0), kAnchorSize, kAnchorSize));
    drawAnchor (attackAnchor);
    drawAnchor (releaseAnchor);

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}

void AREnvelopeComponent::resized ()
{
    editorArea = getLocalBounds ().reduced (kOffset, kOffset).toFloat ();

    startAnchor.setX (kOffset);
    startAnchor.setY (kOffset + editorArea.getHeight ());

    attackAnchor.setX (kOffset + (editorArea.getWidth () * attackAnchor.getTime ()));
    attackAnchor.setY (kOffset + (editorArea.getHeight() - (editorArea.getHeight () * attackAnchor.getAmplitude())));

    releaseAnchor.setX (kOffset + (attackAnchor.getX () + editorArea.getWidth () * releaseAnchor.getTime ()));
    releaseAnchor.setY (kOffset + (editorArea.getHeight () - (editorArea.getHeight () * releaseAnchor.getAmplitude ())));
}

void AREnvelopeComponent::mouseMove (const juce::MouseEvent& e)
{
    auto isMouseOverAnchor = [&e] (EnvelopeAnchor& anchor) -> bool
    {
        return juce::Rectangle<int> (anchor.getX () - kOffset, anchor.getY () - kOffset, kAnchorSize, kAnchorSize).contains (e.x, e.y);
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
                curActiveAnchor->setActive (isActive);
        };
        setActive (false);
        curActiveAnchor = mouseOverAnchor;
        setActive (true);

        repaint ();
    }
}