#pragma once

#include <JuceHeader.h>
#include "Preset/PresetProperties.h"
#include <Stack>

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

private:
    
    std::stack<std::function<void()>> parseStack;
    juce::String sectionName;
    
    PresetProperties presetProperties;

    ActionMap globalActions;
    ActionMap presetActions;
    ActionMap channelActions;
    ActionMap zoneActions;
    ActionMap* curActions;

    juce::ValueTree curPresetSection;
    ChannelProperties channelProperties;
    juce::ValueTree curChannelSection;
    ZoneProperties zoneProperties;
    juce::ValueTree curZoneSection;
    
    juce::String key;
    juce::String value;
};
