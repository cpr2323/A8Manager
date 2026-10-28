#include "EditManager.h"
#include "SampleManager/SampleManagerProperties.h"
#include "../../../SystemServices.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

EditManager::EditManager ()
{
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

    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    audioManager = systemServices.getAudioManager ();

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
    if (zoneIndex != getNumUsedZones (channelIndex) - 1)
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
        //DebugLog ("EditManager::getMaxLoopStart (loopLength)", "sampleLength: " + juce::String (sampleLength) + ", loopLength: " + juce::String (loopLength) + ", maxLoopStart: " + juce::String (maxLoopStart));
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

bool EditManager::assignSamples (int channelIndex, int zoneIndex, const juce::StringArray& files)
{
    for (auto fileName : files)
        if (! audioManager->isA8ManagerSupportedAudioFile (fileName))
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

    // copy, or convert, the files into the preset folder before changing any zones, so a failure leaves the zones as they were
    const auto presetFolder { juce::File (appProperties.getMostRecentFolder ()) };
    std::vector<juce::File> stagedFiles;
    juce::StringArray stagingMessages;
    auto stagingFailed { false };
    for (auto filesIndex { 0 }; filesIndex < files.size () && zoneIndex + filesIndex < 8; ++filesIndex)
    {
        const auto stagedSample { audioManager->stageSampleForPreset (juce::File (files [filesIndex]), presetFolder) };
        if (stagedSample.errorMessage.isNotEmpty ())
            stagingMessages.add (stagedSample.errorMessage);
        if (stagedSample.file == juce::File ())
            stagingFailed = true;
        stagedFiles.emplace_back (stagedSample.file);
    }
    if (stagingFailed)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Assign Samples Failed",
                                                stagingMessages.joinIntoString ("\n") + "\n\nNo zones were changed.", {}, nullptr, nullptr);
        return false;
    }
    if (! stagingMessages.isEmpty ())
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Assign Samples", stagingMessages.joinIntoString ("\n"), {}, nullptr, nullptr);

    // assign the samples
    for (auto filesIndex { 0 }; filesIndex < static_cast<int> (stagedFiles.size ()); ++filesIndex)
    {
        const auto& file { stagedFiles [filesIndex] };
        //juce::Logger::outputDebugString ("assigning '" + file.getFileName () + "' to Zone " + juce::String (zoneIndex + filesIndex));
        // assign file to zone
        auto& zoneProperties { zoneAndSamplePropertiesList [channelIndex][zoneIndex + filesIndex].zoneProperties };
        auto& sampleProperties { zoneAndSamplePropertiesList [channelIndex][zoneIndex + filesIndex].sampleProperties };
        const auto originalSampleFileName { zoneProperties.getSample () };
        zoneProperties.setSample (file.getFileName (), false);
        zoneProperties.setSide (0, false);
        if (zoneProperties.getSampleStart ().value_or (0) > sampleProperties.getLengthInSamples ())
            zoneProperties.setSampleStart (sampleProperties.getLengthInSamples () - 1, false);
        if (zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) > sampleProperties.getLengthInSamples ())
            zoneProperties.setSampleEnd (sampleProperties.getLengthInSamples () - 1, false);
        if (zoneProperties.getLoopStart ().value_or (0) + 4 > sampleProperties.getLengthInSamples ())
            zoneProperties.setLoopStart (sampleProperties.getLengthInSamples () - 4, false);
        if (zoneProperties.getLoopStart ().value_or (0) + zoneProperties.getLoopLength ().value_or (4) > sampleProperties.getLengthInSamples ())
            zoneProperties.setLoopLength (static_cast<double> (sampleProperties.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0)), false);

        // check if stereo and set up right channel
        if (sampleProperties.getStatus () == SampleStatus::exists)
        {
            auto updateRightChannelZone =[&zoneProperties] (ZoneProperties& rightChannelZoneProperties)
            {
                rightChannelZoneProperties.setSide (1, false);
                rightChannelZoneProperties.setSampleStart (-1, true); // I think this,and the next 3 lines, could pass false for doSelfCallback
                rightChannelZoneProperties.setSampleEnd (-1, true);
                rightChannelZoneProperties.setLoopStart (-1, true);
                rightChannelZoneProperties.setLoopLength (-1, true);
                rightChannelZoneProperties.setSample (zoneProperties.getSample (), false); // when the other editor receives this update, it will also update the sample positions, so do it after setting them

            };
            if (sampleProperties.getNumChannels () == 2)
            {
                if (auto parentChannelId { channelPropertiesList [channelIndex].getId () }; parentChannelId < 8 && channelPropertiesList [channelIndex].getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
                {
                    // NOTE PresetProperties.getChannelVT takes a 0 based index, but Id's are 1 based. and since we want the NEXT channel, we can use the Id, because it is already +1 to the index
                    ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelId), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    ZoneProperties nextChannelZoneProperties (nextChannelProperties.getZoneVT (zoneProperties.getId () - 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    // if next Channel does not have a sample
                    if (nextChannelZoneProperties.getSample () == originalSampleFileName || nextChannelZoneProperties.getSample ().isEmpty ())
                    {
                        nextChannelProperties.setChannelMode (ChannelProperties::ChannelMode::stereoRight, false);
                        updateRightChannelZone (nextChannelZoneProperties);
                    }
                }
            }
            else if (sampleProperties.getNumChannels () == 1)
            {
                if (auto parentChannelId { channelPropertiesList [channelIndex].getId () }; parentChannelId < 8 && channelPropertiesList [channelIndex].getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
                {
                    // NOTE PresetProperties.getChannelVT takes a 0 based index, but Id's are 1 based. and since we want the NEXT channel, we can use the Id, because it is already +1 to the index
                    ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelId), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    ZoneProperties nextChannelZoneProperties (nextChannelProperties.getZoneVT (zoneProperties.getId () - 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    // if next Channel does not have a sample
                    if (nextChannelProperties.getChannelMode () == ChannelProperties::ChannelMode::stereoRight &&
                        (nextChannelZoneProperties.getSample () == originalSampleFileName || nextChannelZoneProperties.getSample ().isEmpty ()))
                    {
                        updateRightChannelZone (nextChannelZoneProperties);
                    }
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
        const auto lastZoneIndex { std::min (dropZoneEndIndex, 8) };
        for (int curZoneIndex { initialIndex }; curZoneIndex < lastZoneIndex; ++curZoneIndex)
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

void EditManager::clearAllZones (int channelIndex)
{
    const auto numZones { getNumUsedZones (channelIndex) };
    for (auto curZoneIndex { 0 }; curZoneIndex < numZones; ++curZoneIndex)
        zoneAndSamplePropertiesList [channelIndex][curZoneIndex].zoneProperties.copyFrom (defaultZoneProperties.getValueTree (), false);
}

std::vector<int> EditManager::getLinkedChannels (int channelIndex, int zoneIndex)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneIndex >= 0 && zoneIndex < 8);
    std::vector<int> channels { channelIndex };
    if (channelIndex < 7 && channelPropertiesList [channelIndex].getChannelMode () != ChannelProperties::ChannelMode::stereoRight &&
        channelPropertiesList [channelIndex + 1].getChannelMode () == ChannelProperties::ChannelMode::stereoRight)
    {
        const auto sample { zoneAndSamplePropertiesList [channelIndex][zoneIndex].zoneProperties.getSample () };
        // if the right channel has a different sample, it is not linked, and is left alone
        if (sample.isNotEmpty () && zoneAndSamplePropertiesList [channelIndex + 1][zoneIndex].zoneProperties.getSample () == sample)
            channels.push_back (channelIndex + 1);
    }
    return channels;
}

void EditManager::deleteZone (int channelIndex, int zoneIndex)
{
    for (const auto curChannelIndex : getLinkedChannels (channelIndex, zoneIndex))
        deleteZoneInChannel (curChannelIndex, zoneIndex);
}

void EditManager::duplicateZone (int channelIndex, int zoneIndex)
{
    for (const auto curChannelIndex : getLinkedChannels (channelIndex, zoneIndex))
        duplicateZoneInChannel (curChannelIndex, zoneIndex);
}

void EditManager::explodeZone (int channelIndex, int zoneIndex, int explodeCount)
{
    for (const auto curChannelIndex : getLinkedChannels (channelIndex, zoneIndex))
        explodeZoneInChannel (curChannelIndex, zoneIndex, explodeCount);
}

void EditManager::deleteZoneInChannel (int channelIndex, int zoneIndex)
{
    auto& zones { zoneAndSamplePropertiesList [channelIndex] };
    zones [zoneIndex].zoneProperties.copyFrom (defaultZoneProperties.getValueTree (), false);
    // if this zone was the last in the list, but not also the first, then set the minVoltage for the new last in list to -5
    if (zoneIndex == getNumUsedZones (channelIndex) && zoneIndex != 0)
        zones [zoneIndex - 1].zoneProperties.setMinVoltage (-5.0, false);
    removeEmptyZones (channelIndex);
}

void EditManager::duplicateZoneInChannel (int channelIndex, int zoneIndex)
{
    jassert (zoneIndex > 0 && zoneIndex < 7);
    auto& zones { zoneAndSamplePropertiesList [channelIndex] };
    const auto topBoundary { zones [zoneIndex - 1].zoneProperties.getMinVoltage () };
    const auto bottomBoundary { zones [zoneIndex].zoneProperties.getMinVoltage () };
    const auto newZoneVoltage { bottomBoundary + ((topBoundary - bottomBoundary) / 2) };
    for (auto curZoneIndex { 6 }; curZoneIndex >= zoneIndex; --curZoneIndex)
        zones [curZoneIndex + 1].zoneProperties.copyFrom (zones [curZoneIndex].zoneProperties.getValueTree (), false);
    zones [zoneIndex].zoneProperties.setMinVoltage (newZoneVoltage, false);
    if (getNumUsedZones (channelIndex) == 8)
        zones [7].zoneProperties.setMinVoltage (-5.0, false);
}

void EditManager::explodeZoneInChannel (int channelIndex, int zoneIndex, int explodeCount)
{
    auto& zones { zoneAndSamplePropertiesList [channelIndex] };
    const juce::int64 sampleSize { zones [zoneIndex].sampleProperties.getLengthInSamples () };
    const auto sliceSize { sampleSize / explodeCount };
    auto& sourceZoneProperties { zones [zoneIndex].zoneProperties };
    auto setSamplePoints = [sliceSize] (ZoneProperties& zpToUpdate, int index)
    {
        const auto sampleStart { index * sliceSize };
        const auto sampleEnd { sampleStart + sliceSize };
        zpToUpdate.setSampleStart (sampleStart, false);
        zpToUpdate.setSampleEnd (sampleEnd, false);
        zpToUpdate.setLoopStart (sampleStart, false);
        zpToUpdate.setLoopLength (static_cast<double> (sliceSize), false);
    };
    setSamplePoints (sourceZoneProperties, 0);

    for (auto destinationZoneIndex { zoneIndex + 1 }; destinationZoneIndex < zoneIndex + explodeCount; ++destinationZoneIndex)
    {
        auto& destZoneProperties { zones [destinationZoneIndex].zoneProperties };
        destZoneProperties.copyFrom (sourceZoneProperties.getValueTree (), false);
        setSamplePoints (destZoneProperties, destinationZoneIndex - zoneIndex);
    }

    // distribute the minVoltages evenly across the 10V range
    const auto numUsedZones { getNumUsedZones (channelIndex) };
    zones [numUsedZones - 1].zoneProperties.setMinVoltage (-5.0, false);
    if (numUsedZones > 1)
    {
        const auto voltageRange { 10.0 / numUsedZones };
        auto curVoltage { -5.0 };
        for (auto curZoneIndex { numUsedZones - 2 }; curZoneIndex >= 0; --curZoneIndex)
        {
            curVoltage += voltageRange;
            zones [curZoneIndex].zoneProperties.setMinVoltage (curVoltage, false);
        }
    }
}

void EditManager::flipZones (int channelIndex, int zoneIndex, int flipCount)
{
    // the right channel is only flipped if every zone in the range is linked to it
    auto channels { getLinkedChannels (channelIndex, zoneIndex) };
    for (auto curZoneIndex { zoneIndex + 1 }; curZoneIndex < zoneIndex + flipCount && channels.size () > 1; ++curZoneIndex)
        if (getLinkedChannels (channelIndex, curZoneIndex).size () < 2)
            channels.pop_back ();
    for (const auto curChannelIndex : channels)
        flipZonesInChannel (curChannelIndex, zoneIndex, flipCount);
}

void EditManager::flipZonesInChannel (int channelIndex, int zoneIndex, int flipCount)
{
    auto& zones { zoneAndSamplePropertiesList [channelIndex] };
    ZoneProperties tempZoneProperties;
    for (auto zoneCount { 0 }; zoneCount < flipCount / 2; ++zoneCount)
    {
        auto& firstZone { zones [zoneIndex + zoneCount].zoneProperties };
        const auto firstZoneMinVoltage { firstZone.getMinVoltage () };
        auto& secondZone { zones [zoneIndex + (flipCount - zoneCount - 1)].zoneProperties };
        const auto secondZoneMinVoltage { secondZone.getMinVoltage () };
        tempZoneProperties.copyFrom (secondZone.getValueTree (), false);
        secondZone.copyFrom (firstZone.getValueTree (), false);
        secondZone.setMinVoltage (secondZoneMinVoltage, false);
        firstZone.copyFrom (tempZoneProperties.getValueTree (), false);
        firstZone.setMinVoltage (firstZoneMinVoltage, false);
    }
}

// TODO - does this function really need to look for multiple empty zones? assuming it gets called when a zone is deleted, there should only be one
void EditManager::removeEmptyZones (int channelIndex)
{
    auto& zones { zoneAndSamplePropertiesList [channelIndex] };
    for (auto zoneIndex { 0 }; zoneIndex < 7; ++zoneIndex)
    {
        if (zones [zoneIndex].zoneProperties.getSample ().isNotEmpty ())
            continue;
        // move the next used zone down into this empty one
        auto moveHappened { false };
        for (auto nextZoneIndex { zoneIndex + 1 }; nextZoneIndex < 8; ++nextZoneIndex)
        {
            auto& nextZoneProperties { zones [nextZoneIndex].zoneProperties };
            if (nextZoneProperties.getSample ().isNotEmpty ())
            {
                zones [zoneIndex].zoneProperties.copyFrom (nextZoneProperties.getValueTree (), false);
                nextZoneProperties.copyFrom (defaultZoneProperties.getValueTree (), false);
                moveHappened = true;
                break;
            }
        }
        // there were none others to move
        if (! moveHappened)
            break;
    }
}
