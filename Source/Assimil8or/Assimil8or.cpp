#include "Assimil8or.h"
#include "Preset/ParameterNames.h"
#include "Validator/ValidatorResultProperties.h"

#define LOG_VALIDATION 0
#if LOG_VALIDATION
#define LogValidation(text) juce::Logger::outputDebugString (text);
#else
#define LogValidation(text) ;
#endif

#define LOG_PARSING 0
#if LOG_PARSING
#define LogParsing(text) juce::Logger::outputDebugString (text);
#else
#define LogParsing(text) ;
#endif

const double oneK { 1024.0f };
const double oneMB { oneK * oneK };
const double oneGB { oneMB * oneK };
const auto maxMemory { static_cast<int> (422 * oneMB) };
const auto maxPresets { 199 };

juce::String getMemorySizeString (uint64_t memoryUsage)
{
    auto formatString = [] (double usage, juce::String postFix)
    {
        return (usage == 0 ? "0" : juce::String (usage, 2).trimCharactersAtEnd ("0.")) + postFix;
    };
    if (memoryUsage >= oneGB)
        return formatString (static_cast<float>(memoryUsage) / oneGB, "GB");
    else if (memoryUsage >= oneMB)
        return formatString (static_cast<float>(memoryUsage) / oneMB, "MB");
    else if (memoryUsage >= oneK)
        return formatString (static_cast<float>(memoryUsage) / oneK, "k");
    else
        return formatString (static_cast<float>(memoryUsage), " bytes");
};

Assimil8orValidator::Assimil8orValidator () : Thread ("Assimil8orValidator")
{
    // initialize format manager for sample file reading
    audioFormatManager.registerBasicFormats ();
}

void Assimil8orValidator::init (juce::ValueTree vt)
{
    validatorProperties.wrap (vt, ValidatorProperties::WrapperType::owner, ValidatorProperties::EnableCallbacks::yes);
    validatorProperties.onStartScanAsync = [this] () { validate (); };
    validatorResultListProperties.wrap (validatorProperties.getValueTree (),
                                        ValidatorResultListProperties::WrapperType::client, ValidatorResultListProperties::EnableCallbacks::no);
}

void Assimil8orValidator::validate ()
{
    validatorProperties.setScanStatus ("scanning", false);
    startThread ();
}

void Assimil8orValidator::addResult (juce::String statusType, juce::String statusText)
{
    ValidatorResultProperties validatorResultProperties;
    validatorResultProperties.update (statusType, statusText, false);
    addResult (validatorResultProperties.getValueTree ());
};

