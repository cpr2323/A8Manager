#include "MidiSetupEditorComponent.h"

MidiSetupEditorComponent::MidiSetupEditorComponent ()
{
    auto setupLabel = [this] (juce::Label& label, juce::String text)
    {
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupComboBox = [this] (juce::ComboBox& comboBox, const std::vector<juce::String>& comboBoxItems)
    {
        for (auto itemIndex { 0 }; itemIndex < comboBoxItems.size (); ++itemIndex)
            comboBox.addItem (comboBoxItems [itemIndex], itemIndex + 1);
        addAndMakeVisible (comboBox);
    };
    setupLabel (modeLabel, "Mode");
    setupComboBox (modeComboBox, {"Omni", "Uni", "Multi"});
    setupLabel (assignLabel, "Assign");
    populateAssignmentComboBox (0);
    addAndMakeVisible (assignComboBox);
    setupLabel (basicChannelLabel, "Basic Channel");
    setupComboBox (basicChannelComboBox, {"1", "2", "3", "4","5", "6", "7", "8", "9", "10", "11", "12", "13", "14","15", "16" });
    setupLabel (rcvProgramChangeLabel, "Rcv Program Change");
    setupComboBox (rcvProgramChangeComboBox, {"Off", "Exists","Any"});
    setupLabel (xmtProgramChangeLabel, "Xmt Program Change");
    setupComboBox (xmtProgramChangeComboBox, { "Off", "On" });
    setupLabel (colACCLabel, "Col A CC");
    populateColCCComboBox (colACCComboBox);
    addAndMakeVisible (colACCComboBox);
    setupLabel (colBCCLabel, "Col B CC");
    populateColCCComboBox (colBCCComboBox);
    addAndMakeVisible (colBCCComboBox);
    setupLabel (colCCCLabel, "Col C CC");
    populateColCCComboBox (colCCCComboBox);
    addAndMakeVisible (colCCCComboBox);
    setupLabel (pitchWheelSemiLabel, "Pitch Wheel Semi");
    setupComboBox (pitchWheelSemiComboBox, {"+/- 0", "+/- 1", "+/- 2", "+/- 3", "+/- 4", "+/- 5", "+/- 6", "+/- 7", "+/- 8", "+/- 9", "+/- 10", "+/- 11", "+/- 12" });
    setupLabel (velocityDepthLabel, "Velocity Depth");
    addAndMakeVisible (velocityDepthTextEditor);
    setupLabel (notificationsLabel, "Notifications");
    setupComboBox (notificationsComboBox, { "Off", "On" });

    modeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    assignComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    basicChannelComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    rcvProgramChangeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    xmtProgramChangeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    colACCComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    colBCCComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    colCCCComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    pitchWheelSemiComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    notificationsComboBox.setLookAndFeel (&noArrowComboBoxLnF);
}

MidiSetupEditorComponent::~MidiSetupEditorComponent ()
{
    modeComboBox.setLookAndFeel (nullptr);
    assignComboBox.setLookAndFeel (nullptr);
    basicChannelComboBox.setLookAndFeel (nullptr);
    rcvProgramChangeComboBox.setLookAndFeel (nullptr);
    xmtProgramChangeComboBox.setLookAndFeel (nullptr);
    colACCComboBox.setLookAndFeel (nullptr);
    colBCCComboBox.setLookAndFeel (nullptr);
    colCCCComboBox.setLookAndFeel (nullptr);
    pitchWheelSemiComboBox.setLookAndFeel (nullptr);
    notificationsComboBox.setLookAndFeel (nullptr);
}

void MidiSetupEditorComponent::init (juce::ValueTree midiSetupPropertiesVT)
{
    midiSetupProperties.wrap (midiSetupPropertiesVT, MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::yes);
    midiSetupProperties.onModeChange = [this] (int mode) { modeDataChanged (mode); };
    midiSetupProperties.onAssignChange = [this] (int assign) { assignDataChanged (assign); };
    midiSetupProperties.onBasicChannelChange = [this] (int basciChannel) { basicChannelDataChanged (basciChannel); };
    midiSetupProperties.onRcvProgramChangeChange = [this] (int rcvProgramChange) { rcvProgramChangeDataChanged (rcvProgramChange); };
    midiSetupProperties.onXmtProgramChangeChange = [this] (int xmtProgramChange) { xmtProgramChangeDataChanged (xmtProgramChange); };
    midiSetupProperties.onColACCChange = [this] (int colAcc) { colACCDataChanged (colAcc); };
    midiSetupProperties.onColBCCChange = [this] (int colBcc) { colBCCDataChanged (colBcc); };
    midiSetupProperties.onColCCCChange = [this] (int colCcc) { colCCCDataChanged (colCcc); };
    midiSetupProperties.onPitchWheelSemiChange = [this] (int pitchWheelSmemi) { pitchWheelSemiDataChanged (pitchWheelSmemi); };
    midiSetupProperties.onVelocityDepthChange = [this] (int velocityDepth) { colBCCDataChanged (velocityDepth); };
    midiSetupProperties.onNotificationsChange = [this] (int notifications) { notificationsDataChanged (notifications); };

    modeDataChanged (midiSetupProperties.getMode ());
    assignDataChanged (midiSetupProperties.getAssign ());
    basicChannelDataChanged (midiSetupProperties.getBasicChannel ());
    rcvProgramChangeDataChanged (midiSetupProperties.getRcvProgramChange ());
    xmtProgramChangeDataChanged (midiSetupProperties.getXmtProgramChange ());
    colACCDataChanged (midiSetupProperties.getColACC ());
    colBCCDataChanged (midiSetupProperties.getColBCC ());
    colCCCDataChanged (midiSetupProperties.getColCCC ());
    pitchWheelSemiDataChanged (midiSetupProperties.getPitchWheelSemi ());
    velocityDepthDataChanged (midiSetupProperties.getVelocityDepth ());
    notificationsDataChanged (midiSetupProperties.getNotifications ());

}

void MidiSetupEditorComponent::populateAssignmentComboBox (int mode)
{
    auto populateComboBox = [this] (int idOffset, const std::vector<juce::String>& comboBoxItems)
    {
        assignComboBox.clear ();
        for (auto itemIndex { 0 }; itemIndex < comboBoxItems.size (); ++itemIndex)
            assignComboBox.addItem (comboBoxItems [itemIndex], idOffset + itemIndex + 1);
    };

    if (mode == 0 || mode == 1)
        populateComboBox (0, { "Dynamic", "Dynamic II", "Chroma 8", "Chroma VZ", "Chroma KZ", "SP - 1200", "SP - 1200 X" });
    else
        populateComboBox (10, { "Multi", "Multi VZ", "Multi KZ", "No Trigger" });
}

void MidiSetupEditorComponent::populateColCCComboBox (juce::ComboBox& comboBox)
{
    int curIdValue { 1 };
    auto addItem = [&curIdValue, &comboBox] (juce::String text)
    {
        comboBox.addItem (text, curIdValue);
        ++curIdValue;
    };
    // CV in Jacks, -1
    addItem ("<= CV in Jacks");
    // CC# (0-95), 0 - 95
    for (auto curCCValue { 0 }; curCCValue < 96; ++curCCValue)
        addItem ("<= CC# " + juce::String (curCCValue));
    addItem ("<= CC# 16 - 23");
    addItem ("<= CC# 24-31");
    addItem ("<= CC# 104-111");
    addItem ("<= CC# 112-119");
    addItem ("<= Pressure");
    addItem ("<= Velocity");
    addItem ("<= Note #");
    addItem ("0..5V => Tx 16+");
    addItem ("0..5V => Tx 24+");
    addItem ("0..5V => Tx 104+");
    addItem ("0..5V => Tx 112+");
    addItem ("-5..5V => Tx 16+");
    addItem ("-5..5V => Tx 24+");
    addItem ("-5..5V => Tx 104+");
    addItem ("-5..5V => Tx 112+");
    addItem ("0..5V => multi 16");
    addItem ("0..5V => multi 24");
    addItem ("0..5V => multi 104+");
    addItem ("0..5V => multi 112+");
    addItem ("-5..5V => multi 16");
    addItem ("-5..5V => multi 24");
    addItem ("-5..5V => multi 104+");
    addItem ("-5..5V => multi 112+");
}

void MidiSetupEditorComponent::modeDataChanged (int mode)
{
    modeComboBox.setSelectedId (mode + 1, juce::NotificationType::dontSendNotification);
    populateAssignmentComboBox (mode);
}

void MidiSetupEditorComponent::modeUiChanged (int mode)
{
    midiSetupProperties.setMode (mode, false);
    populateAssignmentComboBox (mode);
}

void MidiSetupEditorComponent::assignDataChanged (int assign)
{
    assignComboBox.setSelectedId (assign + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::assignUiChanged (int assign)
{
    midiSetupProperties.setAssign (assign, false);
}

void MidiSetupEditorComponent::basicChannelDataChanged (int basciChannel)
{
    basicChannelComboBox.setSelectedId (basciChannel + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::basicChannelUiChanged (int basicChannel)
{
    midiSetupProperties.setBasicChannel (basicChannel, false);
}

void MidiSetupEditorComponent::rcvProgramChangeDataChanged (int rcvProgramChange)
{
    rcvProgramChangeComboBox.setSelectedId (rcvProgramChange + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::rcvProgramChangeUiChanged (int rcvProgramChange)
{
    midiSetupProperties.setRcvProgramChange (rcvProgramChange, false);
}

void MidiSetupEditorComponent::xmtProgramChangeDataChanged (int xmtProgramChange)
{
    xmtProgramChangeComboBox.setSelectedId (xmtProgramChange+ 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::xmtProgramChangeUiChanged (int xmtProgramChange)
{
    midiSetupProperties.setXmtProgramChange (xmtProgramChange, false);
}

void MidiSetupEditorComponent::colACCDataChanged (int colAcc)
{
    colACCComboBox.setSelectedId (colAcc + 2, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::colACCUiChanged (int colAcc)
{
    midiSetupProperties.setColACC (colAcc, false);
}

void MidiSetupEditorComponent::colBCCDataChanged (int colBcc)
{
    colBCCComboBox.setSelectedId (colBcc + 2, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::colBCCUiChanged (int colBcc)
{
    midiSetupProperties.setColBCC (colBcc, false);
}

void MidiSetupEditorComponent::colCCCDataChanged (int colCcc)
{
    colCCCComboBox.setSelectedId (colCcc + 2, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::colCCCUiChanged (int colCcc)
{
    midiSetupProperties.setColCCC (colCcc, false);
}

void MidiSetupEditorComponent::pitchWheelSemiDataChanged (int pitchWheelSemi)
{
    pitchWheelSemiComboBox.setSelectedId (pitchWheelSemi + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::pitchWheelSemiUiChanged (int pitchWheelSemi)
{
    midiSetupProperties.setPitchWheelSemi (pitchWheelSemi, false);
}

void MidiSetupEditorComponent::velocityDepthDataChanged (int velocityDepth)
{
    velocityDepthTextEditor.setText (juce::String (velocityDepth), juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::velocityDepthUiChanged (int velocityDepth)
{
    midiSetupProperties.setVelocityDepth (velocityDepth, false);
}

void MidiSetupEditorComponent::notificationsDataChanged (int notifications)
{
    notificationsComboBox.setSelectedId (notifications + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::notificationsUiChanged (int notifications)
{
    midiSetupProperties.setNotifications (notifications, false);
}

void MidiSetupEditorComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (5, 5);

    auto displayComponentPair = [this, &localBounds] (juce::Component& Label, juce::Component& editor)
    {
        auto displayLine { localBounds.removeFromTop (25) };
        Label.setBounds (displayLine.removeFromLeft (140));
        displayLine.removeFromLeft (10);
        editor.setBounds (displayLine.removeFromLeft (150));
        localBounds.removeFromTop (5);
    };
    displayComponentPair (modeLabel, modeComboBox);
    displayComponentPair (assignLabel, assignComboBox);
    displayComponentPair (basicChannelLabel, basicChannelComboBox);
    displayComponentPair (rcvProgramChangeLabel, rcvProgramChangeComboBox);
    displayComponentPair (xmtProgramChangeLabel, xmtProgramChangeComboBox);
    displayComponentPair (colACCLabel, colACCComboBox);
    displayComponentPair (colBCCLabel, colBCCComboBox);
    displayComponentPair (colCCCLabel, colCCCComboBox);
    displayComponentPair (pitchWheelSemiLabel, pitchWheelSemiComboBox);
    displayComponentPair (velocityDepthLabel, velocityDepthTextEditor);
    displayComponentPair (notificationsLabel, notificationsComboBox);
}