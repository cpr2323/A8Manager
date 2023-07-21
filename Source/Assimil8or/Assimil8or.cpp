#include "Assimil8or.h"

#define LOG_VALIDATION 0
#if LOG_VALIDATION
#define LogValidation(text) juce::Logger::outputDebugString(text);
#else
#define LogValidation(text) ;
#endif

#define LOG_PARSING 1
#if LOG_PARSING 
#define LogParsing(text) juce::Logger::outputDebugString(text);
#else
#define LogParsing(text) ;
#endif

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
    a8SDCardValidatorProperties.wrap (vt, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::yes);
    a8SDCardValidatorProperties.onStartScanAsync = [this] () { validate (); };
}

void Assimil8orSDCardImage::validate()
{
    a8SDCardValidatorProperties.setScanStatus ("scanning", false);
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
             file.getFileNameWithoutExtension ().substring(4).containsOnly ("0123456789"))
    {
        LogValidation ("  File (preset)");
        return { "info", "Preset File" };
    }
    else if (file.getFileExtension () == ".wav")
    {
        ScanStatusResult scanStatusResult;
        scanStatusResult.updateType ("info");
        if (file.getFileName ().length () > kMaxFileNameLength)
        {
            LogValidation ("  [ Warning : file name too long ]");
            scanStatusResult.update("error",
                                    "[name too long. " + juce::String(file.getFileName ().length()) + "(length) vs " +
                                       juce::String (kMaxFileNameLength) +"(max)]");
        }

        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
        if (reader == nullptr)
        {
            LogValidation ("    [ Warning : unknown audio format ]");
            scanStatusResult.update ("error", "[unknown audio format. size = " + juce::String (file.getSize()) + "]");
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

            scanStatusResult.udpateText(juce::String ("Audio format: ") +
                                        juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer") + ", " +
                                        juce::String (reader->bitsPerSample) + "bits/" + juce::String (reader->sampleRate / 1000.0f, 2) + "k, " +
                                        juce::String (reader->numChannels == 1 ? "mono" : "stereo") + ", " +
                                        juce::String (reader->lengthInSamples / reader->sampleRate) + " seconds");
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
        a8SDCardValidatorProperties.getValidationStatusVT ().addChild (newStatusChild, -1, nullptr);
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
        addStatus (scanStatusResult.getType (), scanStatusResult.getText());
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
        }
        if (scanStatusResult.getType () != "")
            addStatus (scanStatusResult.getType (), scanStatusResult.getText ());
        scanStatusResult.reset ();
    }
}

