#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class StretchableLayoutResizerBarWithCallback : public juce::StretchableLayoutResizerBar
{
public:
    StretchableLayoutResizerBarWithCallback (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical)
        : StretchableLayoutResizerBar (layoutToUse, itemIndexInLayout, isBarVertical) {}

    std::function<void ()> onLayoutChange;

private:
    void hasBeenMoved () override
    {
        StretchableLayoutResizerBar::hasBeenMoved ();

        if (onLayoutChange != nullptr)
            onLayoutChange ();
    }
};

class SplitWindowComponent : public juce::Component
{
public:
    SplitWindowComponent ();
    ~SplitWindowComponent ();

    void setComponents (juce::Component* firstComponent, juce::Component* secondComponent);
    bool getHorizontalSplit ();
    void setHorizontalSplit (bool horizontalSplit);
    void setLayout (int componentIndex, double size);
    double getSize (int componentIndex);

    std::function<void ()> onLayoutChange;

private:
    juce::Component* firstComponent { nullptr };
    juce::Component* secondComponent { nullptr };
    juce::StretchableLayoutManager stretchableManager;
    std::unique_ptr<StretchableLayoutResizerBarWithCallback> resizerBar;
    bool horizontalSplit { true };
    std::unique_ptr<juce::LookAndFeel_V2> resizerLookAndFeel;

    void resized () override;
    void paint (juce::Graphics& g) override;
};
