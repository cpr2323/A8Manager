#include "MidiSetupEditorComponent.h"

MidiSetupEditorComponent::MidiSetupEditorComponent ()
{
    auto setupLabel = [this] (juce::Label& label, juce::String text)
    {
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupComboBox = [this] (juce::ComboBox& comboBox, const std::vector<juce::String>& comboBoxItems, std::function<void ()> onChangeCallback)
    {
        for (auto itemIndex { 0 }; itemIndex < comboBoxItems.size (); ++itemIndex)
            comboBox.addItem (comboBoxItems [itemIndex], itemIndex + 1);
        comboBox.setTooltip ("");
        comboBox.onChange = onChangeCallback;
        addAndMakeVisible (comboBox);
    };
    // MODE
    setupLabel (modeLabel, "Mode");
    modeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        modeUiChanged (std::clamp (modeComboBox.getSelectedId () + scrollAmount, 1, modeComboBox.getNumItems ()), true);
    };
    modeComboBox.onPopupMenuCallback = [this] () {};
    modeComboBox.setTooltip ("");
    setupComboBox (modeComboBox, { "Omni", "Uni", "Multi" }, [this] ()
    {
        modeUiChanged (modeComboBox.getSelectedId (), false);
        populateAssignmentComboBox (midiSetupProperties.getMode ());
    });

    // ASSIGN
    setupLabel (assignLabel, "Assign");
    assignComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        auto [minAssign, maxAssign] = [this] ()
            {
                const auto mode { midiSetupProperties.getMode () };
                if (mode == 0 || mode == 1)
                    return std::pair<int, int>{1, 7};
                else
                    return std::pair<int, int>{11, 14};
            } ();
        assignUiChanged (std::clamp (assignComboBox.getSelectedId () + scrollAmount, minAssign, maxAssign), true);
    };
    assignComboBox.onPopupMenuCallback = [this] () {};
    assignComboBox.setTooltip ("");
    populateAssignmentComboBox (0);
    assignComboBox.onChange = [this] () { assignUiChanged (assignComboBox.getSelectedId (), false); };
    addAndMakeVisible (assignComboBox);

    // BASIC CHANNEL
    setupLabel (basicChannelLabel, "Basic Channel");
    basicChannelComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        basicChannelUiChanged (std::clamp (basicChannelComboBox.getSelectedId () + scrollAmount, 1, basicChannelComboBox.getNumItems ()), true);
    };
    basicChannelComboBox.onPopupMenuCallback = [this] () {};
    basicChannelComboBox.setTooltip ("");
    setupComboBox (basicChannelComboBox, { "1", "2", "3", "4","5", "6", "7", "8", "9", "10", "11", "12", "13", "14","15", "16" },
                   [this] () { basicChannelUiChanged (basicChannelComboBox.getSelectedId (), false); });

    // RCV PROGRAM CHANGE
    setupLabel (rcvProgramChangeLabel, "Rcv Program Change");
    rcvProgramChangeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        rcvProgramChangeUiChanged (std::clamp (rcvProgramChangeComboBox.getSelectedId () + scrollAmount, 1, rcvProgramChangeComboBox.getNumItems ()), true);
    };
    rcvProgramChangeComboBox.onPopupMenuCallback = [this] () {};
    rcvProgramChangeComboBox.setTooltip ("");
    setupComboBox (rcvProgramChangeComboBox, { "Off", "Exists","Any" }, [this] () { rcvProgramChangeUiChanged (rcvProgramChangeComboBox.getSelectedId (), false); });

    // XMT PROGRAM CHANGE
    setupLabel (xmtProgramChangeLabel, "Xmt Program Change");
    xmtProgramChangeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        xmtProgramChangeUiChanged (std::clamp (xmtProgramChangeComboBox.getSelectedId () + scrollAmount, 1, xmtProgramChangeComboBox.getNumItems ()), true);
    };
    xmtProgramChangeComboBox.onPopupMenuCallback = [this] () {};
    xmtProgramChangeComboBox.setTooltip ("");
    setupComboBox (xmtProgramChangeComboBox, { "Off", "On" }, [this] () { xmtProgramChangeUiChanged (xmtProgramChangeComboBox.getSelectedId (), false); });

    // COL A CC
    setupLabel (colACCLabel, "Col A CC");
    colACCComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        colACCUiChanged (std::clamp (colACCComboBox.getSelectedId () + scrollAmount, 1, colACCComboBox.getNumItems ()), true);
    };
    colACCComboBox.onPopupMenuCallback = [this] () {};
    colACCComboBox.setTooltip ("");
    populateColCCComboBox (colACCComboBox);
    colACCComboBox.onChange = [this] () { colACCUiChanged (colACCComboBox.getSelectedId (), false); };
    addAndMakeVisible (colACCComboBox);

    // COL B CC
    setupLabel (colBCCLabel, "Col B CC");
    colBCCComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        colBCCUiChanged (std::clamp (colBCCComboBox.getSelectedId () + scrollAmount, 1, colBCCComboBox.getNumItems ()), true);
    };
    colBCCComboBox.onPopupMenuCallback = [this] () {};
    colBCCComboBox.setTooltip ("");
    populateColCCComboBox (colBCCComboBox);
    colBCCComboBox.onChange = [this] () { colBCCUiChanged (colBCCComboBox.getSelectedId (), false); };
    addAndMakeVisible (colBCCComboBox);

    // COL C CC
    setupLabel (colCCCLabel, "Col C CC");
    colCCCComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        colCCCUiChanged (std::clamp (colCCCComboBox.getSelectedId () + scrollAmount, 1, colCCCComboBox.getNumItems ()), true);
    };
    colCCCComboBox.onPopupMenuCallback = [this] () {};
    colCCCComboBox.setTooltip ("");
    populateColCCComboBox (colCCCComboBox);
    colCCCComboBox.onChange = [this] () { colCCCUiChanged (colCCCComboBox.getSelectedId (), false); };
    addAndMakeVisible (colCCCComboBox);

    // PITCH WHEEL SEMI
    setupLabel (pitchWheelSemiLabel, "Pitch Wheel Semi");
    pitchWheelSemiComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
        {
            const auto scrollAmount { 1 * direction };
            pitchWheelSemiUiChanged (std::clamp (pitchWheelSemiComboBox.getSelectedId () + scrollAmount, 1, pitchWheelSemiComboBox.getNumItems ()), true);
        };
    pitchWheelSemiComboBox.onPopupMenuCallback = [this] () {};
    pitchWheelSemiComboBox.setTooltip ("");
    setupComboBox (pitchWheelSemiComboBox, { "+/- 0", "+/- 1", "+/- 2", "+/- 3", "+/- 4", "+/- 5", "+/- 6", "+/- 7", "+/- 8", "+/- 9", "+/- 10", "+/- 11", "+/- 12" },
                   [this] () { pitchWheelSemiUiChanged (pitchWheelSemiComboBox.getSelectedId(), false); });

    // VELOCITY DEPTH
    setupLabel (velocityDepthLabel, "Velocity Depth");
    velocityDepthTextEditor.getMinValueCallback = [this] () { return 0; };
    velocityDepthTextEditor.getMaxValueCallback = [this] () { return 127; };
    velocityDepthTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    velocityDepthTextEditor.updateDataCallback = [this] (int value) { velocityDepthUiChanged (value, false); };
    velocityDepthTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 5;
            else
                return 10;
        } ();
        const auto newValue { midiSetupProperties.getVelocityDepth () + (multiplier * direction) };
        velocityDepthTextEditor.setValue (newValue);
    };
    velocityDepthTextEditor.onPopupMenuCallback = [this] ()
    {
    };
    velocityDepthTextEditor.setTooltip ("");
    addAndMakeVisible (velocityDepthTextEditor);

    // NOTIFICATIONS
    setupLabel (notificationsLabel, "Notifications");
    notificationsComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
        {
            const auto scrollAmount { 1 * direction };
            notificationsUiChanged (std::clamp (notificationsComboBox.getSelectedId () + scrollAmount, 1, notificationsComboBox.getNumItems ()), true);
        };
    notificationsComboBox.onPopupMenuCallback = [this] () {};
    notificationsComboBox.setTooltip ("");
    setupComboBox (notificationsComboBox, { "Off", "On" }, [this] () { notificationsUiChanged (notificationsComboBox.getSelectedId (), false); });

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
    auto newAssignSelection = [this, mode] ()
    {
        const auto curAssignSelection { midiSetupProperties.getAssign () };
        if (mode == 0 || mode == 1)
            return curAssignSelection < 7 ? curAssignSelection : 0;
        else
            return curAssignSelection > 6 ? curAssignSelection : 10;
    } ();
    assignDataChanged (newAssignSelection);
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

