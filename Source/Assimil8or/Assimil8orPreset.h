#pragma once

#include <JuceHeader.h>
#include "Preset/PresetProperties.h"
#include <stack>

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
    PresetProperties presetProperties;
    PresetProperties minPresetProperties;
    PresetProperties maxPresetProperties;

    juce::ValueTree parseErrorList { "ParseErrorList" };
    std::stack<Action> undoActionsStack;
    ActionMap globalActions;
    ActionMap presetActions;
    ActionMap channelActions;
    ActionMap zoneActions;
    ActionMap * curActions { nullptr };
    juce::ValueTree curPresetSection;
    ChannelProperties channelProperties;
    juce::ValueTree curChannelSection;
    ZoneProperties zoneProperties;
    juce::ValueTree curZoneSection;
    juce::String key;
    juce::String value;

    void checkCvInputAndAmountFormat (juce::String theKey, juce::String theValue);
    juce::String getSectionName ();
    void initParser ();
};
