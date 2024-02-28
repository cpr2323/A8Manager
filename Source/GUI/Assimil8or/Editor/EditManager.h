#pragma once

#include <JuceHeader.h>
#include "SampleManager/SampleProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

class EditManager
{
public:
    EditManager ();

    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree presetPropertiesVT);

    double getXfadeGroupValueByIndex (int xfadeGroupIndex);
    void setXfadeGroupValueByIndex (int xfadeGroupIndex, double value, bool doSelfCallback);
    juce::String getXfadeCvValueByIndex (int xfadeGroupIndex);
    void setXfadeCvValueByIndex (int xfadeGroupIndex, juce::String cvInput, bool doSelfCallback);

    void forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback);
    void forZones (int channelIndex, std::vector<int> zoneIndexList, std::function<void (juce::ValueTree, juce::ValueTree)> zoneCallback);

    bool assignSamples (int channelIndex, int zoneIndex, const juce::StringArray& files);
    double clampMinVoltage (int channelIndex, int zoneIndex, double voltage);
    juce::int64 getMaxLoopStart (int channelIndex, int zoneIndex);
    int getNumUsedZones (int channelIndex);
    std::tuple<double, double> getVoltageBoundaries (int channelIndex, int zoneIndex, int topDepth);
    bool isMinVoltageInRange (int channelIndex, int zoneIndex, double voltage);
    bool isSupportedAudioFile (juce::File file);
    void resetMinVoltage (int channelIndex, int zoneIndex);

private:
    PresetProperties presetProperties;
    AppProperties appProperties;
    ChannelProperties defaultChannelProperties;
    ChannelProperties minChannelProperties;
    ChannelProperties maxChannelProperties;
    ZoneProperties defaultZoneProperties;
    ZoneProperties minZoneProperties;
    ZoneProperties maxZoneProperties;

    std::array<ChannelProperties, 8> channelPropertiesList;
    struct ZoneAndSampleProperties
    {
        ZoneProperties zoneProperties;
        SampleProperties sampleProperties;
    };
    std::array<std::array<ZoneAndSampleProperties, 8>, 8> zoneAndSamplePropertiesList;
    juce::AudioFormatManager audioFormatManager;
};

