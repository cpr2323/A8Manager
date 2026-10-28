#pragma once

#include <JuceHeader.h>
#include "SampleManager/SampleProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioManager.h"
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
    void clearAllZones (int channelIndex);
    // delete, duplicate and explode also apply to the zone in the stereo right channel, when this zone is linked to it
    void deleteZone (int channelIndex, int zoneIndex);
    void duplicateZone (int channelIndex, int zoneIndex);
    void explodeZone (int channelIndex, int zoneIndex, int explodeCount);
    void flipZones (int channelIndex, int zoneIndex, int flipCount);
    double clampMinVoltage (int channelIndex, int zoneIndex, double voltage);
    juce::ValueTree getChannelDefaults ();
    juce::ValueTree getZoneDefaults ();
    juce::int64 getMaxLoopStart (int channelIndex, int zoneIndex);
    int getNumUsedZones (int channelIndex);
    std::tuple<double, double> getVoltageBoundaries (int channelIndex, int zoneIndex, int topDepth);
    bool isMinVoltageInRange (int channelIndex, int zoneIndex, double voltage);
    void resetMinVoltage (int channelIndex, int zoneIndex);

private:
    // returns channelIndex, followed by its stereo right channel if that channel's zone is linked to the zone (ie. has the same sample)
    std::vector<int> getLinkedChannels (int channelIndex, int zoneIndex);
    void removeEmptyZones (int channelIndex);
    void deleteZoneInChannel (int channelIndex, int zoneIndex);
    void duplicateZoneInChannel (int channelIndex, int zoneIndex);
    void explodeZoneInChannel (int channelIndex, int zoneIndex, int explodeCount);
    void flipZonesInChannel (int channelIndex, int zoneIndex, int flipCount);

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
    AudioManager* audioManager { nullptr };
};

