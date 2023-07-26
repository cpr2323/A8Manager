#include "Assimil8or.h"
#include "Preset/ParameterNames.h"

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

class ScanStatusResult
{
public:
    ScanStatusResult () = default;

    void reset ()
    {
        curType = {};
        curText = {};
    }

    juce::String getType ()
    {
        return curType;
    }

    juce::String getText ()
    {
        return curText;
    }

    void update (const juce::String& newType, const juce::String& newText)
    {
        updateType (newType);
        udpateText (newText);
    }

    void updateType (const juce::String& newType)
    {
        if (newType == "" || curType == "error")
            return;

        if (curType == "")
        {
            curType = newType;
        }
        else if (curType == "info")
        {
            if (newType != "")
                curType = newType;
        }
        else if (curType == "warning")
        {
            if (newType != "info")
                curType = newType;
        }
        else
        {
            jassertfalse;
        }
    }

    void udpateText (const juce::String& newText)
    {
        if (newText.isEmpty ())
            return;

        if (curText.isNotEmpty ())
            curText += ", ";

        curText += newText;
    }
private:
    juce::String curType;
    juce::String curText;
};

Assimil8orSDCardValidator::Assimil8orSDCardValidator () : Thread ("Assimil8orSDCardValidator")
{
    // initialize format manager for sample file reading
    audioFormatManager.registerBasicFormats ();
}

void Assimil8orSDCardValidator::init (juce::ValueTree vt)
{
    validatorProperties.wrap (vt, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::yes);
    validatorProperties.onStartScanAsync = [this] () { validate (); };
}

void Assimil8orSDCardValidator::validate ()
{
    validatorProperties.setScanStatus ("scanning", false);
    startThread ();
}

void Assimil8orSDCardValidator::addStatus (juce::String statusType, juce::String statusText)
{
    auto newStatusChild { juce::ValueTree {"Status"} };
    newStatusChild.setProperty ("type", statusType, nullptr);
    newStatusChild.setProperty ("text", statusText, nullptr);
    validatorProperties.getValidationStatusVT ().addChild (newStatusChild, -1, nullptr);
};

void Assimil8orSDCardValidator::doIfProgressTimeElapsed (std::function<void ()> functionToDo)
{
    jassert (functionToDo != nullptr);
    if (juce::Time::currentTimeMillis () - lastScanInProgressUpdate > 250)
    {
        lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
        functionToDo ();
    }
}

bool Assimil8orSDCardValidator::isAudioFile (juce::File file)
{
    return file.getFileExtension () == ".wav";
}

bool Assimil8orSDCardValidator::isPresetFile (juce::File file)
{
    return file.getFileExtension () == ".yml" &&
           file.getFileNameWithoutExtension ().length () == 7 &&
           file.getFileNameWithoutExtension ().startsWith ("prst") &&
           file.getFileNameWithoutExtension ().substring (4).containsOnly ("0123456789");

}

