#include "Assimil8or.h"

#define LOG_VALIDATION 0
#if LOG_VALIDATION
#define LogValidation(text) juce::Logger::outputDebugString (text);
#else
#define LogValidation(text) ;
#endif

#define LOG_PARSING 1
#if LOG_PARSING
#define LogParsing(text) juce::Logger::outputDebugString (text);
#else
#define LogParsing(text) ;
#endif

const auto maxMemory { 422 * 1024 * 1024 };

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

    void update (juce::String newType, juce::String newText)
    {
        updateType (newType);
        udpateText (newText);
    }

    void updateType (juce::String newType)
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

    void udpateText (juce::String newText)
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

Assimil8orSDCardImage::Assimil8orSDCardImage () : Thread ("Assimil8orSDCardImage")
{
    // initialize format manager for sample file reading
    audioFormatManager.registerBasicFormats ();
}

void Assimil8orSDCardImage::init (juce::ValueTree vt)
{
    validatorProperties.wrap (vt, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::yes);
    validatorProperties.onStartScanAsync = [this] () { validate (); };
}

void Assimil8orSDCardImage::validate ()
{
    validatorProperties.setScanStatus ("scanning", false);
    startThread ();
}

std::tuple<juce::String, juce::String> Assimil8orSDCardImage::validateFile (juce::File file)
{
    const auto kMaxFileNameLength { 47 };

    LogValidation ("File: " + file.getFileName ());
    if (file.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  File (ignored)");
        return { "warning", "(ignored)" };
    }
    else if (file.getFileExtension () == ".yml" &&
             file.getFileNameWithoutExtension ().length () == 7 &&
             file.getFileNameWithoutExtension ().startsWith ("prst") &&
             file.getFileNameWithoutExtension ().substring (4).containsOnly ("0123456789"))
    {
        ScanStatusResult scanStatusResult;
        scanStatusResult.update ("info", "Preset");

        juce::StringArray fileContents;
        file.readLines (fileContents);

        uint64_t sizeRequiredForSamples;
        Assimil8orPreset assimil8orPreset;
        assimil8orPreset.parse (fileContents);

        auto presetVT { assimil8orPreset.getPresetVT ().getChildWithName ("Preset") };
        jassert (presetVT.isValid ());
        ValueTreeHelpers::forEachChild (presetVT, [&file] (juce::ValueTree child)
        {
                if (child.getType ().toString () == "Channel")
                {
                    ValueTreeHelpers::forEachChild (child, [&file] (juce::ValueTree child)
                    {
                        if (child.getType ().toString () == "Zone")
                        {
                            const auto sampleFileName { child.getProperty ("Sample").toString () };
                            juce::File sampleFile (file.getParentDirectory().getChildFile(sampleFileName));
                            if (!sampleFile.exists ())
                            {
                                // report error
                            }
                            else
                            {
                                // open as audio file, calculate memory requirements
                            }
                        }
                        return true;
                    });
                }
            return true;
        });
        LogValidation ("  File (preset)");
        return { scanStatusResult.getType (), scanStatusResult.getText () };
    }
    else if (file.getFileExtension () == ".wav")
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
            const auto memoryUsageString = [memoryUsage] ()
            {
                const double oneK { 1024.0f };
                const double oneMB { 1024.0f * 1024.0f };
                const double oneGB { 1024.0f * 1024.0f * 1024.0f };
                if (memoryUsage >= oneGB)
                    return juce::String (memoryUsage / oneGB, 2).trimCharactersAtEnd ("0.") + "GB";
                else if (memoryUsage >= oneMB)
                    return juce::String (memoryUsage / oneMB, 2).trimCharactersAtEnd ("0.") + "MB";
                else if (memoryUsage >= oneK)
                    return juce::String (memoryUsage / oneK, 2).trimCharactersAtEnd ("0.") + "k";
                else
                    return juce::String (memoryUsage) + "bytes";
            }();
            scanStatusResult.udpateText (juce::String (" {") +
                                         juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer") + ", " +
                                         juce::String (reader->bitsPerSample) + "bits/" + sampleRateString + "k, " +
                                         juce::String (reader->numChannels == 1 ? "mono" : (reader->numChannels == 2 ? "stereo" : juce::String(reader->numChannels) + " channels")) + "}, " +
                                         juce::String (reader->lengthInSamples / reader->sampleRate, 2) + " seconds, " +
                                         "RAM: " + juce::String(memoryUsage) + " bytes (" + memoryUsageString + ")");
            auto reportErrorIfTrue = [&scanStatusResult] (bool conditionalResult, juce::String newText)
            {
                if (conditionalResult)
                    scanStatusResult.update ("error", newText);
            };
            reportErrorIfTrue (reader->usesFloatingPointData == true, "[sample format must be integer]");
            reportErrorIfTrue (reader->bitsPerSample < 8 || reader->bitsPerSample > 32, "[bit depth must be between 8 and 32]");
            reportErrorIfTrue (reader->numChannels < 1 || reader->numChannels > 2, "[only mono and stereo supported]");
            reportErrorIfTrue (reader->sampleRate > 192000, "[sample rate must not exceed 129k]");
        }
        return { scanStatusResult.getType (), scanStatusResult.getText () };
    }
    else
    {
        LogValidation ("  File (unknown)");
        return { "warning", "(unknown file type)" };
    }
}