void Assimil8orSDCardImage::run ()
{
    bool isRoot = true;
    a8SDCardValidatorProperties.getValidationStatusVT ().removeAllChildren (nullptr);
    std::vector<juce::File> foldersToScan;
    foldersToScan.emplace_back (a8SDCardValidatorProperties.getRootFolder ());
    while (foldersToScan.size () > 0)
    {
        auto curFolderToScan { foldersToScan.back () };
        foldersToScan.pop_back ();
        validateFolderContents (curFolderToScan, foldersToScan, isRoot);
        isRoot = false;
    }

    juce::MessageManager::callAsync ([this] ()
    {
        a8SDCardValidatorProperties.setScanStatus ("idle", false);
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
            auto section = juce::ValueTree { sectionId };
            section.setProperty ("index", key.fromFirstOccurrenceOf (" ", false, false), nullptr);
            parent.addChild (section, -1, nullptr);
            return section;
        };
        auto addChildValue = [] (juce::ValueTree parent, juce::String childName, std::function<void (juce::ValueTree)> setProperties)
        {
            auto child { juce::ValueTree {childName} };
            setProperties (child);
            parent.addChild (child, -1, nullptr);
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
                    auto nameChild { juce::ValueTree {PresetNamePropertyId} };
                    nameChild.setProperty ("name", valueList [0], nullptr);
                    curPresetSection.addChild (nameChild, -1, nullptr);
                }
                else if (keyIs ("XfadeACV"))
                {
                    addChildValue (curPresetSection, "XfadeACV", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("XfadeAWidth"))
                {
                    addChildValue (curPresetSection, "XfadeAWidth", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("Data2asCV"))
                {
                    addChildValue (curPresetSection, "Data2asCV", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                        });
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
                    addChildValue (curChannelSection, "Attack", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("AttackFromCurrent"))
                {
                    addChildValue (curChannelSection, "AttackFromCurrent", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("AttackMod"))
                {
                    addChildValue (curChannelSection, "AttackMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("Aliasing"))
                {
                    addChildValue (curChannelSection, "Aliasing", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("AliasingMod"))
                {
                    addChildValue (curChannelSection, "AliasingMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("AutoTrigger"))
                {
                    addChildValue (curChannelSection, "AutoTrigger", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("mode", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("Bits"))
                {
                    addChildValue (curChannelSection, "Bits", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("BitsMod"))
                {
                    addChildValue (curChannelSection, "BitsMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("enabled", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("ChannelMode"))
                {
                    addChildValue (curChannelSection, "ChannelMode", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("mode", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("ExpAM"))
                {
                    addChildValue (curChannelSection, "ExpAM", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("ExpFM"))
                {
                    addChildValue (curChannelSection, "ExpFM", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("Level"))
                {
                    addChildValue (curChannelSection, "Level", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("LinAM"))
                {
                    addChildValue (curChannelSection, "LinAM", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("LinAMisExtEnv"))
                {
                    addChildValue (curChannelSection, "LinAMisExtEnv", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("LinFM"))
                {
                    addChildValue (curChannelSection, "LinFM", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("LoopLengthMod"))
                {
                    addChildValue (curChannelSection, "LoopLengthMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("LoopMode"))
                {
                    addChildValue (curChannelSection, "LoopMode", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("mode", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("LoopStartMod"))
                {
                    addChildValue (curChannelSection, "LoopStartMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("MixLevel"))
                {
                    addChildValue (curChannelSection, "MixLevel", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("MixMod"))
                {
                    addChildValue (curChannelSection, "MixMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("MixModIsFader"))
                {
                    addChildValue (curChannelSection, "MixModIsFader", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("Pan"))
                {
                    addChildValue (curChannelSection, "Pan", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("PanMod"))
                {
                    addChildValue (curChannelSection, "PanMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("PhaseCV"))
                {
                    addChildValue (curChannelSection, "PhaseCV", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("Pitch"))
                {
                    addChildValue (curChannelSection, "Pitch", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("PitchCV"))
                {
                    addChildValue (curChannelSection, "PitchCV", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("PlayMode"))
                {
                    addChildValue (curChannelSection, "PlayMode", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("mode", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("PMIndex"))
                {
                    addChildValue (curChannelSection, "PMIndex", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("PMIndexMod"))
                {
                    addChildValue (curChannelSection, "PMIndexMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("PMSource"))
                {
                    addChildValue (curChannelSection, "PMSource", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("Release"))
                {
                    addChildValue (curChannelSection, "Release", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("ReleaseMod"))
                {
                    addChildValue (curChannelSection, "ReleaseMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("Reverse"))
                {
                    addChildValue (curChannelSection, "Reverse", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("mode", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("SampleEndMod"))
                {
                    addChildValue (curChannelSection, "SampleEndMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                        }
                else if (keyIs ("SampleStartMod"))
                {
                    addChildValue (curChannelSection, "SampleStartMod", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                            child.setProperty ("amount", valueList [1], nullptr);
                        });
                }
                else if (keyIs ("SpliceSmoothing"))
                {
                    addChildValue (curChannelSection, "SpliceSmoothing", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("mode", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("XfadeGroup"))
                {
                    addChildValue (curChannelSection, "XfadeGroup", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("group", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("ZonesCV"))
                {
                    addChildValue (curChannelSection, "ZonesCV", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("cvInput", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("ZonesRT"))
                {
                    addChildValue (curChannelSection, "ZonesRT", [&valueList] (juce::ValueTree child)
                        {
                            child.setProperty ("value", valueList [0], nullptr);
                        });
                }
                else
                {
                    LogParsing("unknown channel key: " + presetLine);
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
                    addChildValue (curZoneSection, "LevelOffset", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("amount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("LoopLength"))
                {
                    addChildValue (curZoneSection, "LoopLength", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("sampleCount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("LoopStart"))
                {
                    addChildValue (curZoneSection, "LoopStart", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("sampleCount", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("MinVoltage"))
                {
                    addChildValue (curZoneSection, "MinVoltage", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("voltage", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("PitchOffset"))
                {
                    addChildValue (curZoneSection, "PitchOffset", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("semitones", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("Sample"))
                {
                    addChildValue (curZoneSection, "Sample", [&valueList] (juce::ValueTree sampleChild)
                        {
                            sampleChild.setProperty ("fileName", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("SampleStart"))
                {
                    addChildValue (curZoneSection, "SampleStart", [&valueList] (juce::ValueTree sampleStartChild)
                        {
                            sampleStartChild.setProperty ("sampleOffset", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("SampleEnd"))
                {
                    addChildValue (curZoneSection, "SampleEnd", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("sampleOffset", valueList [0], nullptr);
                        });
                }
                else if (keyIs ("Side"))
                {
                    addChildValue (curZoneSection, "Side", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("side", valueList [0], nullptr);
                        });
                }
                else
                {
                    LogParsing("unknown zone key: " + presetLine);
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