void Assimil8orValidator::addResult (juce::ValueTree validatorResultsVT)
{
    ValidatorResultProperties validatorResultProperties (validatorResultsVT,
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
    jassert (validatorResultProperties.getType () != ValidatorResultProperties::ResultTypeNone);
    if (validatorResultProperties.getType() != ValidatorResultProperties::ResultTypeNone)
        validatorResultListProperties.addResult (validatorResultsVT);
};

void Assimil8orValidator::doIfProgressTimeElapsed (std::function<void ()> functionToDo)
{
    jassert (functionToDo != nullptr);
    if (juce::Time::currentTimeMillis () - lastScanInProgressUpdate > 250)
    {
        lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
        functionToDo ();
    }
}

bool Assimil8orValidator::isAudioFile (juce::File file)
{
    return file.getFileExtension () == ".wav";
}

bool Assimil8orValidator::isPresetFile (juce::File file)
{
    return file.getFileExtension () == ".yml" &&
           file.getFileNameWithoutExtension ().length () == 7 &&
           file.getFileNameWithoutExtension ().startsWith ("prst") &&
           file.getFileNameWithoutExtension ().substring (4).containsOnly ("0123456789");
}

int Assimil8orValidator::getPresetNumberFromName (juce::File file)
{
    if (! isPresetFile (file))
        return 9999;
    return file.getFileNameWithoutExtension ().substring (4).getIntValue ();
}

std::optional<uint64_t> Assimil8orValidator::validateFile (juce::File file, juce::ValueTree validatorResultsVT)
{
    const auto kMaxFileNameLength { 47 };
    ValidatorResultProperties validatorResultProperties (validatorResultsVT,
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);

    auto checkAudioFile = [this] (juce::File file)
    {
        if (file.getFileExtension () == ".wav")
        {
            // file type assimil8or wants
        }
        else
        {
            // possibly an audio file of a format not supported on the Assimil8or
            auto* format { audioFormatManager.findFormatForFileExtension (file.getFileExtension ()) };
            if (format != nullptr)
            {
                // this is a type we can read, so we can offer to convert it
            }
            else
            {
                // this is an unknown file type
            }
        }
    };
    std::optional<uint64_t> optionalPresetInfo;
    LogValidation ("File: " + file.getFileName ());
    if (file.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  File (ignored)");
		validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(ignored)", false);
        return {};
    }
    else if (isPresetFile (file))
    {
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "Preset", false);

        juce::StringArray fileContents;
        file.readLines (fileContents);

        Assimil8orPreset assimil8orPreset;
        assimil8orPreset.parse (fileContents);

        uint64_t sizeRequiredForSamples {};
        PresetProperties presetProperties (assimil8orPreset.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        if (presetProperties.isValid ())
        {
            presetProperties.forEachChannel([this, &file, &validatorResultProperties, &sizeRequiredForSamples] (juce::ValueTree channelVT)
            {
                ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                channelProperties.forEachZone([this, &file, &validatorResultProperties, &sizeRequiredForSamples] (juce::ValueTree zoneVT)
                {
                    ZoneProperties zoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    // TODO - should we do doIfProgressTimeElapsed() here?
                    const auto sampleFileName { zoneProperties.getSample () };
                    const auto sampleFile { file.getParentDirectory ().getChildFile (sampleFileName) };
                    if (! sampleFile.exists ())
                    {
                        // report error
                        validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "['" + sampleFileName + "' does not exist]", false);
                        validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeNotFound, sampleFile.getFullPathName ());
                    }
                    else
                    {
                        // open as audio file, calculate memory requirements
                        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile));
                        if (reader != nullptr)
                        {
                            sizeRequiredForSamples += reader->numChannels * reader->lengthInSamples * 4;
                        }
                        else
                        {
                            LogValidation ("    [ Warning : unknown audio format ]");
                            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "['" + sampleFileName + "' unknown audio format. size = " + juce::String (file.getSize ()) + "]", false);
                        }
                    }
                    return true;
                });
                return true;
            });
        }
        else
        {
            validatorResultProperties.update ("error", "[missing Preset section]", false);
        }
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "RAM: " + getMemorySizeString (sizeRequiredForSamples), false);
        optionalPresetInfo = sizeRequiredForSamples;
        LogValidation ("  File (preset)");
        return optionalPresetInfo;
    }
    else if (isAudioFile (file))
    {
        validatorResultProperties.updateType (ValidatorResultProperties::ResultTypeInfo, false);
        if (file.getFileName ().length () > kMaxFileNameLength)
        {
            LogValidation ("  [ Warning : file name too long ]");
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError,
                                     "[name too long. " + juce::String (file.getFileName ().length ()) + "(length) vs " +
                                       juce::String (kMaxFileNameLength) +"(max)]", false);
            validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeRenameFile, file.getFullPathName ());
        }

        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
        if (reader != nullptr)
        {
            LogValidation ("    Format: " + reader->getFormatName ());
            LogValidation ("    Sample data: " + juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer"));
            LogValidation ("    Bit Depth: " + juce::String (reader->bitsPerSample));
            LogValidation ("    Sample Rate: " + juce::String (reader->sampleRate));
            LogValidation ("    Channels: " + juce::String (reader->numChannels));
            LogValidation ("    Length/Samples: " + juce::String (reader->lengthInSamples));
            LogValidation ("    Length/Time: " + juce::String (reader->lengthInSamples / reader->sampleRate));

            const auto memoryUsage { reader->numChannels * reader->lengthInSamples * 4 };
            const auto sampleRateString { juce::String (reader->sampleRate / 1000.0f, 2).trimCharactersAtEnd ("0.") };
            validatorResultProperties.updateText (juce::String (" {") +
                                         juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer") + ", " +
                                         juce::String (reader->bitsPerSample) + "bits/" + sampleRateString + "k, " +
                                         juce::String (reader->numChannels == 1 ? "mono" : (reader->numChannels == 2 ? "stereo" : juce::String (reader->numChannels) + " channels")) + "}, " +
                                         juce::String (reader->lengthInSamples / reader->sampleRate, 2) + " seconds, " +
                                         "RAM: " + getMemorySizeString (memoryUsage), false);
            auto reportErrorIfTrue = [&validatorResultProperties, &file] (bool conditionalResult, juce::String newText)
            {
                if (conditionalResult)
                {
                    validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, newText, false);
                    // TODO - this will create multiple fixer entries for multiple format mismatches. there should only be one
                    //        I think the fix should be in addFixerEntry, as there should never be more than one type
                    validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeConvert, file.getFullPathName ());
                }
            };
            reportErrorIfTrue (reader->usesFloatingPointData == true, "[sample format must be integer]");
            reportErrorIfTrue (reader->bitsPerSample < 8 || reader->bitsPerSample > 32, "[bit depth must be between 8 and 32]");
            reportErrorIfTrue (reader->numChannels < 1 || reader->numChannels > 2, "[only mono and stereo supported]");
            reportErrorIfTrue (reader->sampleRate > 192000, "[sample rate must not exceed 192k]");
        }
        else
        {
            LogValidation ("    [ Warning : unknown audio format ]");
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "[unknown audio format. size = " + juce::String (file.getSize ()) + "]", false);
        }
        return {};
    }
    else
    {
        LogValidation ("  File (unknown)");
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(unknown file type)", false);
    }

    return {};
}

void Assimil8orValidator::validateFolder (juce::File folder, juce::ValueTree validatorResultsVT)
{
    const auto kMaxFolderNameLength { 31 };
    ValidatorResultProperties validatorResultProperties (validatorResultsVT,
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no );

    LogValidation ("Folder: " + folder.getFileName ());
    if (folder.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  Folder (ignored) : " + folder.getFileName ());
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(ignored)", false);
    }
    else if (folder.getFileName ().length () > kMaxFolderNameLength)
    {
        LogValidation ("  [ Warning : folder name too long ]");
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "[name too long. " +
                                          juce::String (folder.getFileName ().length ()) + "(length) vs " +
                                          juce::String (kMaxFolderNameLength) + "(max)]", false);
        validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeRenameFolder, folder.getFullPathName ());
    }
}