void MidiSetupEditorComponent::modeUiChanged (int modeId, bool updateUi)
{
    midiSetupProperties.setMode (modeId - 1, updateUi);
    populateAssignmentComboBox (modeId - 1);
}

void MidiSetupEditorComponent::assignDataChanged (int assign)
{
    assignComboBox.setSelectedId (assign + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::assignUiChanged (int assignId, bool updateUi)
{
    midiSetupProperties.setAssign (assignId - 1, updateUi);
}

void MidiSetupEditorComponent::basicChannelDataChanged (int basciChannel)
{
    basicChannelComboBox.setSelectedId (basciChannel + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::basicChannelUiChanged (int basicChannelId, bool updateUi)
{
    midiSetupProperties.setBasicChannel (basicChannelId - 1, updateUi);
}

void MidiSetupEditorComponent::rcvProgramChangeDataChanged (int rcvProgramChange)
{
    rcvProgramChangeComboBox.setSelectedId (rcvProgramChange + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::rcvProgramChangeUiChanged (int rcvProgramChangeId, bool updateUi)
{
    midiSetupProperties.setRcvProgramChange (rcvProgramChangeId - 1, updateUi);
}

void MidiSetupEditorComponent::xmtProgramChangeDataChanged (int xmtProgramChange)
{
    xmtProgramChangeComboBox.setSelectedId (xmtProgramChange+ 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::xmtProgramChangeUiChanged (int xmtProgramChangeId, bool updateUi)
{
    midiSetupProperties.setXmtProgramChange (xmtProgramChangeId - 1, updateUi);
}

void MidiSetupEditorComponent::colACCDataChanged (int colAcc)
{
    colACCComboBox.setSelectedId (colAcc + 2, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::colACCUiChanged (int colAccId, bool updateUi)
{
    midiSetupProperties.setColACC (colAccId - 2, updateUi);
}

void MidiSetupEditorComponent::colBCCDataChanged (int colBcc)
{
    colBCCComboBox.setSelectedId (colBcc + 2, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::colBCCUiChanged (int colBccId, bool updateUi)
{
    midiSetupProperties.setColBCC (colBccId - 2, updateUi);
}

void MidiSetupEditorComponent::colCCCDataChanged (int colCcc)
{
    colCCCComboBox.setSelectedId (colCcc + 2, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::colCCCUiChanged (int colCccId, bool updateUi)
{
    midiSetupProperties.setColCCC (colCccId - 2, updateUi);
}

void MidiSetupEditorComponent::pitchWheelSemiDataChanged (int pitchWheelSemi)
{
    pitchWheelSemiComboBox.setSelectedId (pitchWheelSemi + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::pitchWheelSemiUiChanged (int pitchWheelSemiId, bool updateUi)
{
    midiSetupProperties.setPitchWheelSemi (pitchWheelSemiId - 1, updateUi);
}

void MidiSetupEditorComponent::velocityDepthDataChanged (int velocityDepth)
{
    velocityDepthTextEditor.setText (juce::String (velocityDepth), juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::velocityDepthUiChanged (int velocityDepth, bool updateUi)
{
    midiSetupProperties.setVelocityDepth (velocityDepth, updateUi);
}

void MidiSetupEditorComponent::notificationsDataChanged (int notifications)
{
    notificationsComboBox.setSelectedId (notifications + 1, juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::notificationsUiChanged (int notificationsId, bool updateUi)
{
    midiSetupProperties.setNotifications (notificationsId - 1, updateUi);
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