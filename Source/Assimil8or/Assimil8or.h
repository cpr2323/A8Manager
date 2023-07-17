#pragma once

#include <JuceHeader.h>

//  SD Card (max folders unknown)
//      Folder (32 character len?) (max 199 presets) (no nested folders)
//          Preset (prst000.yml)
//          Preset
//          Audio (32 character len?) (arbitrary name) 
//          Audio

// File Contents
//      Preset 1 (1-8 channels)
//          Channel 1 (? zones)
//              Zone 1
class Assimil8orPreset
{
public:
    void parse (juce::StringArray presetLines);

    juce::ValueTree getPresetVT () { return assimil8orData; }

private:
    enum class ParseState
    {
        SeekingPresetSection,
        ParsingPresetSection,
        ParsingChannelSection,
        ParsingZoneSection,
    };
    ParseState parseState { ParseState::SeekingPresetSection };

    juce::ValueTree assimil8orData { "Assimil8or" };

    juce::Identifier PresetSectionId  { "Preset" };
    juce::Identifier PresetNamePropertyId { "Name" };
    juce::Identifier ChannelSectionId { "Channel" };
    juce::Identifier ZoneSectionId    { "Zone" };

    juce::String getParseStateString (ParseState parseState);
    void setParseState (ParseState newParseState);
};
