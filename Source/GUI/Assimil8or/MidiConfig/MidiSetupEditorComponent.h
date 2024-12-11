#pragma once

#include "../../../Assimil8or/MidiSetup/MidiSetupProperties.h"
#include "../../../Utility/CustomComboBox.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/NoArrowComboBoxLnF.h"

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
// Velocity Depth : (0-127)     - text box integer
// Notification: On/Off         - combobox

class MidiSetupEditorComponent : public juce::Component
{
public:
    MidiSetupEditorComponent ();
    ~MidiSetupEditorComponent ();
    void init (juce::ValueTree midiSetupPropertiesVT);

private:
    MidiSetupProperties midiSetupProperties;

    juce::Label modeLabel;
    CustomComboBox modeComboBox;
    juce::Label assignLabel;
    CustomComboBox assignComboBox;
    juce::Label basicChannelLabel;
    CustomComboBox basicChannelComboBox;
    juce::Label rcvProgramChangeLabel;
    CustomComboBox rcvProgramChangeComboBox;
    juce::Label xmtProgramChangeLabel;
    CustomComboBox xmtProgramChangeComboBox;
    juce::Label colACCLabel;
    CustomComboBox colACCComboBox;
    juce::Label colBCCLabel;
    CustomComboBox colBCCComboBox;
    juce::Label colCCCLabel;
    CustomComboBox colCCCComboBox;
    juce::Label pitchWheelSemiLabel;
    CustomComboBox pitchWheelSemiComboBox;
    juce::Label velocityDepthLabel;
    CustomTextEditorInt velocityDepthTextEditor;
    juce::Label notificationsLabel;
    CustomComboBox notificationsComboBox;

    NoArrowComboBoxLnF noArrowComboBoxLnF;

    void populateAssignmentComboBox (int mode);
    void populateColCCComboBox (juce::ComboBox& comboBox);

    void modeDataChanged (int mode);
    void modeUiChanged (int modeId);
    void assignDataChanged (int assign);
    void assignUiChanged (int assignId);
    void basicChannelDataChanged (int basicChannel);
    void basicChannelUiChanged (int basicChannelId);
    void rcvProgramChangeDataChanged (int rcvProgramChange);
    void rcvProgramChangeUiChanged (int rcvProgramChangeId);
    void xmtProgramChangeDataChanged (int xmtProgramChange);
    void xmtProgramChangeUiChanged (int xmtProgramChangeId);
    void colACCDataChanged (int colAcc);
    void colACCUiChanged (int colAccId);
    void colBCCDataChanged (int colBcc);
    void colBCCUiChanged (int colBccId);
    void colCCCDataChanged (int colCcc);
    void colCCCUiChanged (int colCccId);
    void pitchWheelSemiDataChanged (int pitchWheelSemi);
    void pitchWheelSemiUiChanged (int pitchWheelSemiId);
    void velocityDepthDataChanged (int velocityDepth);
    void velocityDepthUiChanged (int velocityDepth);
    void notificationsDataChanged (int notifications);
    void notificationsUiChanged (int notificationsId);

    void resized () override;
};