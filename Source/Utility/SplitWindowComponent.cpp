#include "SplitWindowComponent.h"

#define kSplitBorderWidth 5

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
    : resizerLookAndFeel (new ResizerLookAndFeel)
{
}

SplitWindowComponent::~SplitWindowComponent ()
{
    resizerBar->setLookAndFeel (nullptr);
}

void SplitWindowComponent::setComponents (Component* theFirstComponent, Component* theSecondComponent)
{
    firstComponent = theFirstComponent;
    secondComponent = theSecondComponent;

    setHorizontalSplit (horizontalSplit);
}

bool SplitWindowComponent::getHorizontalSplit ()
{
    return horizontalSplit;
}

void SplitWindowComponent:: paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::lightslategrey);
}

void SplitWindowComponent::setHorizontalSplit (bool theHorizontalSplit)
{
    horizontalSplit = theHorizontalSplit;

    removeAllChildren ();
    resizerBar = std::make_unique<juce::StretchableLayoutResizerBar> (&stretchableManager, 1, ! horizontalSplit);
    resizerBar->setLookAndFeel (resizerLookAndFeel.get ());

    addAndMakeVisible (firstComponent);
    addAndMakeVisible (resizerBar.get ());
    addAndMakeVisible (secondComponent);

    stretchableManager.setItemLayout (0,
                                       -0.001, -0.99,
                                       -0.5);

    stretchableManager.setItemLayout (1,          // for the resize bar
                                       kSplitBorderWidth, kSplitBorderWidth, kSplitBorderWidth);   // hard limit to 'kSplitBorderWidth' pixels

    stretchableManager.setItemLayout (2,
                                       -0.001, -0.99,
                                       -0.5);
    resized ();
}

void SplitWindowComponent::setLayout (int componentIndex, double size)
{
    if (componentIndex > 2)
        return;

    stretchableManager.setItemLayout (componentIndex, -0.001, -0.99, size);
    resized ();
}

void SplitWindowComponent::resized ()
{
    const auto r { getLocalBounds ().reduced (4) };

    // make a list of two of our child components that we want to reposition
    Component* comps[] { firstComponent, resizerBar.get (), secondComponent };

    // this will position the 3 components, one above, or next to, the other, to fit
    // it into the rectangle provided.
    stretchableManager.layOutComponents (comps, 3,
                                         r.getX (), r.getY (), r.getWidth (), r.getHeight (),
                                         horizontalSplit, true);
}
