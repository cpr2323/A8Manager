#include "EditManager.h"
#include "../../../Utility/PersistentRootProperties.h"

EditManager::EditManager ()
{
    audioFormatManager.registerBasicFormats ();
}

void EditManager::init (juce::ValueTree rootPropertiesVT, juce::ValueTree presetPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    presetProperties.wrap (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    presetProperties.forEachChannel ([this] (juce::ValueTree channelVT, int channelIndex)
    {
        channelPropertiesList [channelIndex].wrap (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        channelPropertiesList [channelIndex].forEachZone ([this, channelIndex] (juce::ValueTree zoneVT, int zoneIndex)
        {
            zonePropertiesList [channelIndex][zoneIndex].wrap (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
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

void EditManager::forChannels (std::vector<int> channelIndexList, std::function<void (juce::ValueTree)> channelCallback)
{
    jassert (channelCallback != nullptr);
    for (const auto channelIndex : channelIndexList)
    {
        jassert (channelIndex >= 0 && channelIndex < 8);
        channelCallback (channelPropertiesList [channelIndex].getValueTree ());
    }
}

void EditManager::forZones (int channelIndex, std::vector<int> zoneIndexList, std::function<void (juce::ValueTree)> zoneCallback)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneCallback != nullptr);
    for (const auto zoneIndex : zoneIndexList)
    {
        jassert (zoneIndex >= 0 && zoneIndex < 8);
        zoneCallback (zonePropertiesList [channelIndex][zoneIndex].getValueTree ());
    }
}

int EditManager::getNumUsedZones (int channelIndex)
{
    jassert (channelIndex < 8);
    auto numUsedZones { 0 };
    for (auto curZoneIndex { 0 }; curZoneIndex < zonePropertiesList [channelIndex].size (); ++curZoneIndex)
        numUsedZones += zonePropertiesList [channelIndex][curZoneIndex].getSample ().isNotEmpty () ? 1 : 0;
    return numUsedZones;
};

std::tuple<double, double> EditManager::getVoltageBoundaries (int channelIndex, int zoneIndex, int topDepth)
{
    auto topBoundary { 5.0 };
    auto bottomBoundary { -5.0 };

    // neither index 0 or 1 can look at the 'top boundary' index (ie. index - 2, the previous previous one)
    if (zoneIndex > topDepth)
        topBoundary = zonePropertiesList [channelIndex][zoneIndex - topDepth - 1].getMinVoltage ();
    if (zoneIndex < getNumUsedZones (channelIndex) - 1)
        bottomBoundary = zonePropertiesList [channelIndex][zoneIndex + 1].getMinVoltage ();
    return { topBoundary, bottomBoundary };
};


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
    const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (channelIndex, zoneIndex, 0) };
    return std::clamp (voltage, bottomBoundary + 0.01, topBoundary - 0.01);
};

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
        auto& zoneProperty { zonePropertiesList [channelIndex][zoneIndex + filesIndex] };
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
        // TODO - this should be a call to loadZone, which will call zoneProperty.setSample 
        zoneProperty.setSample (file.getFileName (), false);
    }

    // update the minVoltages if needed
    if (dropZoneEndIndex - initialEndIndex > 0)
    {
        const auto initialValue { dropZoneStartIndex == 0 || (dropZoneStartIndex == 1 && initialNumZones == 1) ? maxValue : zonePropertiesList [channelIndex][initialEndIndex - 1].getMinVoltage () };
        const auto initialIndex { dropZoneStartIndex > 0 && dropZoneStartIndex == initialNumZones ? dropZoneStartIndex - 1 : dropZoneStartIndex };
        // Calculate the step size for even distribution
        const auto stepSize { (minValue - initialValue) / (dropZoneEndIndex - initialIndex + 1) };
        const auto updateIndexThreshold { initialEndIndex - 1 };

        // Update values for the requested section
        for (int curZoneIndex = initialIndex; curZoneIndex < dropZoneEndIndex; ++curZoneIndex)
        {
            if (curZoneIndex > updateIndexThreshold)
                zonePropertiesList [channelIndex][curZoneIndex].setMinVoltage (initialValue + (curZoneIndex - initialIndex + 1) * stepSize, false);
        }
    }

    // ensure the last zone is always -5.0
    zonePropertiesList [channelIndex][getNumUsedZones (channelIndex) - 1].setMinVoltage (minValue, false);
#if JUCE_DEBUG
    // verifying that all minVoltages are valid
    for (auto curZoneIndex { 0 }; curZoneIndex < getNumUsedZones (channelIndex) - 1; ++curZoneIndex)
        if (zonePropertiesList [channelIndex][curZoneIndex].getMinVoltage () <= zonePropertiesList [channelIndex][curZoneIndex + 1].getMinVoltage ())
        {
            juce::Logger::outputDebugString ("[" + juce::String (curZoneIndex) + "]=" + juce::String (zonePropertiesList [channelIndex][curZoneIndex].getMinVoltage ()) +
                " > [" + juce::String (curZoneIndex + 1) + "]=" + juce::String (zonePropertiesList [channelIndex][curZoneIndex + 1].getMinVoltage ()));
            jassertfalse;
        }
#endif
//    updateAllZoneTabNames ();
    return true;
}

// TODO - the functionallity of assigning the second channel of a stereo wave file to the next channel needs to be moved elsewhere
#if 0
void EditManager::loadSample (int channelIndex, int zoneIndex, juce::String sampleFileName)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneIndex >= 0 && zoneIndex < 8);
    //     if (sampleFileName == currentSampleFileName)
//         return;
//    currentSampleFileName = sampleFileName;

    auto& zoneProperties { zonePropertiesList [channelIndex][zoneIndex]};
    auto sampleData = [this, sampleFileName] ()
    {
        if (sampleFileName.isNotEmpty ())
            return samplePool->open (sampleFileName);
        else
            return SampleData ();
    } ();

    if (sampleData.getStatus () == SampleData::SampleDataStatus::exists)
    {

        zoneProperties.setSampleStart (-1, true);
        zoneProperties.setSampleEnd (-1, true);
        zoneProperties.setLoopStart (-1, true);
        zoneProperties.setLoopLength (-1, true);
        zoneProperties.setSample (sampleFileName, false);

        if (sampleData.getNumChannels () == 2)
        {
            // if this zone not the last channel && the parent channel isn't set to Stereo/Right
            if (auto parentChannelId { channelPropertiesList [channelIndex].getId ()}; parentChannelId < 8 && channelPropertiesList [channelIndex].getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
            {
                // NOTE PresetProperties.getChannelVT takes a 0 based index, but Id's are 1 based. and since we want the NEXT channel, we can use the Id, because it is already +1 to the index
                ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelId), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                ZoneProperties nextChannelZone1Properties (nextChannelProperties.getZoneVT (zoneProperties.getId () - 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                // if next Channel does not have a sample
                if (nextChannelZone1Properties.getSample ().isEmpty ())
                {
                    nextChannelProperties.setChannelMode (ChannelProperties::ChannelMode::stereoRight, false);
                    nextChannelZone1Properties.setSide (1, false);
                    nextChannelZone1Properties.setSampleStart (-1, true);
                    nextChannelZone1Properties.setSampleEnd (-1, true);
                    nextChannelZone1Properties.setLoopStart (-1, true);
                    nextChannelZone1Properties.setLoopLength (-1, true);
                    nextChannelZone1Properties.setSample (sampleFileName, false); // when the other editor receives this update, it will also update the sample positions, so do it after setting them
                }
            }
        }
    }

//     updateLoopPointsView ();
//     updateSamplePositionInfo ();
//     sampleUiChanged (sampleFileName);
//     sampleNameSelectLabel.setText (sampleFileName, juce::NotificationType::dontSendNotification);
//     sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
//     const auto sampleCanBePlayed { sampleData.getStatus () == SampleData::SampleDataStatus::exists };
//     oneShotPlayButton.setEnabled (sampleCanBePlayed);
//     loopPlayButton.setEnabled (sampleCanBePlayed);
// 
//     if (onSampleChange != nullptr)
//         onSampleChange (sampleFileName);
}
#endif