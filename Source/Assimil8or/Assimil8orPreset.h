#pragma once

#include <JuceHeader.h>
#include "Preset/PresetProperties.h"

// File Contents
//      Preset 1 (1-8 channels)
//          Channel 1 (? zones)
//              Zone 1

using ActionMap = std::map<juce::String, std::function<void ()>>;

class Assimil8orPreset
{
public:
    Assimil8orPreset ();
    void write (juce::File presetFile);
    void write (juce::File presetFile, juce::ValueTree presetProperties);
    void parse (juce::StringArray presetLines);

    juce::ValueTree getPresetVT () { return presetProperties.getValueTree (); }

    juce::ValueTree getParseErrorsVT () { return parseErrorList; }

private:
    enum class ParseState
    {
        ParsingGlobalSection,
        ParsingPresetSection,
        ParsingChannelSection,
        ParsingZoneSection,
    };
    ParseState parseState { ParseState::ParsingGlobalSection };
    PresetProperties presetProperties;
    juce::ValueTree parseErrorList { "ParseErrorList" };

    ActionMap globalActions;
    ActionMap presetActions;
    ActionMap channelActions;
    ActionMap zoneActions;
    ActionMap* curActions;
    juce::String sectionName;

    juce::ValueTree curPresetSection;
    ChannelProperties channelProperties;
    juce::ValueTree curChannelSection;
    ZoneProperties zoneProperties;
    juce::ValueTree curZoneSection;
    juce::String key;
    juce::String value;

    juce::String getParseStateString (ParseState parseState);
    void setParseState (ParseState newParseState, ActionMap* newActions, juce::String newSectionName);
};
