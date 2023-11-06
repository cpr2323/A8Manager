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

    juce::Logger::outputDebugString ("editorWidth: " + juce::String (editorWidth, 5) + ", attackTime: " + juce::String (attackAnchor.getTime (), 5) + ", attackX: " + juce::String (editorWidth * attackAnchor.getTime (), 5));
    attackAnchor.setX (editorWidth * attackAnchor.getTime ());
    attackAnchor.setY (editorHeight - (editorHeight * attackAnchor.getAmplitude ()));

    releaseAnchor.setX (attackAnchor.getX () + editorWidth * releaseAnchor.getTime ());
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
                curActiveAnchor->setActive (isActive);
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

    if (e.mods.isShiftDown ())
    {
        // TODO move both
    }
}

void AREnvelopeComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (curActiveAnchor == nullptr)
        return;

    if (curActiveAnchor == &attackAnchor)
    {
        auto minX { 0.0 };
        auto maxX { e.mods.isShiftDown () ? releaseAnchor.getX () : editorWidth - releaseAnchor.getX () };
        const auto newX { std::fmin (std::fmax ((float) e.x, minX), maxX) };
        if (newX != attackAnchor.getX ())
        {
            const auto newAttackTime { newX / editorWidth };
            attackAnchor.setTime (newAttackTime);
            arEnvelopeProperties.setAttackPercent (newAttackTime, false);
            if (! e.mods.isShiftDown ())
            {
                //releaseAnchor.setX (attackAnchor.getX () + editorArea.getWidth () * releaseAnchor.getTime ());
            }
            else
            {
                juce::Logger::outputDebugString ("releaseAnchor.getX () - attackAnchor.getX (): " + juce::String (releaseAnchor.getX () - attackAnchor.getX ()));
                juce::Logger::outputDebugString ("editorArea.getWidth (): " + juce::String (editorWidth));
                const auto newReleaseTime { (releaseAnchor.getX () - newX) / editorWidth };
                juce::Logger::outputDebugString ("(releaseAnchor.getX () - attackAnchor.getX ()) / editorArea.getWidth (): " + juce::String (newReleaseTime, 5));
                releaseAnchor.setTime (newReleaseTime);
                arEnvelopeProperties.setReleasePercent (newReleaseTime, false);
            }
            recalcAnchorPositions ();
            repaint ();
        }
    }
    else
    {
        auto minX { attackAnchor.getX () };
        auto maxX { editorWidth };
        const auto newX { std::fmin (std::fmax ((float) e.x, minX), maxX) };
        if (newX != releaseAnchor.getX ())
        {
            const auto newReleaseTime { (newX - attackAnchor.getX ()) / editorWidth };
            releaseAnchor.setTime (newReleaseTime);
            arEnvelopeProperties.setReleasePercent (newReleaseTime, false);
            recalcAnchorPositions ();
            repaint ();
        }
    }
}
