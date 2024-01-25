#include "EditManager.h"

void EditManager::init (juce::ValueTree presetPropertiesVT)
{
    presetProperties.wrap (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    auto channelIndex { 0 };
    presetProperties.forEachChannel ([this, &channelIndex] (juce::ValueTree channelVT)
    {
        ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        channelPropertiesList [channelIndex] = channelVT;
        auto zoneIndex { 0 };
        channelProperties.forEachZone ([this, channelIndex, &zoneIndex] (juce::ValueTree zoneVT)
        {
            zonePropertiesList [channelIndex][zoneIndex] = zoneVT;
            ++zoneIndex;
            return true;
        });
        ++channelIndex;
        return true;
    });
}

double EditManager::getXfadeGroupValueByIndex (int xfadeGroupIndex)
{
    switch (xfadeGroupIndex)
    {
        case 0: return presetProperties.getXfadeAWidth (); break;
        case 1: return presetProperties.getXfadeBWidth (); break;
        case 2: return presetProperties.getXfadeCWidth (); break;
        case 3: return presetProperties.getXfadeDWidth (); break;
        default: jassertfalse; return 0.0; break;
    }
}

void EditManager::setXfadeGroupValueByIndex (int xfadeGroupIndex, double value, bool doSelfCallback)
{
    switch (xfadeGroupIndex)
    {
        case 0: presetProperties.setXfadeAWidth (value, doSelfCallback); break;
        case 1: presetProperties.setXfadeBWidth (value, doSelfCallback); break;
        case 2: presetProperties.setXfadeCWidth (value, doSelfCallback); break;
        case 3: presetProperties.setXfadeDWidth (value, doSelfCallback); break;
        default: jassertfalse; break;
    }
}

void EditManager::forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback)
{
    jassert (channelCallback != nullptr);
    for (const auto channelIndex : channelIndexList)
    {
        jassert (channelIndex >= 0 && channelIndex < 8);
        channelCallback (channelPropertiesList [channelIndex]);
    }
}

void EditManager::forChannel (int channelIndex, std::function<void (juce::ValueTree)> channelCallback)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (channelCallback != nullptr);
    channelCallback (channelPropertiesList [channelIndex]);
}

void EditManager::forZones (int channelIndex, std::vector<int> zoneIndexList, std::function<void (juce::ValueTree)> zoneCallback)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneCallback != nullptr);
    for (const auto zoneIndex : zoneIndexList)
    {
        jassert (zoneIndex >= 0 && zoneIndex < 8);
        zoneCallback (zonePropertiesList [channelIndex][zoneIndex]);
    }
}

void EditManager::forZone (int channelIndex, int zoneIndex, std::function<void (juce::ValueTree)> zoneCallback)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneIndex >= 0 && zoneIndex < 8);
    jassert (zoneCallback != nullptr);
    zoneCallback (zonePropertiesList [channelIndex][zoneIndex]);
}
