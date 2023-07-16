#include "Assimil8or.h"

void Assimil8orPresets::parse (juce::StringArray presetLines)
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

        // juce::Logger::outputDebugString (juce::String (scopeDepth) + "-" + lineFromFile);

        presetLine = presetLine.trim ();
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
                    // Name: template001
                    // XfadeACV: 1A
                    // XfadeAWidth : 9.10
                    if (keyIs ("Name"))
                    {
                        auto nameChild { juce::ValueTree {"Name"} };
                        nameChild.setProperty ("name", valueList [0], nullptr);
                        curPresetSection.addChild (nameChild, -1, nullptr);
                    }
                    else if (keyIs ("XfadeACV"))
                    {
                        addChildValue (curChannelSection, "XfadeACV", [&valueList] (juce::ValueTree child)
                            {
                                child.setProperty ("cvInput", valueList [0], nullptr);
                            });
                    }
                    else if (keyIs ("XfadeAWidth"))
                    {
                        addChildValue (curChannelSection, "XfadeAWidth", [&valueList] (juce::ValueTree child)
                            {
                                child.setProperty ("amount", valueList [0], nullptr);
                            });
                    }
                    else
                        jassertfalse;
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
                    // BitsMod : Off 0.00
                    // ChannelMode : 2
                    // Level : -10.0
                    // LoopMode : 1
                    // LoopStartMod : 0C 0.00
                    // MixLevel : -90.0
                    // Pan : -0.30
                    // PanMod : Off 0.00
                    // Pitch : -16.79
                    // PitchCV : 0A 0.50
                    // PMIndexMod : 0C 0.18
                    // PMSource : 8
                    // Release : 99.0000
                    // ReleaseMod : 0C 1.00
                    // SpliceSmoothing : 1
                    // ZonesCV : 0B
                    // ZonesRT : 1
                    if (keyIs ("BitsMod"))
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
                    else if (keyIs ("Level"))
                    {
                        addChildValue (curChannelSection, "Level", [&valueList] (juce::ValueTree child)
                            {
                                child.setProperty ("amount", valueList [0], nullptr);
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
                    else if (keyIs ("SpliceSmoothing"))
                    {
                        addChildValue (curChannelSection, "SpliceSmoothing", [&valueList] (juce::ValueTree child)
                            {
                                child.setProperty ("mode", valueList [0], nullptr);
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
                        jassertfalse;
                    }
                }
            }
            break;
            case ParseState::ParsingZoneSection:
            {
                // LoopLength: 256.0000
                // MinVoltage : +4.56
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
                else if (keyIs ("MinVoltage"))
                {
                    addChildValue (curZoneSection, "MinVoltage", [&valueList] (juce::ValueTree sampleEndChild)
                        {
                            sampleEndChild.setProperty ("voltage", valueList [0], nullptr);
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
                else
                {
                    jassertfalse;
                }
            }
            break;
            default:
                jassertfalse;
                break;
        }
    }
}

juce::String Assimil8orPresets::getParseStateString (ParseState theParseState)
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

void Assimil8orPresets::setParseState (ParseState newParseState)
{
    parseState = newParseState;
    // juce::Logger::outputDebugString ("new state: " + getParseStateString (parseState));
}