std::tuple<juce::String, juce::String, std::optional<uint64_t>> Assimil8orSDCardValidator::validateFile (juce::File file)
{
    const auto kMaxFileNameLength { 47 };

    std::optional<uint64_t> optionalPresetInfo;
    LogValidation ("File: " + file.getFileName ());
    if (file.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  File (ignored)");
        return { "warning", "(ignored)", {} };
    }
    else if (isPresetFile (file))
    {
        ScanStatusResult scanStatusResult;
        scanStatusResult.update ("info", "Preset");

        juce::StringArray fileContents;
        file.readLines (fileContents);

        Assimil8orPreset assimil8orPreset;
        assimil8orPreset.parse (fileContents);

        uint64_t sizeRequiredForSamples {};
        const auto presetVT { assimil8orPreset.getPresetVT ().getChildWithName ("Preset") };
        if (presetVT.isValid ())
        {
            ValueTreeHelpers::forEachChildOfType (presetVT, "Channel", [this, &file, &scanStatusResult, &sizeRequiredForSamples] (juce::ValueTree child)
            {
                ValueTreeHelpers::forEachChildOfType (child, "Zone", [this, &file, &scanStatusResult, &sizeRequiredForSamples] (juce::ValueTree child)
                {
                    // TODO - should we do doIfProgressTimeElapsed() here?
                    const auto sampleFileName { child.getProperty ("Sample").toString () };
                    const auto sampleFile { file.getParentDirectory ().getChildFile (sampleFileName) };
                    if (! sampleFile.exists ())
                    {
                        // report error
                        scanStatusResult.update ("error", "['" + sampleFileName + "' does not exist]");
                    }
                    else
                    {
                        // open as audio file, calculate memory requirements
                        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile));
                        if (reader == nullptr)
                        {
                            LogValidation ("    [ Warning : unknown audio format ]");
                            scanStatusResult.update ("error", "['" + sampleFileName + "' unknown audio format. size = " + juce::String (file.getSize ()) + "]");
                        }
                        else
                        {
                            sizeRequiredForSamples += reader->numChannels * reader->lengthInSamples * 4;
                        }
                    }
                    return true;
                });
                return true;
            });
        }
        else
        {
            scanStatusResult.update ("error", "[missing Preset section]");
        }
        scanStatusResult.update ("info", "RAM: " + getMemorySizeString (sizeRequiredForSamples));
        optionalPresetInfo = sizeRequiredForSamples;
        LogValidation ("  File (preset)");
        return { scanStatusResult.getType (), scanStatusResult.getText (), optionalPresetInfo };
    }
    else if (isAudioFile (file))
    {
        ScanStatusResult scanStatusResult;
        scanStatusResult.updateType ("info");
        if (file.getFileName ().length () > kMaxFileNameLength)
        {
            LogValidation ("  [ Warning : file name too long ]");
            scanStatusResult.update ("error",
                                     "[name too long. " + juce::String (file.getFileName ().length ()) + "(length) vs " +
                                       juce::String (kMaxFileNameLength) +"(max)]");
        }

        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
        if (reader == nullptr)
        {
            LogValidation ("    [ Warning : unknown audio format ]");
            scanStatusResult.update ("error", "[unknown audio format. size = " + juce::String (file.getSize ()) + "]");
        }
        else
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
            scanStatusResult.udpateText (juce::String (" {") +
                                         juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer") + ", " +
                                         juce::String (reader->bitsPerSample) + "bits/" + sampleRateString + "k, " +
                                         juce::String (reader->numChannels == 1 ? "mono" : (reader->numChannels == 2 ? "stereo" : juce::String (reader->numChannels) + " channels")) + "}, " +
                                         juce::String (reader->lengthInSamples / reader->sampleRate, 2) + " seconds, " +
                                         "RAM: " + getMemorySizeString (memoryUsage));
            auto reportErrorIfTrue = [&scanStatusResult] (bool conditionalResult, juce::String newText)
            {
                if (conditionalResult)
                    scanStatusResult.update ("error", newText);
            };
            reportErrorIfTrue (reader->usesFloatingPointData == true, "[sample format must be integer]");
            reportErrorIfTrue (reader->bitsPerSample < 8 || reader->bitsPerSample > 32, "[bit depth must be between 8 and 32]");
            reportErrorIfTrue (reader->numChannels < 1 || reader->numChannels > 2, "[only mono and stereo supported]");
            reportErrorIfTrue (reader->sampleRate > 192000, "[sample rate must not exceed 192k]");
        }
        return { scanStatusResult.getType (), scanStatusResult.getText (), {} };
    }
    else
    {
        LogValidation ("  File (unknown)");
        return { "warning", "(unknown file type)", {} };
    }
}

std::tuple<juce::String,juce::String> Assimil8orSDCardValidator::validateFolder (juce::File folder)
{
    const auto kMaxFolderNameLength { 31 };

    if (folder.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  Folder (ignored) : " + folder.getFileName ());
        return { "warning", "(ignored)" };
    }
    else if (folder.getFileName ().length () > kMaxFolderNameLength)
    {
        LogValidation ("  [ Warning : folder name too long ]");
        return { "error", "[name too long. " +
                            juce::String (folder.getFileName ().length ()) + "(length) vs " +
                            juce::String (kMaxFolderNameLength) + "(max)]" };
    }
    else
    {
        return { "info", "" };
    }
}

