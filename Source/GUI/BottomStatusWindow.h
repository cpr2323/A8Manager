#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Assimil8or/Validator/ValidatorProperties.h"

class BottomStatusWindow : public juce::Component
{
public:
    BottomStatusWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::Label progressUpdateLabel;
    ValidatorProperties validatorProperties;

    void updateProgress (juce::String progressUpdate);

    void lookAndFeelChanged () override;
    void paint (juce::Graphics& g) override;
    void resized () override;
};
