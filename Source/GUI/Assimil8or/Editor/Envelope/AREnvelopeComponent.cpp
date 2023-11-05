#include "AREnvelopeComponent.h"

const auto kAnchorSize { 8.0 };

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
    auto drawAnchor = [this, &g] (float centerX, float centerY)
    {
        juce::Colour color { juce::Colours::grey };
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
            g.drawEllipse (juce::Rectangle<float> (centerX - curRadius, centerY - curRadius, curSize, curSize), lineWidth);
            alpha -= alphaStep;
        }
    };

    g.drawLine (startAnchor.getX (), startAnchor.getY (), attackAnchor.getX (), attackAnchor.getY ());
    g.drawLine (attackAnchor.getX (), attackAnchor.getY (), releaseAnchor.getX (), releaseAnchor.getY ());
    juce::Colour color { juce::Colours::black};
    g.fillEllipse (juce::Rectangle<float> (startAnchor.getX () - (kAnchorSize / 2.0), startAnchor.getY () - (kAnchorSize / 2.0), kAnchorSize, kAnchorSize));
    drawAnchor (attackAnchor.getX (), attackAnchor.getY ());
    drawAnchor (releaseAnchor.getX (), releaseAnchor.getY ());

//     g.setColour (juce::Colours::azure);
//     g.drawRect (editorArea);
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}

void AREnvelopeComponent::resized ()
{
    const auto offset { kAnchorSize / 2.0 };
    editorArea = getLocalBounds ().reduced (offset, offset).toFloat ();

    startAnchor.setX (offset);
    startAnchor.setY (offset + editorArea.getHeight ());

    attackAnchor.setX (offset + (editorArea.getWidth () * attackAnchor.getTime ()));
    attackAnchor.setY (offset + (editorArea.getHeight() - (editorArea.getHeight () * attackAnchor.getAmplitude())));

    releaseAnchor.setX (offset + (attackAnchor.getX () + editorArea.getWidth () * releaseAnchor.getTime ()));
    releaseAnchor.setY (offset + (editorArea.getHeight () - (editorArea.getHeight () * releaseAnchor.getAmplitude ())));
}
