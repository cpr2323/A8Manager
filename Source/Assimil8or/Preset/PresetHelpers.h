#pragma once

#include <JuceHeader.h>
#include "ChannelProperties.h"
#include "PresetProperties.h"
#include "ZoneProperties.h"

namespace PresetHelpers
{
    bool areEntirePresetsEqual (juce::ValueTree presetOneVT, juce::ValueTree presetTwoVT);
    bool arePresetsEqual (juce::ValueTree presetOneVT, juce::ValueTree presetTwoVT);
    bool areChannelsEqual (juce::ValueTree channelOneVT, juce::ValueTree channelTwoVT);
    bool areZonesEqual (juce::ValueTree zoneOneVT, juce::ValueTree zoneTwoVT);
    void displayZoneDifferences (juce::ValueTree zonePropertiesOneVT, juce::ValueTree zonePropertiesTwoVT);
    void setCvInputAndAmount (CvInputAndAmount cvInputAndAmount, std::function<void (juce::String, double)> setter);
};
