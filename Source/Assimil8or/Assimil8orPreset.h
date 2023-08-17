#pragma once

#include <JuceHeader.h>
#include "Preset/PresetProperties.h"
#include <stack>

// File Contents
//      Preset 1 (1-8 channels)
//          Channel 1 (? zones)
//              Zone 1

// everything has a default
// some things have min/max
// type info:
//      bool
//      int
//      fp, decimal places?
//      fp, +/-
//      string

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

    juce::ValueTree getParseErrorsVT () { return parseErrorList; }

private:
    
    juce::String getSectionName ();
    
    std::stack<Action> undoActionsStack;
    
    PresetProperties presetProperties;
    juce::ValueTree parseErrorList { "ParseErrorList" };

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

    void checkCvInputAndAmountFormat (juce::String theKey, juce::String theValue);
};
