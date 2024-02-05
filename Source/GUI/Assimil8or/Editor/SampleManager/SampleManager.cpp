#include "SampleManager.h"
#include "../../../../Assimil8or/PresetManagerProperties.h"
#include "../../../../Utility/DebugLog.h"
#include "../../../../Utility/PersistentRootProperties.h"

void SampleManager::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFileChange = [this] (juce::String fileName)
    {
        // reset all samples being used
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
            for (auto zoneIndex { 0 }; zoneIndex < 8; ++zoneIndex)
                handleSampleChange (channelIndex, zoneIndex, "");
        samplePool.setFolder (juce::File (fileName).getParentDirectory ());
    };

    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::owner, SampleManagerProperties::EnableCallbacks::no);

    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.forEachChannel ([this] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        auto& channelProperties { channelPropertiesList [channelIndex] };
        channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
        channelProperties.forEachZone ([this, channelIndex] (juce::ValueTree zonePropertiesVT, int zoneIndex)
        {
            auto& zoneProperties { zonePropertiesList [channelIndex][zoneIndex] };
            zoneProperties.wrap (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
            zoneProperties.onSampleChange = [this, channelIndex, zoneIndex] (juce::String sampleName)
            {
                handleSampleChange (channelIndex, zoneIndex, sampleName);
            };
            samplePropertiesList [channelIndex][zoneIndex].wrap (sampleManagerProperties.getSamplePropertiesVT (channelIndex, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
            return true;
        });
        return true;
    });
}

juce::ValueTree SampleManager::getSampleProperties (int channelIndex, int zoneIndex)
{
    return samplePropertiesList [channelIndex][zoneIndex].getValueTree ();
}

void SampleManager::handleSampleChange (int channelIndex, int zoneIndex, juce::String sampleName)
{
    // NOTE: at first I was going to have and assert if the sampleName was the same as the currently loaded sample, but then I realized that one could load a new version of a sample (from an external folder)
    //       and we would need to re-open it to get all of the pertinent data.
    // TODO: Does this also means we need to make sure, when loading a sample that we bypass the ValueTree feature of not executing callbacks for same data, or do we set the name to empty before loading, which
    //       would enable the callbacks to happen correctly?
    //       ** Currently I think we need to reset the sampleName to "" before setting it to a new value, so it will force a reloading of the sample data
    auto& sampleProperties { samplePropertiesList [channelIndex][zoneIndex] };

    // if there is an already open sample, we want to close it
    if (sampleProperties.getName ().isNotEmpty ())
    {
        DebugLog ("SampleManager::handleSampleChange", "closing sample '" + sampleProperties.getName () + " 'for c" + juce::String(channelIndex) + "/z" + juce::String (zoneIndex));
        sampleProperties.setStatus (SampleData::SampleDataStatus::uninitialized, false); // this should inform clients to stop using the sample, before we reset everything else
        samplePool.close (sampleProperties.getName ());
        sampleProperties.setAudioBufferPtr (nullptr, false);
        sampleProperties.setBitsPerSample (0, false);
        sampleProperties.setLengthInSamples (0, false);
        sampleProperties.setName ("", false);
        sampleProperties.setNumChannels (0, false);
    }

    // if there is a new sample coming in (vs sample being reset) we want to open it
    if (sampleName.isNotEmpty ())
    {
        DebugLog ("SampleManager::handleSampleChange", "opening sample '" + sampleName + " 'for c" + juce::String (channelIndex) + "/z" + juce::String (zoneIndex));
        auto sampleData { samplePool.open (sampleName) };
        sampleProperties.setName (sampleName, false);
        sampleProperties.setBitsPerSample (sampleData.getBitsPerSample (), false);
        sampleProperties.setLengthInSamples (sampleData.getLengthInSamples (), false);
        sampleProperties.setNumChannels (sampleData.getNumChannels (), false);
        sampleProperties.setAudioBufferPtr (sampleData.getAudioBuffer (), false);
        sampleProperties.setStatus (sampleData.getStatus (), false);
    }
}
