#pragma once

#include <JuceHeader.h>
#include "ToolWindow.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent (juce::ValueTree persistentRootPropertiesVT, juce::ValueTree runtimeRootPropertiesVT);
    ~MainComponent () override;

private:
    ToolWindow toolWindow;
//    BezierComponent bezierComponent;

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
