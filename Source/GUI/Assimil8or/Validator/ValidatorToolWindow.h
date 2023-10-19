#pragma once

#include <JuceHeader.h>

class ValidatorToolWindow : public juce::Component
{
public:
    ValidatorToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::TextButton renameAllButton;
    juce::TextButton infoFilterButton;
    juce::TextButton warningFilterButton;
    juce::TextButton errorFilterButton;

    void paint (juce::Graphics& g) override;
    void resized () override;
};
