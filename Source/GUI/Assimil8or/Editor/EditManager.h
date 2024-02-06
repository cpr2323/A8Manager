#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

class EditManager
{
public:
    EditManager ();

    void init (juce::ValueTree rootPropertiesVT, juce::ValueTree presetPropertiesVT);

    double getXfadeGroupValueByIndex (int xfadeGroupIndex);
    void setXfadeGroupValueByIndex (int xfadeGroupIndex, double value, bool doSelfCallback);

    void forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback);
    void forZones (int channelIndex, std::vector<int> zoneIndexList, std::function<void (juce::ValueTree)> zoneCallback);

    bool assignSamples (int channelIndex, int zoneIndex, const juce::StringArray& files);
    double clampMinVoltage (int channelIndex, int zoneIndex, double voltage);
    int getNumUsedZones (int channelIndex);
    std::tuple<double, double> getVoltageBoundaries (int channelIndex, int zoneIndex, int topDepth);
    bool isMinVoltageInRange (int channelIndex, int zoneIndex, double voltage);
    bool isSupportedAudioFile (juce::File file);

private:
    PresetProperties presetProperties;
    AppProperties appProperties;
    std::array<ChannelProperties, 8> channelPropertiesList;
    std::array<std::array<ZoneProperties, 8>, 8> zonePropertiesList;
    juce::AudioFormatManager audioFormatManager;
};

