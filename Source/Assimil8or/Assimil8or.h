#pragma once

#include <JuceHeader.h>

// validate
//  SD Card (max folders unknown)
//      Folder (31 character len) (max 199 presets) (no nested folders)
//          Preset (prst000.yml)
//          Preset
//          Audio (47 character len) (arbitrary name) 
//          Audio
class Assimil8orSDCardImage
{
public:
    void setRootFolder (juce::File newRootFolder);
    void validate ();
private:
    juce::File rootFolder;

    void refreshFiles ();
};

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
