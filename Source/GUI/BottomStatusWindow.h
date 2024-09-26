#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../Assimil8or/Validator/ValidatorProperties.h"

class BottomStatusWindow : public juce::Component
{
public:
    BottomStatusWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::Label progressUpdateLabel;
    ValidatorProperties validatorProperties;
    AudioPlayerProperties audioPlayerProperties;

    juce::TextButton settingsButton;
    std::unique_ptr<juce::AlertWindow> settingsAlertWindow;

    void updateProgress (juce::String progressUpdate);

    void paint (juce::Graphics& g) override;
    void resized () override;
};