void Assimil8orValidator::processFolder (juce::ValueTree folderVT)
{
    // iterate over file system
    // for directories
    //      report if name over 31 characters
    //      (TODO) report if folder is 2 or more deep in hierarchy
    // for preset files (to be done when the preset file is fully defined)
    //      report if channels * samples * 4 > Assimil8or memory
    //      (TODO) validate preset file data (really only important if it has been edited)
    // for audio files
    //      report if name over 47 characters
    //      report if it does not match supported formats
    // report any other files as unused by assimil8or
    jassert (folderVT.getType ().toString () == "Folder");
    uint64_t totalSizeOfPresets {};
    int numberOfPresets {};
    for (auto folderEntryVT : folderVT)
    {
        if (isThreadRunning () && threadShouldExit ())
            break;

        const auto curEntry { juce::File (folderEntryVT.getProperty ("name")) };
        doIfProgressTimeElapsed ([this, fileName = curEntry.getFileName ()] ()
        {
            juce::MessageManager::callAsync ([this, fileName] ()
            {
                validatorProperties.setProgressUpdate (fileName, false);
            });
        });
        if (curEntry.isDirectory ())
        {
            ValidatorResultProperties validatorResultProperties;
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "Folder: " + curEntry.getFileName (), false);
            validateFolder (curEntry, validatorResultProperties.getValueTree ());
            addResult (validatorResultProperties.getValueTree ());
            processFolder (folderEntryVT);
        }
        else // curEntry is a file
        {
            ValidatorResultProperties validatorResultProperties;
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "File: " + curEntry.getFileName (), false);
            const auto presetUpdate = validateFile (curEntry, validatorResultProperties.getValueTree ());
            if (presetUpdate.has_value ())
            {
                totalSizeOfPresets += presetUpdate.value ();
                ++numberOfPresets;
            }
            addResult (validatorResultProperties.getValueTree ());
        }
    }
    const auto folderName { juce::File (folderVT.getProperty ("name")).getFileName () };
    addResult ("info", "Total RAM Usage for `" + folderName + "': " + getMemorySizeString (totalSizeOfPresets));
    if (totalSizeOfPresets > maxMemory)
        addResult ("error", "[RAM Usage exceeds memory capacity by " + getMemorySizeString (totalSizeOfPresets - maxMemory) + "]");
    if (numberOfPresets > maxPresets)
        addResult ("error", "[Number of Presets (" + juce::String (numberOfPresets) + ") exceeds maximum allowed (" + juce::String (maxPresets) + ")]");
}

juce::ValueTree Assimil8orValidator::getContentsOfFolder (juce::File folder)
{
    juce::ValueTree folderVT {"Folder"};
    folderVT.setProperty ("name", folder.getFullPathName (), nullptr);
    for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
    {
        if (isThreadRunning() && threadShouldExit ())
            break;

        doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] ()
        {
            juce::MessageManager::callAsync ([this, fileName] ()
            {
                validatorProperties.setProgressUpdate (fileName, false);
            });
        });
        if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
        {
            folderVT.addChild (getContentsOfFolder (curFile), -1, nullptr);
        }
        else
        {
            juce::ValueTree fileVT {"File"};
            fileVT.setProperty ("name", curFile.getFullPathName (), nullptr);
            folderVT.addChild (fileVT, -1, nullptr);
        }
    }
    return folderVT;
}


void Assimil8orValidator::validateRootFolder ()
{
    validatorResultListProperties.clear ();
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootFolderName { validatorProperties.getRootFolder () };

    const auto rootEntry { juce::File (rootFolderName) };
    addResult (ValidatorResultProperties::ResultTypeInfo, "Root Folder: " + rootEntry.getFileName ());
    // do one initial progress update to fill in the first one
    juce::MessageManager::callAsync ([this, folderName = rootEntry.getFileName ()] ()
    {
        validatorProperties.setProgressUpdate (folderName, false);
    });

    rootFolderVT = getContentsOfFolder (rootFolderName);
    // FolderVT
    // <Folder name= "">
    //   <Folder name= "">
    //   <File name= "">
    //   <File name= "">
    // </Folder>
    //   <File name= "">
    //   <Folder name= "">
    //   <Folder name= "">
    //
    // TODO sort each Folder as such
    //   Folders
    //   Preset files
    //   Audio files
    //   unknown files
    sortContentsOfFolder (rootFolderVT);
    processFolder (rootFolderVT);
    rootFolderVT = {};

    juce::MessageManager::callAsync ([this] ()
    {
        validatorProperties.setProgressUpdate ("", false);
        validatorProperties.setScanStatus ("idle", false);
    });
}

void Assimil8orValidator::run ()
{
    validateRootFolder ();
}

