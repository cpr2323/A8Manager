#pragma once

#include <JuceHeader.h>
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

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    PresetProperties presetProperties;
    std::array<ChannelProperties, 8> channelPropertiesList;
    std::array<std::array<ZoneProperties, 8>, 8> zonePropertiesList;
    // TODO - should SamplePool functionality just be moved into SampleManager, since the the pool class shouldn't be shared anymore
    SamplePool samplePool;
};