void Assimil8orSDCardValidator::processFolder (juce::ValueTree folderVT)
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
    ScanStatusResult scanStatusResult;
    jassert (folderVT.getType ().toString () == "Folder");
    uint64_t totalSizeOfPresets {};
    int numberOfPresets {};
    for (auto folderEntryVT : folderVT)
    {
        if (isThreadRunning () && threadShouldExit ())
            break;

        scanStatusResult.reset ();
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
            scanStatusResult.update ("info", "Folder: " + curEntry.getFileName ());
            const auto [newStatusType, newStatusText] = validateFolder (curEntry);
            scanStatusResult.update (newStatusType, newStatusText);
            addStatus (scanStatusResult.getType (), scanStatusResult.getText ());
            processFolder (folderEntryVT);
        }
        else
        {
            scanStatusResult.update ("info", "File: " + curEntry.getFileName ());
            const auto [newStatusType, newStatusText, presetUpdate] = validateFile (curEntry);
            if (presetUpdate.has_value ())
            {
                totalSizeOfPresets += presetUpdate.value ();
                ++numberOfPresets;
            }
            scanStatusResult.update (newStatusType, newStatusText);
            jassert (scanStatusResult.getType () != "");
            if (scanStatusResult.getType () != "")
                addStatus (scanStatusResult.getType (), scanStatusResult.getText ());
        }
    }
    const auto folderName { juce::File (folderVT.getProperty ("name")).getFileName () };
    addStatus ("info", "Total RAM Usage for `" + folderName + "': " + getMemorySizeString (totalSizeOfPresets));
    if (totalSizeOfPresets > maxMemory)
        addStatus ("error", "[RAM Usage exceeds memory capacity by " + getMemorySizeString (totalSizeOfPresets - maxMemory) + "]");
    if (numberOfPresets > maxPresets)
        addStatus ("error", "[Number of Presets (" + juce::String (numberOfPresets) + ") exceeds maximum allowed (" + juce::String (maxPresets) + ")]");
}

juce::ValueTree Assimil8orSDCardValidator::getContentsOfFolder (juce::File folder)
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

