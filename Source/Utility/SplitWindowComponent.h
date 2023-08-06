#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class SplitWindowComponent : public juce::Component
{
public:
    SplitWindowComponent ();
    ~SplitWindowComponent ();

    void setComponents (juce::Component* firstComponent, juce::Component* secondComponent);
    bool getHorizontalSplit ();
    void setHorizontalSplit (bool horizontalSplit);
    void setLayout (int componentIndex, double size);

private:
    juce::Component* firstComponent { nullptr };
    juce::Component* secondComponent { nullptr };
    juce::StretchableLayoutManager stretchableManager;
    std::unique_ptr<juce::StretchableLayoutResizerBar> resizerBar;
    bool horizontalSplit { true };
    std::unique_ptr<juce::LookAndFeel_V2> resizerLookAndFeel;

    void resized () override;
    void paint (juce::Graphics& g) override;
};
