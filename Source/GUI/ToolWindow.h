#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Assimil8or/Audio/AudioConfigProperties.h"
#include "../Assimil8or/Validator/ValidatorProperties.h"

class ToolWindow : public juce::Component
{
public:
    ToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::Label progressUpdateLabel;
    ValidatorProperties validatorProperties;
    AudioConfigProperties audioConfigProperties;

    juce::TextButton settingsButton;
    std::unique_ptr<juce::AlertWindow> settingsAlertWindow;

    void updateProgress (juce::String progressUpdate);

    void paint (juce::Graphics& g) override;
    void resized () override;
};
