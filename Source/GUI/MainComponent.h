#pragma once

#include <JuceHeader.h>
#include "ToolWindow.h"
#include "Assimil8or/Assimil8orPresetComponent.h"
#include "Assimil8or/Assimil8orSdCardComponent.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent (juce::ValueTree rootPropertiesVT);
    ~MainComponent () override = default;

private:
        juce::TabbedComponent contentTab { juce::TabbedButtonBar::Orientation::TabsAtTop };

    Assimil8orPresetComponent assimil8orPresetComponent;
    Assimil8orSdCardComponent assimil8orSdCardComponent;
    ToolWindow toolWindow;

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
