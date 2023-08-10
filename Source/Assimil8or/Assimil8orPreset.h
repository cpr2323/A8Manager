#pragma once

#include <JuceHeader.h>
#include "Preset/PresetProperties.h"
#include <stack>

// File Contents
//      Preset 1 (1-8 channels)
//          Channel 1 (? zones)
//              Zone 1


using Action = std::function<void ()>;
using ActionMap = std::map<juce::String, Action>;

class Assimil8orPreset
{
public:
    Assimil8orPreset ();
    void write (juce::File presetFile);
    void write (juce::File presetFile, juce::ValueTree presetProperties);
    void parse (juce::StringArray presetLines);

    juce::ValueTree getPresetVT () { return presetProperties.getValueTree (); }

private:
    
    juce::String getSectionName ();
    
    std::stack<Action> undoActionsStack;
    
    PresetProperties presetProperties;

    ActionMap globalActions;
    ActionMap presetActions;
    ActionMap channelActions;
    ActionMap zoneActions;
    ActionMap * curActions {nullptr};

    juce::ValueTree curPresetSection;
    ChannelProperties channelProperties;
    juce::ValueTree curChannelSection;
    ZoneProperties zoneProperties;
    juce::ValueTree curZoneSection;
    
    juce::String key;
    juce::String value;
};
