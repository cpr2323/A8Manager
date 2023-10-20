#pragma once

#include <JuceHeader.h>
#include "ValidatorComponentProperties.h"

class ValidatorToolWindow : public juce::Component
{
public:
    ValidatorToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    ValidatorComponentProperties validatorComponentProperties;

    juce::TextButton convertAllButton;
    juce::TextButton locateAllButton;
    juce::TextButton renameAllButton;
    juce::TextButton viewInfoButton;
    juce::TextButton viewWarningButton;
    juce::TextButton viewErrorButton;

    void paint (juce::Graphics& g) override;
    void resized () override;
};
