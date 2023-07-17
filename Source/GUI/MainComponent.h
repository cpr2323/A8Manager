#pragma once

#include <JuceHeader.h>
#include "ToolWindow.h"
#include "Assimil8or/Assimil8orPresetComponent.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent (juce::ValueTree rootPropertiesVT);
    ~MainComponent () override = default;

private:
    Assimil8orPresetComponent assimil8orPresetComponent;
    ToolWindow toolWindow;

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
