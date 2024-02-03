#include "SampleManager.h"
#include "../../../../Assimil8or/PresetManagerProperties.h"
#include "../../../../Utility/PersistentRootProperties.h"

void SampleManager::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFileChange = [this] (juce::String fileName)
    {
            // TODO - if we are changing folders, we need to also clear the current samples, and inform our clients of that
        samplePool.setFolder (juce::File (fileName).getParentDirectory ());
    };

    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.forEachChannel ([this] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        auto& channelProperties { channelPropertiesList [channelIndex] };
        channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
        channelProperties.forEachZone ([this, channelIndex] (juce::ValueTree zonePropertiesVT, int zoneIndex)
        {
            auto& sampleProperties { zonePropertiesList [channelIndex][zoneIndex] };
            sampleProperties.wrap (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);

            return true;
        });
        return true;
    });
}
