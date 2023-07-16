#pragma once

#include <JuceHeader.h>

class Assimil8orPresets
{
public:
    void parse (juce::StringArray presetLines);

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