std::tuple<juce::String,juce::String> Assimil8orSDCardImage::validateFolder (juce::File folder)
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

void Assimil8orSDCardImage::validateFolderContents (juce::File folder, std::vector<juce::File>& foldersToScan, bool isRoot)
{
    // iterate over files system
    // for directories
    //      report if name over 31 characters
    //      (TODO) report if folder is 2 or more deep in hierarchy
    // for preset files (to be done when the preset file is fully defined)
    //      (TODO) report if channels * samples * 32 > Assimil8or memory
    //      (TODO) validate preset file data (really only important if it has been edited)
    // for audio files
    //      report if name over 47 characters
    //      report if it does not match supported formats
    // report any other files as unused by assimil8or

    auto addStatus = [this] (juce::String statusType, juce::String statusText)
    {
        auto newStatusChild { juce::ValueTree {"Status"}};
        newStatusChild.setProperty ("type", statusType, nullptr);
        newStatusChild.setProperty ("text", statusText, nullptr);
        validatorProperties.getValidationStatusVT ().addChild (newStatusChild, -1, nullptr);
    };
    ScanStatusResult scanStatusResult;
    LogValidation ("Folder: " + folder.getFileName ());
    if (isRoot)
    {
        addStatus ("info", "Root Folder: " + folder.getFileName ());
    }
    else
    {
        scanStatusResult.update ("info", "Folder: " + folder.getFileName ());
        auto [newStatusType, newStatusText] = validateFolder (folder);
        scanStatusResult.update (newStatusType, newStatusText);
        addStatus (scanStatusResult.getType (), scanStatusResult.getText ());
    }

    scanStatusResult.reset ();
    for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
    {
        if (threadShouldExit ())
            return;

        const auto& curFile { entry.getFile () };
        if (curFile.isDirectory ())
        {
            // queue folders up to be handled in scan thread loop, ie. our caller
            foldersToScan.emplace_back (curFile);
        }
        else
        {
            scanStatusResult.update ("info", "File: " + curFile.getFileName ());
            auto [newStatusType, newStatusText] = validateFile (curFile);
            scanStatusResult.update (newStatusType, newStatusText);

            jassert (scanStatusResult.getType () != "");
            if (scanStatusResult.getType () != "")
                addStatus (scanStatusResult.getType (), scanStatusResult.getText ());
            scanStatusResult.reset ();
        }
    }
}

