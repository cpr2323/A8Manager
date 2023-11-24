#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/PresetProperties.h"

class EditManager
{
public:
    void init (juce::ValueTree presetPropertiesVT);

    double getXfadeGroupValueByIndex (int xfadeGroupIndex);
    void setXfadeGroupValueByIndex (int xfadeGroupIndex, double value, bool doSelfCallback);

    void forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback);
    void forChannel (int channelIndex, std::function<void (juce::ValueTree)> channelCallback);

private:
    PresetProperties presetProperties;
    std::array<juce::ValueTree, 8> channelPropertiesList;
    std::array<std::array<juce::ValueTree, 8>, 8> zonePropertiesList;
};

