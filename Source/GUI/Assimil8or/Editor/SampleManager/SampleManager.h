#pragma once

#include <JuceHeader.h>
#include "SampleManagerProperties.h"
#include "SampleProperties.h"
#include "SampleStatus.h"
#include "../../../../AppProperties.h"
#include "../../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../../Utility/DebugLog.h"
#include "../../../../Utility/DirectoryDataProperties.h"
#include "../../../../Utility/RuntimeRootProperties.h"

class SampleManager
{
public:
    SampleManager ();
    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getSampleProperties (int channelIndex, int zoneIndex);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    DirectoryDataProperties directoryDataProperties;
    PresetProperties presetProperties;
    SampleManagerProperties sampleManagerProperties;
    std::array<ChannelProperties, 8> channelPropertiesList;
    std::array<std::array<ZoneProperties, 8>, 8> zonePropertiesList;
    std::array<std::array<SampleProperties, 8>, 8> samplePropertiesList;

    juce::AudioFormatManager audioFormatManager;
    juce::File currentFolder;
    struct SampleData
    {
        int useCount { 0 };
        SampleStatus status { SampleStatus::uninitialized };
        int bitsPerSample { 0 };
        int numChannels { 0 };
        juce::int64 lengthInSamples { 0 };
        juce::AudioBuffer<float> audioBuffer;
    };
    std::map <juce::String, SampleData> sampleList;

    void handleSampleChange (int channelIndex, int zoneIndex, juce::String sampleName);
    SampleData& open (juce::String fileName);
    void close (juce::String fileName);
    void clear ();
    void update ();
    SampleData& loadSample (juce::String fileName);
    void updateSample (juce::String fileName, SampleData& sampleData);

};