void Assimil8orSDCardImage::run ()
{
    bool isRoot { true };
    validatorProperties.getValidationStatusVT ().removeAllChildren (nullptr);
    std::vector<juce::File> foldersToScan;
    foldersToScan.emplace_back (validatorProperties.getRootFolder ());
    while (foldersToScan.size () > 0)
    {
        auto curFolderToScan { foldersToScan.back () };
        foldersToScan.pop_back ();
        validateFolderContents (curFolderToScan, foldersToScan, isRoot);
        isRoot = false;
    }

    juce::MessageManager::callAsync ([this] ()
    {
        validatorProperties.setScanStatus ("idle", false);
    });
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
        auto indent { presetLine.initialSectionContainingOnly (" ") };
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
                    case ParseState::SeekingPresetSection: { jassertfalse; } break;
                    case ParseState::ParsingPresetSection: { curPresetSection = {}; setParseState (ParseState::SeekingPresetSection); } break;
                    case ParseState::ParsingChannelSection: { curChannelSection = {}; setParseState (ParseState::ParsingPresetSection); } break;
                    case ParseState::ParsingZoneSection: { curZoneSection = {}; setParseState (ParseState::ParsingChannelSection); } break;
                    default: { jassertfalse; } break;
                }
            }
        }

        presetLine = presetLine.trim ();
        LogParsing (juce::String (scopeDepth) + "-" + presetLine);
        auto key { presetLine.upToFirstOccurrenceOf (":", false, false).trim () };
        auto values { presetLine.fromFirstOccurrenceOf (":", false, false).trim () };
        auto valueList { juce::StringArray::fromTokens (values, " ", "\"") };

        auto keyIs = [&key] (juce::String desiredKey)
        {
            return key.upToFirstOccurrenceOf (" ", false, false) == desiredKey;
        };
        auto addValueTreeChild = [&key] (juce::Identifier sectionId, juce::ValueTree parent)
        {
            auto section { juce::ValueTree { sectionId } };
            section.setProperty ("index", key.fromFirstOccurrenceOf (" ", false, false), nullptr);
            parent.addChild (section, -1, nullptr);
            return section;
        };
        switch (parseState)
        {
            case ParseState::SeekingPresetSection:
            {
                if (keyIs (PresetSectionId.toString ()))
                {
                    curPresetSection = addValueTreeChild (PresetSectionId, assimil8orData);
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
                if (keyIs (ChannelSectionId.toString ()))
                {
                    curChannelSection = addValueTreeChild (ChannelSectionId, curPresetSection);
                    setParseState (ParseState::ParsingChannelSection);
                }
                else if (keyIs (PresetNamePropertyId.toString ()))
                {
                    curPresetSection.setProperty (PresetNamePropertyId, valueList [0], nullptr);
                }
                else if (keyIs ("XfadeACV"))
                {
                    curPresetSection.setProperty ("XfadeACV", valueList [0], nullptr);
                }
                else if (keyIs ("XfadeAWidth"))
                {
                    curPresetSection.setProperty ("XfadeAWidth", valueList [0], nullptr);
                }
                else if (keyIs ("Data2asCV"))
                {
                    curPresetSection.setProperty ("Data2asCV", valueList [0], nullptr);
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
                if (keyIs (ZoneSectionId.toString ()))
                {
                    curZoneSection = addValueTreeChild (ZoneSectionId, curChannelSection);
                    setParseState (ParseState::ParsingZoneSection);
                }
                else if (keyIs ("Attack"))
                {
                    curChannelSection.setProperty ("Attack", valueList [0], nullptr);
                }
                else if (keyIs ("AttackFromCurrent"))
                {
                    curChannelSection.setProperty ("AttackFromCurrent", valueList [0], nullptr);
                }
                else if (keyIs ("AttackMod"))
                {
                    curChannelSection.setProperty ("AttackMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("Aliasing"))
                {
                    curChannelSection.setProperty ("Aliasing", valueList [0], nullptr);
                }
                else if (keyIs ("AliasingMod"))
                {
                    curChannelSection.setProperty ("AliasingMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("AutoTrigger"))
                {
                    curChannelSection.setProperty ("AutoTrigger", valueList [0], nullptr);
                }
                else if (keyIs ("Bits"))
                {
                    curChannelSection.setProperty ("Bits", valueList [0], nullptr);
                }
                else if (keyIs ("BitsMod"))
                {
                    curChannelSection.setProperty ("BitsMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("ChannelMode"))
                {
                    curChannelSection.setProperty ("ChannelMode", valueList [0], nullptr);
                }
                else if (keyIs ("ExpAM"))
                {
                    curChannelSection.setProperty ("ExpAM", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("ExpFM"))
                {
                    curChannelSection.setProperty ("ExpFM", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("Level"))
                {
                    curChannelSection.setProperty ("Level", valueList [0], nullptr);
                }
                else if (keyIs ("LinAM"))
                {
                    curChannelSection.setProperty ("LinAM", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("LinAMisExtEnv"))
                {
                    curChannelSection.setProperty ("LinAMisExtEnv", valueList [0], nullptr);
                }
                else if (keyIs ("LinFM"))
                {
                    curChannelSection.setProperty ("LinFM", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("LoopLengthMod"))
                {
                    curChannelSection.setProperty ("LoopLengthMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("LoopMode"))
                {
                    curChannelSection.setProperty ("LoopMode", valueList [0], nullptr);
                }
                else if (keyIs ("LoopStartMod"))
                {
                    curChannelSection.setProperty ("LoopStartMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("MixLevel"))
                {
                    curChannelSection.setProperty ("MixLevel", valueList [0], nullptr);
                }
                else if (keyIs ("MixMod"))
                {
                    curChannelSection.setProperty ("MixMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("MixModIsFader"))
                {
                    curChannelSection.setProperty ("MixModIsFader", valueList [0], nullptr);
                }
                else if (keyIs ("Pan"))
                {
                    curChannelSection.setProperty ("Pan", valueList [0], nullptr);
                }
                else if (keyIs ("PanMod"))
                {
                    curChannelSection.setProperty ("PanMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("PhaseCV"))
                {
                    curChannelSection.setProperty ("PhaseCV", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("Pitch"))
                {
                    curChannelSection.setProperty ("Pitch", valueList [0], nullptr);
                }
                else if (keyIs ("PitchCV"))
                {
                    curChannelSection.setProperty ("PitchCV", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("PlayMode"))
                {
                    curChannelSection.setProperty ("PlayMode", valueList [0], nullptr);
                }
                else if (keyIs ("PMIndex"))
                {
                    curChannelSection.setProperty ("PMIndex", valueList [0], nullptr);
                }
                else if (keyIs ("PMIndexMod"))
                {
                    curChannelSection.setProperty ("PMIndexMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("PMSource"))
                {
                    curChannelSection.setProperty ("PMSource", valueList [0], nullptr);
                }
                else if (keyIs ("Release"))
                {
                    curChannelSection.setProperty ("Release", valueList [0], nullptr);
                }
                else if (keyIs ("ReleaseMod"))
                {
                    curChannelSection.setProperty ("ReleaseMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("Reverse"))
                {
                    curChannelSection.setProperty ("Reverse", valueList [0], nullptr);
                }
                else if (keyIs ("SampleEndMod"))
                {
                    curChannelSection.setProperty ("SampleEndMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("SampleStartMod"))
                {
                    curChannelSection.setProperty ("SampleStartMod", valueList [0] + " " + valueList [1], nullptr);
                }
                else if (keyIs ("SpliceSmoothing"))
                {
                    curChannelSection.setProperty ("SpliceSmoothing", valueList [0], nullptr);
                }
                else if (keyIs ("XfadeGroup"))
                {
                    curChannelSection.setProperty ("XfadeGroup", valueList [0], nullptr);
                }
                else if (keyIs ("ZonesCV"))
                {
                    curChannelSection.setProperty ("ZonesCV", valueList [0], nullptr);
                }
                else if (keyIs ("ZonesRT"))
                {
                    curChannelSection.setProperty ("ZonesRT", valueList [0], nullptr);
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
                if (keyIs ("LevelOffset"))
                {
                    curZoneSection.setProperty ("LevelOffset", valueList [0], nullptr);
                }
                else if (keyIs ("LoopLength"))
                {
                    curZoneSection.setProperty ("LoopLength", valueList [0], nullptr);
                }
                else if (keyIs ("LoopStart"))
                {
                    curZoneSection.setProperty ("LoopStart", valueList [0], nullptr);
                }
                else if (keyIs ("MinVoltage"))
                {
                    curZoneSection.setProperty ("MinVoltage", valueList [0], nullptr);
                }
                else if (keyIs ("PitchOffset"))
                {
                    curZoneSection.setProperty ("PitchOffset", valueList [0], nullptr);
                }
                else if (keyIs ("Sample"))
                {
                    curZoneSection.setProperty ("Sample", valueList [0], nullptr);
                }
                else if (keyIs ("SampleStart"))
                {
                    curZoneSection.setProperty ("SampleStart", valueList [0], nullptr);
                }
                else if (keyIs ("SampleEnd"))
                {
                    curZoneSection.setProperty ("SampleEnd", valueList [0], nullptr);
                }
                else if (keyIs ("Side"))
                {
                    curZoneSection.setProperty ("Side", valueList [0], nullptr);
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
        case ParseState::SeekingPresetSection: { return "SeekingPresetSection"; } break;
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
