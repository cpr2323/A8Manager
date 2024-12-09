#pragma once

#include <JuceHeader.h>
#include <stack>
#include "../../Utility/ValueTreeWrapper.h"

using Action = std::function<void ()>;
using ActionMap = std::map<juce::String, Action>;

namespace MidiSetup
{
    static inline const juce::String ModeId             { "mode" };
    static inline const juce::String AssignId           { "assign" };
    static inline const juce::String BasicChannelId     { "basicchannel" };
    static inline const juce::String RcvProgramChangeId { "rcvprogamchange" };
    static inline const juce::String XmtProgramChangeId { "xmtprogamchange" };
    static inline const juce::String ColACCId           { "colAcc" };
    static inline const juce::String ColBCCId           { "colBcc" };
    static inline const juce::String ColCCCId           { "colCcc" };
    static inline const juce::String PitchWheelSemiId   { "pitchwheelsemi" };
    static inline const juce::String VelocityDepthId    { "velocitydepth" };
    static inline const juce::String NotificationsId    { "notifications" };
};

// Mode : Omni, Uni, Multi - 0,1,2
// Assignment : One set of values for Omni/Uni and another for Multi. The unit seems to remember the setting for each of these
//              Dynamic, Dynamic II, Chroma 8, Chroma VZ, Chroma KZ, SP-1200, SP-1200 X - (0-6)
//              Multi, Multi VZ, Multi KZ, No Trigger (10-13)
// Basic Channel : (1 - 16)
// Rcv Program Change : Off (0), Exists (1), Any (2)
// Xmt Program Change : Off (0), On (1)
// Col A CC : <= CV in Jacks, -1
//            <= CC# (0-95), 0 - 95
//            <= CC# 16-23, 96
//            <= CC# 24-31, 97
//            <= CC# 104-111, 98
//            <= CC# 112-119, - 99
//            <= Pressure, 100
//            <= Velocity, 101
//            <= Note #, 102
//            0..5V => Tx 16+, 103
//            0..5V => Tx 24+, 104
//            0..5V => Tx 104+, 105
//            0..5V => Tx 112+, 106
//            -5..5V => Tx 16+, 107
//            -5..5V => Tx 24+, 108
//            -5..5V => Tx 104+, 109
//            -5..5V => Tx 112+, 110
//            0..5V => multi 16, 111
//            0..5V => multi 24, 112
//            0..5V => multi 104+, 113
//            0..5V => multi 112+, 114
//            -5..5V => multi 16, 115
//            -5..5V => multi 24, 116
//            -5..5V => multi 104+, 117
//            -5..5V => multi 112+, 118
// Col B CC : [same as Col A CC]
// Col C CC : [same as Col A CC]
// Pitchwheel : +/- (0-12)
// Velocity Depth : (0-127)
// Notification: On/Off
class MidiSetupProperties : public ValueTreeWrapper<MidiSetupProperties>
{
public:
    MidiSetupProperties () noexcept : ValueTreeWrapper<MidiSetupProperties> (MidiSetupTypeId)
    {
    }
    MidiSetupProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<MidiSetupProperties> (MidiSetupTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setMode (int mode, bool includeSelfCallback);
    void setAssign (int assign, bool includeSelfCallback);
    void setBasicChannel (int basicChannel, bool includeSelfCallback);
    void setRcvProgramChange (int receiveChange, bool includeSelfCallback);
    void setXmtProgramChange (int transmitChange, bool includeSelfCallback);
    void setColACC (int cc, bool includeSelfCallback);
    void setColBCC (int cc, bool includeSelfCallback);
    void setColCCC (int cc, bool includeSelfCallback);
    void setPitchWheelSemi (int semiTones, bool includeSelfCallback);
    void setVelocityDepth (int velocityDepth, bool includeSelfCallback);
    void setNotifications (int notifications, bool includeSelfCallback);

    int getMode ();
    int getAssign ();
    int getBasicChannel ();
    int getRcvProgramChange ();
    int getXmtProgramChange ();
    int getColACC ();
    int getColBCC ();
    int getColCCC ();
    int getPitchWheelSemi ();
    int getVelocityDepth ();
    int getNotifications ();

    std::function<void (int mode)> onModeChange;
    std::function<void (int assign)> onAssignChange;
    std::function<void (int basciChannel)> onBasicChannelChange;
    std::function<void (int receiveChange)> onRcvProgramChangeChange;
    std::function<void (int mode)> onXmtProgramChangeChange;
    std::function<void (int mode)> onCollACCChange;
    std::function<void (int mode)> onCollBCCChange;
    std::function<void (int mode)> onCollCCCChange;
    std::function<void (int mode)> onPitchWheelSemiChange;
    std::function<void (int mode)> onVelocityDepthChange;
    std::function<void (int mode)> onNotificationsChange;

    static inline const juce::Identifier MidiSetupTypeId { "MidiSetup" };
    static inline const juce::Identifier ModePropertyId             { "mode" };
    static inline const juce::Identifier AssignPropertyId           { "assign" };
    static inline const juce::Identifier BasicChannelPropertyId     { "basicChannel" };
    static inline const juce::Identifier RcvProgramChangePropertyId { "rcvProgramChange" };
    static inline const juce::Identifier XmtProgramChangePropertyId { "xmtProgramChange" };
    static inline const juce::Identifier ColACCPropertyId           { "colACC" };
    static inline const juce::Identifier ColBCCPropertyId           { "colBCC" };
    static inline const juce::Identifier ColCCCPropertyId           { "colCCC" };
    static inline const juce::Identifier PitchWheelSemiPropertyId   { "pitchWheelSemi" };
    static inline const juce::Identifier VelocityDepthPropertyId    { "velocityDepth" };
    static inline const juce::Identifier NotificationsPropertyId    { "notifications" };

    void initValueTree () {}
    void processValueTree () {}

private:

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
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