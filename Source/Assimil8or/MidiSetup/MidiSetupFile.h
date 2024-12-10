#pragma once

#include <JuceHeader.h>
#include "MidiSetupProperties.h"

using Action = std::function<void ()>;
using ActionMap = std::map<juce::String, Action>;


namespace MidiSetup
{
    static inline const juce::String ModeId { "mode" };
    static inline const juce::String AssignId { "assign" };
    static inline const juce::String BasicChannelId { "basicchannel" };
    static inline const juce::String RcvProgramChangeId { "rcvprogamchange" };
    static inline const juce::String XmtProgramChangeId { "xmtprogamchange" };
    static inline const juce::String ColACCId { "colAcc" };
    static inline const juce::String ColBCCId { "colBcc" };
    static inline const juce::String ColCCCId { "colCcc" };
    static inline const juce::String PitchWheelSemiId { "pitchwheelsemi" };
    static inline const juce::String VelocityDepthId { "velocitydepth" };
    static inline const juce::String NotificationsId { "notifications" };
};

class MidiSetupFile
{
public:
    MidiSetupFile ();
    void write (juce::File presetFile, juce::ValueTree presetProperties);
    juce::ValueTree parse (juce::StringArray presetLines);

    juce::ValueTree getMidiSetupPropertiesVT () { return midiSetupProperties.getValueTree (); }

private:
    MidiSetupProperties midiSetupProperties;
    ActionMap globalActions;
    juce::String key;
    juce::String value;

    void initParser ();
};