#pragma once

#include <JuceHeader.h>
#include "../SampleManager/SampleManagerProperties.h"
#include "../SampleManager/SampleProperties.h"
#include "../../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../../Assimil8or/Preset/ZoneProperties.h"

class WaveformDisplay : public juce::Component
{
public:
    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT);
    void setZone (int zoneIndex);

private:
    ChannelProperties channelProperties;
    SampleManagerProperties sampleManagerProperties;
    ZoneProperties zoneProperties;
    SampleProperties sampleProperties;

    void paint (juce::Graphics& g);
};