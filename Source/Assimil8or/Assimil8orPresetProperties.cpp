#include "Assimil8orPresetProperties.h"

void Assimil8orPresetProperties::initValueTree ()
{

}

void Assimil8orPresetProperties::forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback)
{
    jassert (channelVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, Assimil8orChannelProperties::ChannelTypeId, [this, channelVTCallback] (juce::ValueTree channelVT)
    {
        channelVTCallback (channelVT);
        return true;
    });
}

void Assimil8orChannelProperties::forEachZone (std::function<bool (juce::ValueTree zoneVT)> zoneVTCallback)
{
    jassert (zoneVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, Assimil8orZoneProperties::ZoneTypeId, [this, zoneVTCallback] (juce::ValueTree zoneVT)
    {
        zoneVTCallback (zoneVT);
        return true;
    });
}

void Assimil8orPresetProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{

}