void Assimil8orSDCardValidator::validateRootFolder ()
{
    validatorProperties.getValidationStatusVT ().removeAllChildren (nullptr);
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootFolderName { validatorProperties.getRootFolder () };

    const auto rootEntry { juce::File (rootFolderName) };
    addStatus ("info", "Root Folder: " + rootEntry.getFileName ());
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

void Assimil8orSDCardValidator::run ()
{
    validateRootFolder ();
}

void Assimil8orSDCardValidator::sortContentsOfFolder (juce::ValueTree folderVT)
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
            if ( isPresetFile (curFile))
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

// NOTE: still very much under construction
void Assimil8orPreset::parse (juce::StringArray presetLines)
{
    auto scopeDepth { 0 };

    juce::ValueTree curPresetSection;
    juce::ValueTree curChannelSection;
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
        auto addSection = [&key] (const juce::Identifier& sectionId, juce::ValueTree parent)
        {
            auto section { juce::ValueTree { sectionId } };
            section.setProperty ("index", key.fromFirstOccurrenceOf (" ", false, false), nullptr);
            parent.addChild (section, -1, nullptr);
            return section;
        };
        switch (parseState)
        {
            case ParseState::ParsingGlobalSection:
            {
                if (keyIs (Section::PresetId))
                {
                    curPresetSection = addSection (Section::PresetId, assimil8orData);
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
                    curChannelSection = addSection (Section::ChannelId, curPresetSection);
                    setParseState (ParseState::ParsingChannelSection);
                }
                else if (keyIs (Parameter::Preset::NameId))
                {
                    curPresetSection.setProperty (Parameter::Preset::NameId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::Data2asCVId))
                {
                    curPresetSection.setProperty (Parameter::Preset::Data2asCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeACVId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeACVId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeAWidthId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeAWidthId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeBCVId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeBCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeBWidthId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeBWidthId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeCCVId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeCCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeCWidthId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeCWidthId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeDCVId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeDCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Preset::XfadeDWidthId))
                {
                    curPresetSection.setProperty (Parameter::Preset::XfadeDWidthId, value, nullptr);
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
                    curZoneSection = addSection (Section::ZoneId, curChannelSection);
                    setParseState (ParseState::ParsingZoneSection);
                }
                else if (keyIs (Parameter::Channel::AttackId))
                {
                    curChannelSection.setProperty (Parameter::Channel::AttackId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::AttackFromCurrentId))
                {
                    curChannelSection.setProperty (Parameter::Channel::AttackFromCurrentId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::AttackModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::AttackModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::AliasingId))
                {
                    curChannelSection.setProperty (Parameter::Channel::AliasingId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::AliasingModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::AliasingModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::AutoTriggerId))
                {
                    curChannelSection.setProperty (Parameter::Channel::AutoTriggerId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::BitsId))
                {
                    curChannelSection.setProperty (Parameter::Channel::BitsId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::BitsModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::BitsModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ChannelModeId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ChannelModeId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ExpAMId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ExpAMId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ExpFMId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ExpFMId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LevelId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LevelId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LinAMId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LinAMId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LinAMisExtEnvId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LinAMisExtEnvId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LinFMId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LinFMId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LoopLengthModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LoopLengthModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LoopModeId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LoopModeId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::LoopStartModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::LoopStartModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::MixLevelId))
                {
                    curChannelSection.setProperty (Parameter::Channel::MixLevelId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::MixModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::MixModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::MixModIsFaderId))
                {
                    curChannelSection.setProperty (Parameter::Channel::MixModIsFaderId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PanId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PanId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PanModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PanModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PhaseCVId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PhaseCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PitchId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PitchId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PitchCVId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PitchCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PlayModeId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PlayModeId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PMIndexId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PMIndexId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PMIndexModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PMIndexModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::PMSourceId))
                {
                    curChannelSection.setProperty (Parameter::Channel::PMSourceId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ReleaseId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ReleaseId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ReleaseModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ReleaseModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ReverseId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ReverseId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::SampleEndModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::SampleEndModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::SampleStartModId))
                {
                    curChannelSection.setProperty (Parameter::Channel::SampleStartModId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::SpliceSmoothingId))
                {
                    curChannelSection.setProperty (Parameter::Channel::SpliceSmoothingId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::XfadeGroupId))
                {
                    curChannelSection.setProperty (Parameter::Channel::XfadeGroupId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ZonesCVId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ZonesCVId, value, nullptr);
                }
                else if (keyIs (Parameter::Channel::ZonesRTId))
                {
                    curChannelSection.setProperty (Parameter::Channel::ZonesRTId, value, nullptr);
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
                    curZoneSection.setProperty (Parameter::Zone::LevelOffsetId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::LoopLengthId))
                {
                    curZoneSection.setProperty (Parameter::Zone::LoopLengthId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::LoopStartId))
                {
                    curZoneSection.setProperty (Parameter::Zone::LoopStartId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::MinVoltageId))
                {
                    curZoneSection.setProperty (Parameter::Zone::MinVoltageId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::PitchOffsetId))
                {
                    curZoneSection.setProperty (Parameter::Zone::PitchOffsetId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::SampleId))
                {
                    curZoneSection.setProperty (Parameter::Zone::SampleId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::SampleStartId))
                {
                    curZoneSection.setProperty (Parameter::Zone::SampleStartId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::SampleEndId))
                {
                    curZoneSection.setProperty (Parameter::Zone::SampleEndId, value, nullptr);
                }
                else if (keyIs (Parameter::Zone::SideId))
                {
                    curZoneSection.setProperty (Parameter::Zone::SideId, value, nullptr);
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