void Assimil8orValidator::sortContentsOfFolder (juce::ValueTree folderVT)
{
    jassert (folderVT.getType ().toString () == "Folder");

    //   Folders
    //   Preset files
    //   Audio files
    //   unknown files
    struct SectionInfo
    {
        int startIndex { 0 };
        int length { 0 };
    };
    enum SectionIndex
    {
        folders,
        presetFiles,
        audioFiles,
        unknownFiles,
        size
    };
    std::array<SectionInfo, SectionIndex::size> sections;
    const auto numFolderEntries { folderVT.getNumChildren () };

    auto insertSorted = [&sections, &folderVT] (int itemIndex, int sectionIndex)
    {
        jassert (sectionIndex < SectionIndex::size);
        auto& section { sections[sectionIndex] };
        jassert (itemIndex >= section.startIndex + section.length);
        auto startingSectionLength { section.length };
        auto insertItem = [&folderVT, &section, &sections] (int itemIndex, int insertIndex)
        {
            auto tempVT { folderVT.getChild (itemIndex) };
            folderVT.removeChild (itemIndex, nullptr);
            folderVT.addChild (tempVT, insertIndex, nullptr);
            ++section.length;
            for (auto curSectionIndex { 1 }; curSectionIndex < SectionIndex::size; ++curSectionIndex)
                sections [curSectionIndex].startIndex = sections [curSectionIndex - 1].startIndex + sections [curSectionIndex - 1].length;
        };
        for (auto sectionEntryIndex { section.startIndex }; sectionEntryIndex < section.startIndex + section.length; ++sectionEntryIndex)
        {
            if (folderVT.getChild (itemIndex).getProperty ("name").toString () < folderVT.getChild (sectionEntryIndex).getProperty ("name").toString ())
            {
                insertItem (itemIndex, sectionEntryIndex);
                break;
            }
        }
        if (section.length == startingSectionLength)
            insertItem (itemIndex, section.startIndex + section.length);
    };
    for (auto folderIndex { 0 }; folderIndex < numFolderEntries; ++folderIndex)
    {
        auto folderEntryVT { folderVT.getChild (folderIndex) };
        if (folderEntryVT.getType ().toString () == "Folder")
        {
            insertSorted (folderIndex, SectionIndex::folders);
            sortContentsOfFolder (folderEntryVT);
        }
        else if (folderEntryVT.getType ().toString () == "File")
        {
            // alphabetize in these lists
            //   Preset files
            //   Audio files
            //   unknown files
            auto curFile { juce::File (folderEntryVT.getProperty ("name").toString ()) };
            jassert (curFile.exists ());
            if (isPresetFile (curFile))
                insertSorted (folderIndex, SectionIndex::presetFiles);
            else if (isAudioFile (curFile))
                insertSorted (folderIndex, SectionIndex::audioFiles);
            else // unknown file
                insertSorted (folderIndex, SectionIndex::unknownFiles);
        }
        else
        {
            jassertfalse;
        }
    }
}

Assimil8orPreset::Assimil8orPreset ()
{
}

