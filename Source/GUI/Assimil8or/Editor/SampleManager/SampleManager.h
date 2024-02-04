#pragma once

#include <JuceHeader.h>
#include "SampleManagerProperties.h"
#include "SampleProperties.h"
#include "SamplePool.h"
#include "../../../../AppProperties.h"
#include "../../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../../Utility/DebugLog.h"
#include "../../../../Utility/RuntimeRootProperties.h"

class SampleManager
{
public:
    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getSampleProperties (int channelIndex, int zoneIndex);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    PresetProperties presetProperties;
    SampleManagerProperties sampleManagerProperties;
    std::array<ChannelProperties, 8> channelPropertiesList;
    std::array<std::array<ZoneProperties, 8>, 8> zonePropertiesList;
    std::array<std::array<SampleProperties, 8>, 8> samplePropertiesList;
    // TODO - should SamplePool functionality just be moved into SampleManager, since the the pool class shouldn't be shared anymore
    SamplePool samplePool;
    void handleSampleChange (int channelIndex, int zoneIndex, juce::String sampleName);
};