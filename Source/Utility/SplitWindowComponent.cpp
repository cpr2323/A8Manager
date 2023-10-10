#include "SplitWindowComponent.h"

#define kSplitBarWidth 5

//////////////////////////////////////////////////////////////////////////////////////
class ResizerLookAndFeel : public juce::LookAndFeel_V2
{
    void drawStretchableLayoutResizerBar (juce::Graphics& g, int w, int h,
        bool /*isVerticalBar*/,
        bool isMouseOver,
        bool isMouseDragging) override
    {
        auto alpha { 0.5f };

        if (isMouseOver || isMouseDragging)
        {
            g.fillAll (juce::Colours::darkgrey);
            alpha = 1.0f;
        }
        else
        {
            g.fillAll (juce::Colours::silver);
            alpha = 1.0f;
        }

        const auto cx { w * 0.5f };
        const auto cy { h * 0.5f };
        const auto cr { juce::jmin (w, h) * 0.4f };

        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (alpha), cx + cr * 0.1f, cy + cr,
                           juce::Colours::black.withAlpha (alpha), cx, cy - cr * 4.0f, true));

        g.fillEllipse (cx - cr, cy - cr, cr * 2.0f, cr * 2.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////
SplitWindowComponent::SplitWindowComponent ()
{
}

SplitWindowComponent::~SplitWindowComponent ()
{
}

void SplitWindowComponent::setComponents (Component* theFirstComponent, Component* theSecondComponent)
{
    if (firstComponent != nullptr)
        removeChildComponent (firstComponent);
    if (secondComponent != nullptr)
        removeChildComponent (secondComponent);
    firstComponent = theFirstComponent;
    secondComponent = theSecondComponent;

    addAndMakeVisible (firstComponent);
    addAndMakeVisible (secondComponent);

    setHorizontalSplit (horizontalSplit);
}

bool SplitWindowComponent::getHorizontalSplit ()
{
    return horizontalSplit;
}

void SplitWindowComponent::setHorizontalSplit (bool theHorizontalSplit)
{
    horizontalSplit = theHorizontalSplit;
    resized ();
}

void SplitWindowComponent::setSplitOffset (int newSplitOffset)
{
    splitOffset = newSplitOffset;
    resized ();
}

int SplitWindowComponent::getSplitOffset ()
{
    return splitOffset;
}

void SplitWindowComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::lightslategrey);
    g.setColour (juce::Colours::darkgrey);
    g.fillRect (resizeBarBounds);
    if (mouseOver)
    {
        g.setColour (juce::Colours::white);
        g.drawRect (resizeBarBounds);
    }
}

void SplitWindowComponent::mouseMove (const juce::MouseEvent& me)
{
    auto mousePosition { me.getPosition () };
    {
        if (resizeBarBounds.contains (me.getPosition ()))
        {
            if (mouseOver)
                return;
            mouseOver = true;
            repaint ();
        }
        else
        {
            if (mouseOver)
            {
                mouseOver = false;
                repaint ();
            }
        }
    }
};

void SplitWindowComponent::mouseDrag (const juce::MouseEvent& me)
{
    if (mouseOver)
    {
        auto mousePosition { me.getPosition () };
        setSplitOffset (horizontalSplit ? mousePosition.getY () : mousePosition.getX ());
        repaint ();
    }
};

void SplitWindowComponent::resized ()
{
    auto r { getLocalBounds ().reduced (4) };
    if (horizontalSplit)
    {
        auto firstComponentBounds { r.removeFromTop (splitOffset - (kSplitBarWidth / 2)) };
        if (firstComponent != nullptr)
            firstComponent->setBounds (firstComponentBounds);
        resizeBarBounds = r.removeFromTop (kSplitBarWidth);
        if (secondComponent != nullptr)
            secondComponent->setBounds (r);
    }
    else
    {
        auto firstComponentBounds { r.removeFromLeft (splitOffset - (kSplitBarWidth / 2)) };
        if (firstComponent != nullptr)
            firstComponent->setBounds (firstComponentBounds);
        resizeBarBounds = r.removeFromLeft (kSplitBarWidth);
        if (secondComponent != nullptr)
            secondComponent->setBounds (r);
    }
}
