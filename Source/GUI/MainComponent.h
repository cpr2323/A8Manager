#pragma once

#include <JuceHeader.h>
#include "ToolWindow.h"
#include "Assimil8or/Editor/Assimil8orEditorComponent.h"
#include "Assimil8or/Validator/Assimil8orValidatorComponent.h"

class MainComponent  : public juce::Component
{
public:
    MainComponent (juce::ValueTree rootPropertiesVT);
    ~MainComponent () override = default;

private:
        juce::TabbedComponent contentTab { juce::TabbedButtonBar::Orientation::TabsAtTop };

    Assimil8orEditorComponent assimil8orEditorComponent;
    Assimil8orValidatorComponent assimil8orValidatorComponent;
    ToolWindow toolWindow;
    juce::TooltipWindow tooltipWindow;

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
