#include "Assimil8or.h"

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
        //juce::Logger::outputDebugString (juce::String (scopeDepth) + "-" + presetLine);
        auto key { presetLine.upToFirstOccurrenceOf (":", false, false).trim () };
        auto values { presetLine.fromFirstOccurrenceOf (":", false, false).trim () };
        auto valueList { juce::StringArray::fromTokens (values, " ", "\"") };

        auto keyIs = [&key] (juce::String desiredKey)
        {
            return key.upToFirstOccurrenceOf (" ", false, false) == desiredKey;
        };
        auto getSectionIndex = [&key] ()
        {
            return key.fromFirstOccurrenceOf (" ", false, false);
        };
        auto addValueTreeChild = [&getSectionIndex] (juce::Identifier sectionId, juce::ValueTree parent)
        {
            auto section = juce::ValueTree { sectionId };
            section.setProperty ("index", getSectionIndex (), nullptr);
            parent.addChild (section, -1, nullptr);
            return section;
        };
        auto addChildValue = [] (juce::ValueTree parent, juce::String childName, std::function<void (juce::ValueTree)> setProperties)
        {
            auto child { juce::ValueTree {childName} };
            setProperties (child);
            parent.addChild (child, -1, nullptr);
        };
        auto setValueAndCvInput = [&valueList] (juce::ValueTree child)
        {
            if (valueList.size () == 1)
            {
                child.setProperty ("amount", valueList [0], nullptr);
            }
            else if (valueList.size () == 2)
            {
                if (valueList [0].length () == 2)
                {
                    if (valueList [0].substring (1, 2).containsOnly ("012345678") &&
                        valueList [0].substring (2, 3).containsOnly ("ABC"))
                    {
                        child.setProperty ("cvInput", valueList [0], nullptr);
                        child.setProperty ("amount", valueList [1], nullptr);
                    }
                    else
                    {
                        jassertfalse;
                    }
                }
                else
                {
                    child.setProperty ("amount", valueList [0], nullptr);
                }
            }
            else
            {
                jassertfalse;
            }
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
                if (keyIs (ChannelSectionId.toString ()))
                {
                    curChannelSection = addValueTreeChild (ChannelSectionId, curPresetSection);
                    setParseState (ParseState::ParsingChannelSection);
                }
                else
                {
                    // Data2asCV : 1A
                    // Name: template001 (max len?)
                    // XfadeACV: 1A
                    // XfadeAWidth : 9.10
                    if (keyIs (PresetNamePropertyId.toString ()))
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
                        juce::Logger::outputDebugString ("unknown preset key: " + presetLine);
                        //jassertfalse;
                    }
                }
            }
            break;
            case ParseState::ParsingChannelSection:
            {
                if (keyIs (ZoneSectionId.toString ()))
                {
                    curZoneSection = addValueTreeChild (ZoneSectionId, curChannelSection);
                    setParseState (ParseState::ParsingZoneSection);
                }
                else
                {
                    // Aliasing: 100
                    // Attack:  0.0006
                    // AutoTrigger: 1
                    // Bits : 01.0
                    // BitsMod : Off 0.00
                    // ChannelMode : 2
                    // ExpAM : 0A 1.00
                    // ExpFM : 0A 1.00
                    // Level : -10.0
                    // LinAM: 0A -1.00
                    // LinFM: 0A -1.00
                    // LoopLengthMod: 0B -0.26
                    // LoopMode : 1
                    // LoopStartMod : 0C 0.00
                    // MixLevel : -90.0
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
                    // SpliceSmoothing : 1
                    // XfadeGroup : A
                    // ZonesCV : 0B
                    // ZonesRT : 1
                    if (keyIs ("Aliasing"))
                    {
                        addChildValue (curChannelSection, "Aliasing", [&valueList] (juce::ValueTree child)
                            {
                                child.setProperty ("amount", valueList [0], nullptr);
                            });
                            }
                    else if (keyIs ("Attack"))
                    {
                        addChildValue (curChannelSection, "Attack", [&valueList] (juce::ValueTree child)
                            {
                                child.setProperty ("amount", valueList [0], nullptr);
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
                        juce::Logger::outputDebugString ("unknown channel key: " + presetLine);
                        //jassertfalse;
                    }
                }
            }
            break;
            case ParseState::ParsingZoneSection:
            {
                // LoopLength: 256.0000
                // LoopStart : 111683
                // MinVoltage : +4.56
                // PitchOffset : +2.00
                // Sample : sampleA.wav
                // SampleStart : 79416
                // SampleEnd : 6058
                // Side : 1
                if (keyIs ("LoopLength"))
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
                    juce::Logger::outputDebugString ("unknown zone key: " + presetLine);
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
