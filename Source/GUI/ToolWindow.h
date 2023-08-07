#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Assimil8or/Validator/ValidatorProperties.h"

class ToolWindow : public juce::Component
{
public:
    ToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private: 
    juce::TimeSliceThread scanThread {"DirectoryContentsListThread"};
    juce::DirectoryContentsList directoryContentsList {nullptr, scanThread};

    juce::Label scanningStatusLabel;
    juce::Label progressUpdateLabel;
    ValidatorProperties validatorProperties;

    void updateProgress (juce::String progressUpdate);
    void updateScanStatus (juce::String scanStatus);

    void paint (juce::Graphics& g) override;
    void resized () override;
};