void Assimil8orPreset::write (juce::File presetFile, juce::ValueTree presetPropertiesVT)
{
    jassert (presetPropertiesVT.isValid ());
    PresetProperties presetPropertiesToWrite (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    if (Assimil8orValidator::isPresetFile (presetFile))
    {
        const auto presetNumber { Assimil8orValidator::getPresetNumberFromName (presetFile) };
        if (presetNumber != 9999)
            presetPropertiesToWrite.setIndex (presetNumber, false);
    }

    auto indentAmount { 0 };
    juce::StringArray lines;
    auto addLine = [this, &indentAmount, &lines] (juce::ValueTree vt, juce::Identifier parameterName, juce::String lineToAdd)
    {
        if (vt.hasProperty (parameterName))
            lines.add (juce::String (lineToAdd).paddedLeft (' ', lineToAdd.length () + indentAmount * 2));
    };
    auto hasContent = [] (juce::ValueTree vt)
    {
        auto doChildrenHaveContent = [&vt] ()
        {
            auto contentFound { false };
            ValueTreeHelpers::forEachChild (vt, [&contentFound] (juce::ValueTree child)
            {
                if (child.getNumProperties() != 0)
                {
                    contentFound = true;
                    return false;
                }
                return true;
            });
            return contentFound;
        };
        if (vt.getNumProperties () > 1)
            return true;
        if (vt.getNumChildren () == 0)
            return false;
        return doChildrenHaveContent ();
    };
    addLine (presetPropertiesToWrite.getValueTree (), "_index", Section::PresetId + " " + juce::String (presetPropertiesToWrite.getIndex ()) + " :");
    ++indentAmount;
    addLine (presetPropertiesToWrite.getValueTree (), PresetProperties::NamePropertyId, Parameter::Preset::NameId + " : " + presetPropertiesToWrite.getName ());
    addLine (presetPropertiesToWrite.getValueTree (), PresetProperties::Data2asCVPropertyId, Parameter::Preset::Data2asCVId+ " : " + presetPropertiesToWrite.getData2AsCV ());
    addLine (presetPropertiesToWrite.getValueTree (), PresetProperties::XfadeACVPropertyId, Parameter::Preset::XfadeACVId + " : " + presetPropertiesToWrite.getXfadeACV() );
    addLine (presetPropertiesToWrite.getValueTree (), PresetProperties::XfadeAWidthPropertyId, Parameter::Preset::XfadeAWidthId + " : " + juce::String(presetPropertiesToWrite.getXfadeAWidth (), 2));

    presetPropertiesToWrite.forEachChannel ([this, &hasContent, &addLine, &indentAmount, &presetPropertiesToWrite] (juce::ValueTree channelVT)
    {
        ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        if (hasContent(channelVT))
        {
            const auto channelPropertiesVT { channelProperties.getValueTree () };
            addLine (channelPropertiesVT, "_index", Section::ChannelId + " " + juce::String (channelProperties.getIndex ()) + " :");
            ++indentAmount;
            addLine (channelPropertiesVT, ChannelProperties::AliasingPropertyId, Parameter::Channel::AliasingId + " : " + juce::String (channelProperties.getAliasing ()));
            addLine (channelPropertiesVT, ChannelProperties::AliasingModPropertyId, Parameter::Channel::AliasingModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getAliasingMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::AttackPropertyId, Parameter::Channel::AttackId + " : " + juce::String (channelProperties.getAttack ()));
            addLine (channelPropertiesVT, ChannelProperties::AttackFromCurrentPropertyId, Parameter::Channel::AttackFromCurrentId + " : " + juce::String (channelProperties.getAttackFromCurrent ()));
            addLine (channelPropertiesVT, ChannelProperties::AttackModPropertyId, Parameter::Channel::AttackModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getAttackMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::AutoTriggerPropertyId, Parameter::Channel::AutoTriggerId + " : " + (channelProperties.getAutoTrigger () ? "1" : "0"));
            addLine (channelPropertiesVT, ChannelProperties::BitsPropertyId, Parameter::Channel::BitsId + " : " + juce::String (channelProperties.getBits (), 1));
            addLine (channelPropertiesVT, ChannelProperties::BitsModPropertyId, Parameter::Channel::BitsModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getBitsMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::ChannelModePropertyId, Parameter::Channel::ChannelModeId + " : " + juce::String (channelProperties.getChannelMode ()));
            addLine (channelPropertiesVT, ChannelProperties::ExpAMPropertyId, Parameter::Channel::ExpAMId + " : " + juce::String (channelProperties.getExpAM ()));
            addLine (channelPropertiesVT, ChannelProperties::ExpFMPropertyId, Parameter::Channel::ExpFMId + " : " + juce::String (channelProperties.getExpFM ()));
            addLine (channelPropertiesVT, ChannelProperties::LevelPropertyId, Parameter::Channel::LevelId + " : " + juce::String (channelProperties.getLevel ()));
            addLine (channelPropertiesVT, ChannelProperties::LinAMPropertyId, Parameter::Channel::LinAMId + " : " + juce::String (channelProperties.getLinAM ()));
            addLine (channelPropertiesVT, ChannelProperties::LinAMisExtEnvPropertyId, Parameter::Channel::LinAMisExtEnvId + " : " + (channelProperties.getLinAMisExtEnv () ? "1" : "0"));
            addLine (channelPropertiesVT, ChannelProperties::LinFMPropertyId, Parameter::Channel::LinFMId + " : " + juce::String (channelProperties.getLinFM ()));
            addLine (channelPropertiesVT, ChannelProperties::LoopLengthModPropertyId, Parameter::Channel::LoopLengthModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLoopLengthMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::LoopModePropertyId, Parameter::Channel::LoopModeId + " : " + juce::String (channelProperties.getLoopMode ()));
            addLine (channelPropertiesVT, ChannelProperties::LoopStartModPropertyId, Parameter::Channel::LoopStartModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLoopStartMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::MixLevelPropertyId, Parameter::Channel::MixLevelId + " : " + juce::String (channelProperties.getMixLevel ()));
            addLine (channelPropertiesVT, ChannelProperties::MixModPropertyId, Parameter::Channel::MixModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getMixMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::MixModIsFaderPropertyId, Parameter::Channel::MixModIsFaderId + " : " + (channelProperties.getMixModIsFader () ? "1" : "0"));
            addLine (channelPropertiesVT, ChannelProperties::PanPropertyId, Parameter::Channel::PanId + " : " + juce::String (channelProperties.getPan ()));
            addLine (channelPropertiesVT, ChannelProperties::PanModPropertyId, Parameter::Channel::PanModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPanMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::PhaseCVPropertyId, Parameter::Channel::PhaseCVId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPhaseCV (), 4));
            addLine (channelPropertiesVT, ChannelProperties::PitchPropertyId, Parameter::Channel::PitchId + " : " + juce::String (channelProperties.getPitch ()));
            addLine (channelPropertiesVT, ChannelProperties::PitchCVPropertyId, Parameter::Channel::PitchCVId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPitchCV (), 4));
            addLine (channelPropertiesVT, ChannelProperties::PlayModePropertyId, Parameter::Channel::PlayModeId + " : " + juce::String (channelProperties.getPlayMode ()));
            addLine (channelPropertiesVT, ChannelProperties::PMIndexPropertyId, Parameter::Channel::PMIndexId + " : " + juce::String (channelProperties.getPMIndex ()));
            addLine (channelPropertiesVT, ChannelProperties::PMIndexModPropertyId, Parameter::Channel::PMIndexModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPMIndexMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::PMSourcePropertyId, Parameter::Channel::PMSourceId + " : " + juce::String (channelProperties.getPMSource ()));
            addLine (channelPropertiesVT, ChannelProperties::ReleasePropertyId, Parameter::Channel::ReleaseId + " : " + juce::String (channelProperties.getRelease ()));
            addLine (channelPropertiesVT, ChannelProperties::ReleaseModPropertyId, Parameter::Channel::ReleaseModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getReleaseMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::ReversePropertyId, Parameter::Channel::ReverseId + " : " + (channelProperties.getReverse () ? "1" : "0"));
            addLine (channelPropertiesVT, ChannelProperties::SampleStartModPropertyId, Parameter::Channel::SampleStartModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getSampleStartMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::SampleEndModPropertyId, Parameter::Channel::SampleEndModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getSampleEndMod (), 4));
            addLine (channelPropertiesVT, ChannelProperties::SpliceSmoothingPropertyId, Parameter::Channel::SpliceSmoothingId + " : " + (channelProperties.getSpliceSmoothing () ? "1" : "0"));
            addLine (channelPropertiesVT, ChannelProperties::XfadeGroupPropertyId, Parameter::Channel::XfadeGroupId + " : " + channelProperties.getXfadeGroup ());
            addLine (channelPropertiesVT, ChannelProperties::ZonesCVPropertyId, Parameter::Channel::ZonesCVId + " : " + channelProperties.getZonesCV ());
            addLine (channelPropertiesVT, ChannelProperties::ZonesRTPropertyId, Parameter::Channel::ZonesRTId + " : " + juce::String (channelProperties.getZonesRT ()));

            channelProperties.forEachZone ([this, &hasContent, &indentAmount, &addLine] (juce::ValueTree zoneVT)
            {
                ZoneProperties zoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                if (hasContent(zoneVT))
                {
                    const auto zonePropertiesVT { zoneProperties.getValueTree () };
                    addLine (zonePropertiesVT, "_index", Section::ZoneId + " " + juce::String (zoneProperties.getIndex ()) + " :");
                    ++indentAmount;
                    addLine (zonePropertiesVT, ZoneProperties::LevelOffsetPropertyId, Parameter::Zone::LevelOffsetId + " : " + juce::String (zoneProperties.getLevelOffset ()));
                    addLine (zonePropertiesVT, ZoneProperties::LoopLengthPropertyId, Parameter::Zone::LoopLengthId + " : " + juce::String (zoneProperties.getLoopLength ()));
                    addLine (zonePropertiesVT, ZoneProperties::LoopStartPropertyId, Parameter::Zone::LoopStartId + " : " + juce::String (zoneProperties.getLoopStart ()));
                    addLine (zonePropertiesVT, ZoneProperties::MinVoltagePropertyId, Parameter::Zone::MinVoltageId + " : " + juce::String (zoneProperties.getMinVoltage ()));
                    addLine (zonePropertiesVT, ZoneProperties::PitchOffsetPropertyId, Parameter::Zone::PitchOffsetId + " : " + juce::String (zoneProperties.getPitchOffset ()));
                    addLine (zonePropertiesVT, ZoneProperties::SamplePropertyId, Parameter::Zone::SampleId + " : " + zoneProperties.getSample ());
                    addLine (zonePropertiesVT, ZoneProperties::SampleStartPropertyId, Parameter::Zone::SampleStartId + " : " + juce::String (zoneProperties.getSampleStart ()));
                    addLine (zonePropertiesVT, ZoneProperties::SampleEndPropertyId, Parameter::Zone::SampleEndId + " : " + juce::String (zoneProperties.getSampleEnd ()));
                    addLine (zonePropertiesVT, ZoneProperties::SidePropertyId, Parameter::Zone::SideId + " : " + juce::String (zoneProperties.getSide ()));
                    --indentAmount;
                }
                return true;
            });
            --indentAmount;
        }
        return true;
    });

    // write data out to preset file
    const auto stringToWrite { lines.joinIntoString ("\r\n") };
    presetFile.replaceWithText (stringToWrite);
}

