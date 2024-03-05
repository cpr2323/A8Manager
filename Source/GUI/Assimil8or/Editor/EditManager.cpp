#include "EditManager.h"
#include "SampleManager/SampleManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

EditManager::EditManager ()
{
    audioFormatManager.registerBasicFormats ();
}

void EditManager::init (juce::ValueTree rootPropertiesVT, juce::ValueTree presetPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    {
        PresetProperties minPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MinParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        minChannelProperties.wrap (minPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        minZoneProperties.wrap (minChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }
    {
        PresetProperties maxPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        maxChannelProperties.wrap (maxPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        maxZoneProperties.wrap (maxChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }
    {
        PresetProperties defaultPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                                  PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        defaultChannelProperties.wrap (defaultPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        defaultZoneProperties.wrap (defaultChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    SampleManagerProperties sampleManagerProperties (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::owner, SampleManagerProperties::EnableCallbacks::no);

    presetProperties.wrap (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    presetProperties.forEachChannel ([this, &sampleManagerProperties] (juce::ValueTree channelVT, int channelIndex)
    {
        channelPropertiesList [channelIndex].wrap (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        channelPropertiesList [channelIndex].forEachZone ([this, &sampleManagerProperties, channelIndex] (juce::ValueTree zoneVT, int zoneIndex)
        {
            zoneAndSamplePropertiesList [channelIndex][zoneIndex].zoneProperties.wrap (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
            zoneAndSamplePropertiesList [channelIndex][zoneIndex].sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelIndex, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
            ++zoneIndex;
            return true;
        });
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

juce::String  EditManager::getXfadeCvValueByIndex (int xfadeGroupIndex)
{
    switch (xfadeGroupIndex)
    {
        case 0: return presetProperties.getXfadeACV (); break;
        case 1: return presetProperties.getXfadeBCV (); break;
        case 2: return presetProperties.getXfadeCCV (); break;
        case 3: return presetProperties.getXfadeDCV (); break;
        default: jassertfalse; return "Off"; break;
    }
}

void EditManager::setXfadeCvValueByIndex (int xfadeGroupIndex, juce::String value, bool doSelfCallback)
{
    switch (xfadeGroupIndex)
    {
        case 0: presetProperties.setXfadeACV (value, doSelfCallback); break;
        case 1: presetProperties.setXfadeBCV (value, doSelfCallback); break;
        case 2: presetProperties.setXfadeCCV (value, doSelfCallback); break;
        case 3: presetProperties.setXfadeDCV (value, doSelfCallback); break;
        default: jassertfalse; break;
    }
}

void EditManager::forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback)
{
    jassert (channelCallback != nullptr);
    for (const auto channelIndex : channelIndexList)
    {
        jassert (channelIndex >= 0 && channelIndex < 8);
        channelCallback (channelPropertiesList [channelIndex].getValueTree ());
    }
}

void EditManager::forZones (int channelIndex, std::vector<int> zoneIndexList, std::function<void (juce::ValueTree, juce::ValueTree)> zoneCallback)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneCallback != nullptr);
    for (const auto zoneIndex : zoneIndexList)
    {
        jassert (zoneIndex >= 0 && zoneIndex < 8);
        zoneCallback (zoneAndSamplePropertiesList [channelIndex][zoneIndex].zoneProperties.getValueTree (), zoneAndSamplePropertiesList [channelIndex][zoneIndex].sampleProperties.getValueTree ());
    }
}

int EditManager::getNumUsedZones (int channelIndex)
{
    jassert (channelIndex < 8);
    auto numUsedZones { 0 };
    for (auto curZoneIndex { 0 }; curZoneIndex < zoneAndSamplePropertiesList [channelIndex].size (); ++curZoneIndex)
        numUsedZones += zoneAndSamplePropertiesList [channelIndex][curZoneIndex].zoneProperties.getSample ().isNotEmpty () ? 1 : 0;
    return numUsedZones;
};

std::tuple<double, double> EditManager::getVoltageBoundaries (int channelIndex, int zoneIndex, int topDepth)
{
    auto topBoundary { 5.0 };
    auto bottomBoundary { -5.0 };

    // neither index 0 or 1 can look at the 'top boundary' index (ie. index - 2, the previous previous one)
    if (zoneIndex > topDepth)
        topBoundary = zoneAndSamplePropertiesList [channelIndex][zoneIndex - topDepth - 1].zoneProperties.getMinVoltage ();
    if (zoneIndex < getNumUsedZones (channelIndex) - 1)
        bottomBoundary = zoneAndSamplePropertiesList [channelIndex][zoneIndex + 1].zoneProperties.getMinVoltage ();
    return { topBoundary, bottomBoundary };
};

void EditManager::resetMinVoltage (int channelIndex, int zoneIndex)
{
    if (zoneIndex != getNumUsedZones (channelIndex) - 1)
    {
        const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (channelIndex, zoneIndex, 0) };
        zoneAndSamplePropertiesList [channelIndex][zoneIndex].zoneProperties.setMinVoltage (bottomBoundary + ((topBoundary - bottomBoundary) / 2), false);
    }
    else
    {
        zoneAndSamplePropertiesList [channelIndex][zoneIndex].zoneProperties.setMinVoltage (-5.0, false);
    }
}

bool EditManager::isMinVoltageInRange (int channelIndex, int zoneIndex, double voltage)
{
    jassert (channelIndex< 8);
    jassert (zoneIndex < 8);
    const auto numUsedZones { getNumUsedZones (channelIndex) };
    if (zoneIndex + 1 == numUsedZones)
    {
        return voltage == -5.0;
    }
    else if (zoneIndex + 1 > numUsedZones)
    {
        return voltage == 0.0;
    }
    else
    {
        const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (channelIndex, zoneIndex, 0) };
        return voltage > bottomBoundary && voltage < topBoundary;
    }
};

double EditManager::clampMinVoltage (int channelIndex, int zoneIndex, double voltage)
{
    if (zoneIndex != getNumUsedZones(channelIndex) - 1)
    {
        const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (channelIndex, zoneIndex, 0) };
        return std::clamp (voltage, bottomBoundary + 0.01, topBoundary - 0.01);
    }
    else
    {
        return -5.0;
    }
};

juce::ValueTree EditManager::getChannelDefaults ()
{
    return defaultChannelProperties.getValueTree ();
}

juce::ValueTree EditManager::getZoneDefaults ()
{
    return defaultZoneProperties.getValueTree ();
}

juce::int64 EditManager::getMaxLoopStart (int channelIndex, int zoneIndex)
{
    jassert (channelIndex < 8);
    jassert (zoneIndex < 8);
    auto& sampleProperties { zoneAndSamplePropertiesList [channelIndex][zoneIndex].sampleProperties };
    auto& zoneProperties { zoneAndSamplePropertiesList [channelIndex][zoneIndex].zoneProperties };
    if (! channelPropertiesList [channelIndex].getLoopLengthIsEnd ())
    {
        // if normal Loop Length behavior is used, then Loop Start cannot push Loop Length past the end of the sample
        const auto sampleLength { sampleProperties.getLengthInSamples () };
        const auto loopLength { static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (sampleLength - zoneProperties.getLoopStart ().value_or (0))) };
        const auto maxLoopStart { sampleLength - loopLength };
        //DebugLog ("EditManager::getMaxLoopStart (loopLength)", "sampleLength: " + juce::String(sampleLength) + ", loopLength: " + juce::String (loopLength) + ", maxLoopStart: " + juce::String (maxLoopStart));
        return sampleProperties.getLengthInSamples () < 4 ? 0 : maxLoopStart;
    }
    else
    {
        // if Loop Length is being viewed as Loop End, then Loop Length will be changed by the location of Loop Start, down to a End Of Sample - 4
        const auto loopStart { zoneProperties.getLoopStart ().value_or (0) };
        const auto loopLength { static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (minZoneProperties.getLoopLength ().value ())) };
        const auto loopEnd { loopStart + loopLength };
        const auto maxLoopStart { loopEnd - 4 };
        //const auto sampleLength { sampleProperties.getLengthInSamples () };
        //     DebugLog ("EditManager::getMaxLoopStart (loopEnd)", "sampleLength: " + juce::String (sampleLength) + ", loopStart: " + juce::String (loopStart) +
        //               ", loopLength: " + juce::String (loopLength) + ", loopEnd: " + juce::String (loopEnd));
        return maxLoopStart;
    }
}

bool EditManager::isSupportedAudioFile (juce::File file)
{
    if (file.isDirectory () || file.getFileExtension ().toLowerCase () != ".wav")
        return false;
    std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;
    // check for any format settings that are unsupported
    if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample < 8 || reader->bitsPerSample > 32) || (reader->numChannels == 0 || reader->numChannels > 2) || (reader->sampleRate > 192000))
        return false;

    return true;
}

bool EditManager::assignSamples (int channelIndex, int zoneIndex, const juce::StringArray& files)
{
    // TODO - should this have been checked prior to this call?
    for (auto fileName : files)
        if (! isSupportedAudioFile (fileName))
            return false;

    const auto initialNumZones { getNumUsedZones (channelIndex) };
    const auto initialEndIndex { initialNumZones - 1 };
    const auto dropZoneStartIndex { zoneIndex };
    const auto dropZoneEndIndex { zoneIndex + files.size () - 1 };
    const auto maxValue { 5.0 };
    const auto minValue { -5.0 };
//     LogMinVoltageDistribution ("  initialNumZones: " + juce::String (initialNumZones));
//     LogMinVoltageDistribution ("  initialEndIndex: " + juce::String (initialEndIndex));
//     LogMinVoltageDistribution ("  numFiles: " + juce::String (files.size ()));
//     LogMinVoltageDistribution ("  dropZoneStartIndex: " + juce::String (dropZoneStartIndex));
//     LogMinVoltageDistribution ("  dropZoneEndIndex: " + juce::String (dropZoneEndIndex));

    // assign the samples
    for (auto filesIndex { 0 }; filesIndex < files.size () && zoneIndex + filesIndex < 8; ++filesIndex)
    {
        juce::File file (files [filesIndex]);
        // if file not in preset folder, then copy
        if (appProperties.getMostRecentFolder () != file.getParentDirectory ().getFullPathName ())
        {
            // TODO handle case where file of same name already exists
            // TODO should copy be moved to a thread?
            file.copyFileTo (juce::File (appProperties.getMostRecentFolder ()).getChildFile (file.getFileName ()));
            // TODO handle failure
        }
        //juce::Logger::outputDebugString ("assigning '" + file.getFileName () + "' to Zone " + juce::String (zoneIndex + filesIndex));
        // assign file to zone
        auto& zoneProperties { zoneAndSamplePropertiesList [channelIndex][zoneIndex + filesIndex].zoneProperties };
        zoneProperties.setSample (file.getFileName (), false);

        // check if stereo and set up right channel
        auto& sampleProperties { zoneAndSamplePropertiesList [channelIndex][zoneIndex + filesIndex].sampleProperties };
        if (sampleProperties.getStatus () == SampleStatus::exists && sampleProperties.getNumChannels () == 2)
        {
            if (auto parentChannelId { channelPropertiesList [channelIndex].getId () }; parentChannelId < 8 && channelPropertiesList [channelIndex].getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
            {
                // NOTE PresetProperties.getChannelVT takes a 0 based index, but Id's are 1 based. and since we want the NEXT channel, we can use the Id, because it is already +1 to the index
                ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelId), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                ZoneProperties nextChannelZoneProperties (nextChannelProperties.getZoneVT (zoneProperties.getId () - 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                // if next Channel does not have a sample
                if (nextChannelZoneProperties.getSample ().isEmpty ())
                {
                    nextChannelProperties.setChannelMode (ChannelProperties::ChannelMode::stereoRight, false);
                    nextChannelZoneProperties.setSide (1, false);
                    nextChannelZoneProperties.setSampleStart (-1, true);
                    nextChannelZoneProperties.setSampleEnd (-1, true);
                    nextChannelZoneProperties.setLoopStart (-1, true);
                    nextChannelZoneProperties.setLoopLength (-1, true);
                    nextChannelZoneProperties.setSample (zoneProperties.getSample (), false); // when the other editor receives this update, it will also update the sample positions, so do it after setting them
                }
            }
        }
    }

    // update the minVoltages if needed
    if (dropZoneEndIndex - initialEndIndex > 0)
    {
        const auto initialValue { dropZoneStartIndex == 0 || (dropZoneStartIndex == 1 && initialNumZones == 1) ? maxValue : zoneAndSamplePropertiesList [channelIndex][initialEndIndex - 1].zoneProperties.getMinVoltage () };
        const auto initialIndex { dropZoneStartIndex > 0 && dropZoneStartIndex == initialNumZones ? dropZoneStartIndex - 1 : dropZoneStartIndex };
        // Calculate the step size for even distribution
        const auto stepSize { (minValue - initialValue) / (dropZoneEndIndex - initialIndex + 1) };
        const auto updateIndexThreshold { initialEndIndex - 1 };

        // Update values for the requested section
        for (int curZoneIndex = initialIndex; curZoneIndex < dropZoneEndIndex; ++curZoneIndex)
        {
            if (curZoneIndex > updateIndexThreshold)
                zoneAndSamplePropertiesList [channelIndex][curZoneIndex].zoneProperties.setMinVoltage (initialValue + (curZoneIndex - initialIndex + 1) * stepSize, false);
        }
    }

    // ensure the last zone is always -5.0
    zoneAndSamplePropertiesList [channelIndex][getNumUsedZones (channelIndex) - 1].zoneProperties.setMinVoltage (minValue, false);
#if JUCE_DEBUG
    // verifying that all minVoltages are valid
    for (auto curZoneIndex { 0 }; curZoneIndex < getNumUsedZones (channelIndex) - 1; ++curZoneIndex)
        if (zoneAndSamplePropertiesList [channelIndex][curZoneIndex].zoneProperties.getMinVoltage () <= zoneAndSamplePropertiesList [channelIndex][curZoneIndex + 1].zoneProperties.getMinVoltage ())
        {
            juce::Logger::outputDebugString ("[" + juce::String (curZoneIndex) + "]=" + juce::String (zoneAndSamplePropertiesList [channelIndex][curZoneIndex].zoneProperties.getMinVoltage ()) +
                " > [" + juce::String (curZoneIndex + 1) + "]=" + juce::String (zoneAndSamplePropertiesList [channelIndex][curZoneIndex + 1].zoneProperties.getMinVoltage ()));
            jassertfalse;
        }
#endif
    return true;
}