void Assimil8orPreset::write (juce::File presetFile)
{
    write (presetFile, presetProperties.getValueTree ());
}

// NOTE: still very much under construction
void Assimil8orPreset::parse (juce::StringArray presetLines)
{
    jassert (presetProperties.isValid ());

    auto scopeDepth { 0 };

    juce::ValueTree curPresetSection;
    ChannelProperties channelProperties;
    juce::ValueTree curChannelSection;
    ZoneProperties zoneProperties;
    juce::ValueTree curZoneSection;
    for (auto& presetLine : presetLines)
    {
        const auto indent { presetLine.initialSectionContainingOnly (" ") };
        const auto previousScopeDepth { scopeDepth };
        scopeDepth = indent.length () / 2;
        const auto scopeDifference { scopeDepth - previousScopeDepth };
        // check if we are exiting scope(s)
        if (scopeDifference < 0)
        {
            // for each scope we are exiting, reset the appropriate parent objects
            for (auto remainingScopes { scopeDifference * -1 }; remainingScopes > 0; --remainingScopes)
            {
                switch (parseState)
                {
                    case ParseState::ParsingGlobalSection: { jassertfalse; } break;
                    case ParseState::ParsingPresetSection: { curPresetSection = {}; setParseState (ParseState::ParsingGlobalSection); } break;
                    case ParseState::ParsingChannelSection: { curChannelSection = {}; setParseState (ParseState::ParsingPresetSection); } break;
                    case ParseState::ParsingZoneSection: { curZoneSection = {}; setParseState (ParseState::ParsingChannelSection); } break;
                    default: { jassertfalse; } break;
                }
            }
        }

        LogParsing (juce::String (scopeDepth) + "-" + presetLine.trimStart ());
        const auto key { presetLine.upToFirstOccurrenceOf (":", false, false).trim () };
        const auto value { presetLine.fromFirstOccurrenceOf (":", false, false).trim () };

        auto keyIs = [&key] (const juce::String& desiredKey)
        {
            return key.upToFirstOccurrenceOf (" ", false, false) == desiredKey;
        };
        auto getParameterIndex = [&key] ()
        {
            return key.fromFirstOccurrenceOf (" ", false, false).getIntValue ();
        };
        switch (parseState)
        {
            case ParseState::ParsingGlobalSection:
            {
                if (keyIs (Section::PresetId))
                {
                    curPresetSection = presetProperties.getValueTree ();
                    setParseState (ParseState::ParsingPresetSection);
                }
            }
            break;
            case ParseState::ParsingPresetSection:
            {
                // Data2asCV : 1A
                // Name: template001 (max len?)
                // XfadeACV: 1A
                // XfadeAWidth : 9.10
                // XfadeBCV: 1A
                // XfadeBWidth : 9.10
                // XfadeCCV: 1A
                // XfadeCWidth : 9.10
                // XfadeDCV: 1A
                // XfadeDWidth : 9.10
                if (keyIs (Section::ChannelId))
                {
                    // TODO - do we need to check for malformed data, i.e. more than 8 channels
                    curChannelSection = presetProperties.addChannel (getParameterIndex ());
                    channelProperties.wrap (curChannelSection, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    setParseState (ParseState::ParsingChannelSection);
                }
                else if (keyIs (Parameter::Preset::NameId))
                {
                    presetProperties.setName (value, false);
                }
                else if (keyIs (Parameter::Preset::Data2asCVId))
                {
                    presetProperties.setData2AsCV (value, false);
                }
                else if (keyIs (Parameter::Preset::XfadeACVId))
                {
                    presetProperties.setXfadeACV (value, false);
                }
                else if (keyIs (Parameter::Preset::XfadeAWidthId))
                {
                    presetProperties.setXfadeAWidth (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Preset::XfadeBCVId))
                {
                    presetProperties.setXfadeBCV (value, false);
                }
                else if (keyIs (Parameter::Preset::XfadeBWidthId))
                {
                    presetProperties.setXfadeBWidth (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Preset::XfadeCCVId))
                {
                    presetProperties.setXfadeCCV (value, false);
                }
                else if (keyIs (Parameter::Preset::XfadeCWidthId))
                {
                    presetProperties.setXfadeCWidth (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Preset::XfadeDCVId))
                {
                    presetProperties.setXfadeDCV (value, false);
                }
                else if (keyIs (Parameter::Preset::XfadeDWidthId))
                {
                    presetProperties.setXfadeDWidth (value.getDoubleValue (), false);
                }
                else
                {
                    LogParsing ("unknown preset key: " + presetLine);
                    //jassertfalse;
                }
            }
            break;
            case ParseState::ParsingChannelSection:
            {
                // Aliasing : 100
                // AliasingMod : 0B 0.92
                // Attack :  0.0006
                // AttackFromCurrent : 1
                // AttackMod : 0B 0.82
                // AutoTrigger : 1
                // Bits : 01.0
                // BitsMod : Off 0.00
                // ChannelMode : 2
                // ExpAM : 0A 1.00
                // ExpFM : 0A 1.00
                // Level : -10.0
                // LinAM : 0A -1.00
                // LinAMisExtEnv : 1
                // LinFM : 0A -1.00
                // LoopLengthMod : 0B -0.26
                // LoopMode : 1
                // LoopStartMod : 0C 0.00
                // MixLevel : -90.0
                // MixMod : 0A 0.93
                // MixModIsFader : 1
                // Pan : -0.30
                // PanMod : Off 0.00
                // PhaseCV : 0A 1.00
                // Pitch : -16.79
                // PitchCV : 0A 0.50
                // PlayMode : 1
                // PMIndex : 0.12
                // PMIndexMod : 0C 0.18
                // PMSource : 8
                // Release : 99.0000
                // ReleaseMod : 0C 1.00
                // Reverse : 1
                // SampleStartMod : 0B 1.00
                // SampleEndMod : 0A 0.66
                // SpliceSmoothing : 1
                // XfadeGroup : A
                // ZonesCV : 0B
                // ZonesRT : 1
                if (keyIs (Section::ZoneId))
                {
                    // TODO - do we need to check for malformed data, ie more than 8 zones
                    curZoneSection = channelProperties.addZone (getParameterIndex ());
                    zoneProperties.wrap (curZoneSection, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    setParseState (ParseState::ParsingZoneSection);
                }
                else if (keyIs (Parameter::Channel::AttackId))
                {
                    channelProperties.setAttack (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::AttackFromCurrentId))
                {
                    channelProperties.setAttackFromCurrent (value.getIntValue () == 1, false);
                }
                else if (keyIs (Parameter::Channel::AttackModId))
                {
                    auto [cvInput, attackModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setAttackMod (cvInput, attackModAmount, false);
                }
                else if (keyIs (Parameter::Channel::AliasingId))
                {
                    channelProperties.setAliasing (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Channel::AliasingModId))
                {
                    auto [cvInput, aliasingModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setAliasingMod (cvInput, aliasingModAmount, false);
                }
                else if (keyIs (Parameter::Channel::AutoTriggerId))
                {
                    channelProperties.setAutoTrigger (value.getIntValue () == 1, false);
                }
                else if (keyIs (Parameter::Channel::BitsId))
                {
                    channelProperties.setBits (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::BitsModId))
                {
                    auto [cvInput, bitsModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setBitsMod (cvInput, bitsModAmount, false);
                }
                else if (keyIs (Parameter::Channel::ChannelModeId))
                {
                    channelProperties.setChannelMode (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Channel::ExpAMId))
                {
                    channelProperties.setExpAM (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::ExpFMId))
                {
                    channelProperties.setExpFM (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::LevelId))
                {
                    channelProperties.setLevel (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::LinAMId))
                {
                    channelProperties.setLinAM (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::LinAMisExtEnvId))
                {
                    channelProperties.setLinAMisExtEnv (value.getIntValue () == 1, false);
                }
                else if (keyIs (Parameter::Channel::LinFMId))
                {
                    channelProperties.setLinFM (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::LoopLengthModId))
                {
                    auto [cvInput, loopLengthModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setLoopLengthMod (cvInput, loopLengthModAmount, false);
                }
                else if (keyIs (Parameter::Channel::LoopModeId))
                {
                    channelProperties.setLoopMode (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Channel::LoopStartModId))
                {
                    auto [cvInput, loopStartModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setLoopStartMod (cvInput, loopStartModAmount, false);
                }
                else if (keyIs (Parameter::Channel::MixLevelId))
                {
                    channelProperties.setMixLevel (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::MixModId))
                {
                    auto [cvInput, mixModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setMixMod (cvInput, mixModAmount, false);
                }
                else if (keyIs (Parameter::Channel::MixModIsFaderId))
                {
                    channelProperties.setMixModIsFader (value.getIntValue () == 1, false);
                }
                else if (keyIs (Parameter::Channel::PanId))
                {
                    channelProperties.setPan (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::PanModId))
                {
                    auto [cvInput, panModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setPanMod (cvInput, panModAmount, false);
                }
                else if (keyIs (Parameter::Channel::PhaseCVId))
                {
                    auto [cvInput, phaseCvAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setPhaseCV (cvInput, phaseCvAmount, false);
                }
                else if (keyIs (Parameter::Channel::PitchId))
                {
                    channelProperties.setPitch (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::PitchCVId))
                {
                    auto [cvInput, pitchCvAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setPitchCV (cvInput, pitchCvAmount, false);
                }
                else if (keyIs (Parameter::Channel::PlayModeId))
                {
                    channelProperties.setPlayMode (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Channel::PMIndexId))
                {
                    channelProperties.setPMIndex (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::PMIndexModId))
                {
                    auto [cvInput, pMIndexModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setPMIndexMod (cvInput, pMIndexModAmount, false);
                }
                else if (keyIs (Parameter::Channel::PMSourceId))
                {
                    channelProperties.setPMSource (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Channel::ReleaseId))
                {
                    channelProperties.setRelease (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Channel::ReleaseModId))
                {
                    auto [cvInput, releaseModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setReleaseMod (cvInput, releaseModAmount, false);
                }
                else if (keyIs (Parameter::Channel::ReverseId))
                {
                    channelProperties.setReverse (value.getIntValue () == 1, false);
                }
                else if (keyIs (Parameter::Channel::SampleEndModId))
                {
                    auto [cvInput, sampleEndModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setSampleEndMod (cvInput, sampleEndModAmount, false);
                }
                else if (keyIs (Parameter::Channel::SampleStartModId))
                {
                    auto [cvInput, sampleStartModAmount] = ChannelProperties::getCvInputAndValueFromString (value);
                    channelProperties.setSampleStartMod (cvInput, sampleStartModAmount, false);
                }
                else if (keyIs (Parameter::Channel::SpliceSmoothingId))
                {
                    channelProperties.setSpliceSmoothing (value.getIntValue () == 1, false);
                }
                else if (keyIs (Parameter::Channel::XfadeGroupId))
                {
                    channelProperties.setXfadeGroup (value, false);
                }
                else if (keyIs (Parameter::Channel::ZonesCVId))
                {
                    channelProperties.setZonesCV (value, false);
                }
                else if (keyIs (Parameter::Channel::ZonesRTId))
                {
                    channelProperties.setZonesRT (value.getIntValue (), false);
                }
                else
                {
                    LogParsing ("unknown channel key: " + presetLine);
                    //jassertfalse;
                }
            }
            break;
            case ParseState::ParsingZoneSection:
            {
                // LevelOffset : -6.3
                // LoopLength : 256.0000
                // LoopStart : 111683
                // MinVoltage : +4.56
                // PitchOffset : +2.00
                // Sample : sampleA.wav
                // SampleStart : 79416
                // SampleEnd : 6058
                // Side : 1
                if (keyIs (Parameter::Zone::LevelOffsetId))
                {
                    zoneProperties.setLevelOffset (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Zone::LoopLengthId))
                {
                    zoneProperties.setLoopLength (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Zone::LoopStartId))
                {
                    zoneProperties.setLoopStart (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Zone::MinVoltageId))
                {
                    zoneProperties.setMinVoltage (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Zone::PitchOffsetId))
                {
                    zoneProperties.setPitchOffset (value.getDoubleValue (), false);
                }
                else if (keyIs (Parameter::Zone::SampleId))
                {
                    zoneProperties.setSample (value, false);
                }
                else if (keyIs (Parameter::Zone::SampleStartId))
                {
                    zoneProperties.setSampleStart (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Zone::SampleEndId))
                {
                    zoneProperties.setSampleEnd (value.getIntValue (), false);
                }
                else if (keyIs (Parameter::Zone::SideId))
                {
                    zoneProperties.setSide (value.getIntValue (), false);
                }
                else
                {
                    LogParsing ("unknown zone key: " + presetLine);
                    //jassertfalse;
                }
            }
            break;
            default:
                jassertfalse;
                break;
        }
    }
}

juce::String Assimil8orPreset::getParseStateString (ParseState theParseState)
{
    switch (theParseState)
    {
        case ParseState::ParsingGlobalSection: { return "ParsinGlobalSection"; } break;
        case ParseState::ParsingPresetSection: { return "ParsingPresetSection"; } break;
        case ParseState::ParsingChannelSection: { return "ParsingChannelSection"; } break;
        case ParseState::ParsingZoneSection: { return "ParsingZoneSection"; } break;
        default: { return "[error]"; } break;
    }
};

void Assimil8orPreset::setParseState (ParseState newParseState)
{
    parseState = newParseState;
    // juce::Logger::outputDebugString ("new state: " + getParseStateString (parseState));
}
