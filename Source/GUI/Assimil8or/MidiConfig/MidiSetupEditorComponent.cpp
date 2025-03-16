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
    modeComboBox.onDragCallback = [this] ([[maybe_unused]]DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        modeUiChanged (std::clamp (modeComboBox.getSelectedId () + scrollAmount, 1, modeComboBox.getNumItems ()), true);
    };
    modeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setMode (midiSetupProperties.getMode (), false); },
                                                 [this] () { midiSetupProperties.setMode (1, true); },
                                                 [this] () { midiSetupProperties.setMode (uneditedMidiSetupProperties.getMode (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    modeComboBox.setTooltip ("");
    setupComboBox (modeComboBox, { "Omni", "Uni", "Multi" }, [this] ()
    {
        modeUiChanged (modeComboBox.getSelectedId (), false);
        populateAssignmentComboBox (midiSetupProperties.getMode ());
    });

    // ASSIGN
    setupLabel (assignLabel, "Assign");
    assignComboBox.onDragCallback = [this] ([[maybe_unused]] DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        auto [minAssign, maxAssign] = [this] ()
            {
                const auto mode { midiSetupProperties.getMode () };
                if (mode == 0 || mode == 1)
                    return std::pair<int, int>{1, 7};
                else
                    return std::pair<int, int>{11, 15};
            } ();
        assignUiChanged (std::clamp (assignComboBox.getSelectedId () + scrollAmount, minAssign, maxAssign), true);
    };
    assignComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties)
                                                 {
                                                     destMidiSetupProperties.setMode (midiSetupProperties.getMode (), false);
                                                     destMidiSetupProperties.setAssign (midiSetupProperties.getAssign (), false);
                                                 },
                                                 [this] ()
                                                 {
                                                     midiSetupProperties.setMode (1, true);
                                                     midiSetupProperties.setAssign (0, false);
                                                 },
                                                 [this] ()
                                                 {
                                                     midiSetupProperties.setMode (uneditedMidiSetupProperties.getMode (), true);
                                                     midiSetupProperties.setAssign (uneditedMidiSetupProperties.getAssign (), true);
                                                 }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    assignComboBox.setTooltip ("");
    populateAssignmentComboBox (0);
    assignComboBox.onChange = [this] () { assignUiChanged (assignComboBox.getSelectedId (), false); };
    addAndMakeVisible (assignComboBox);

    // INDEX BASE KEY
    setupLabel (indexBaseKeyLabel, "Index Base Key");
    indexBaseKeyTextEditor.getMinValueCallback = [this] () { return 0; };
    indexBaseKeyTextEditor.getMaxValueCallback = [this] () { return 127; }; // possibly 119, to leave room for the 8 successive keys
    indexBaseKeyTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    indexBaseKeyTextEditor.updateDataCallback = [this] (int value) { indexBaseKeyUiChanged (value, false); };
    indexBaseKeyTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
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
        const auto newValue { midiSetupProperties.getIndexBaseKey () + (multiplier * direction) };
        indexBaseKeyTextEditor.setValue (newValue);
    };
    indexBaseKeyTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setIndexBaseKey (midiSetupProperties.getIndexBaseKey (), false); },
                                                 [this] () { midiSetupProperties.setIndexBaseKey (0, true); },
                                                 [this] () { midiSetupProperties.setIndexBaseKey (uneditedMidiSetupProperties.getIndexBaseKey (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    indexBaseKeyTextEditor.setTooltip ("");
    addAndMakeVisible (indexBaseKeyTextEditor);

    // BASIC CHANNEL
    setupLabel (basicChannelLabel, "Basic Channel");
    basicChannelComboBox.onDragCallback = [this] ([[maybe_unused]] DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        basicChannelUiChanged (std::clamp (basicChannelComboBox.getSelectedId () + scrollAmount, 1, basicChannelComboBox.getNumItems ()), true);
    };
    basicChannelComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setBasicChannel (midiSetupProperties.getBasicChannel (), false); },
                                                 [this] () { midiSetupProperties.setBasicChannel (0, true); },
                                                 [this] () { midiSetupProperties.setBasicChannel( uneditedMidiSetupProperties.getBasicChannel (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    basicChannelComboBox.setTooltip ("");
    setupComboBox (basicChannelComboBox, { "1", "2", "3", "4","5", "6", "7", "8", "9", "10", "11", "12", "13", "14","15", "16" },
                   [this] () { basicChannelUiChanged (basicChannelComboBox.getSelectedId (), false); });

    // RCV PROGRAM CHANGE
    setupLabel (rcvProgramChangeLabel, "Rcv Program Change");
    rcvProgramChangeComboBox.onDragCallback = [this] ([[maybe_unused]] DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        rcvProgramChangeUiChanged (std::clamp (rcvProgramChangeComboBox.getSelectedId () + scrollAmount, 1, rcvProgramChangeComboBox.getNumItems ()), true);
    };
    rcvProgramChangeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setRcvProgramChange (midiSetupProperties.getRcvProgramChange (), false); },
                                                 [this] () { midiSetupProperties.setRcvProgramChange (1, true); },
                                                 [this] () { midiSetupProperties.setRcvProgramChange (uneditedMidiSetupProperties.getRcvProgramChange (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    rcvProgramChangeComboBox.setTooltip ("");
    setupComboBox (rcvProgramChangeComboBox, { "Off", "Exists","Any" }, [this] () { rcvProgramChangeUiChanged (rcvProgramChangeComboBox.getSelectedId (), false); });

    // XMT PROGRAM CHANGE
    setupLabel (xmtProgramChangeLabel, "Xmt Program Change");
    xmtProgramChangeComboBox.onDragCallback = [this] ([[maybe_unused]] DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        xmtProgramChangeUiChanged (std::clamp (xmtProgramChangeComboBox.getSelectedId () + scrollAmount, 1, xmtProgramChangeComboBox.getNumItems ()), true);
    };
    xmtProgramChangeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setXmtProgramChange (midiSetupProperties.getXmtProgramChange (), false); },
                                                 [this] () { midiSetupProperties.setXmtProgramChange (1, true); },
                                                 [this] () { midiSetupProperties.setXmtProgramChange (uneditedMidiSetupProperties.getXmtProgramChange (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };

    xmtProgramChangeComboBox.setTooltip ("");
    setupComboBox (xmtProgramChangeComboBox, { "Off", "On" }, [this] () { xmtProgramChangeUiChanged (xmtProgramChangeComboBox.getSelectedId (), false); });

    // COL A CC
    setupLabel (colACCLabel, "Col A CC");
    colACCComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        colACCUiChanged (std::clamp (colACCComboBox.getSelectedId () + scrollAmount, 1, colACCComboBox.getNumItems ()), true);
    };
    colACCComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setColACC (midiSetupProperties.getColACC (), false); },
                                                 [this] () { midiSetupProperties.setColACC (-1, true); },
                                                 [this] () { midiSetupProperties.setColACC (uneditedMidiSetupProperties.getColACC (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
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
    colBCCComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setColBCC (midiSetupProperties.getColBCC (), false); },
                                                 [this] () { midiSetupProperties.setColBCC (-1, true); },
                                                 [this] () { midiSetupProperties.setColBCC (uneditedMidiSetupProperties.getColBCC (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
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
    colCCCComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setColCCC (midiSetupProperties.getColCCC (), false); },
                                                [this] () { midiSetupProperties.setColCCC (-1, true); },
                                                [this] () { midiSetupProperties.setColCCC (uneditedMidiSetupProperties.getColCCC (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    colCCCComboBox.setTooltip ("");
    populateColCCComboBox (colCCCComboBox);
    colCCCComboBox.onChange = [this] () { colCCCUiChanged (colCCCComboBox.getSelectedId (), false); };
    addAndMakeVisible (colCCCComboBox);

    // PITCH WHEEL SEMI
    setupLabel (pitchWheelSemiLabel, "Pitch Wheel Semi");
    pitchWheelSemiComboBox.onDragCallback = [this] ([[maybe_unused]] DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        pitchWheelSemiUiChanged (std::clamp (pitchWheelSemiComboBox.getSelectedId () + scrollAmount, 1, pitchWheelSemiComboBox.getNumItems ()), true);
    };
    pitchWheelSemiComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setPitchWheelSemi (midiSetupProperties.getPitchWheelSemi (), false); },
                                                 [this] () { midiSetupProperties.setPitchWheelSemi (12, true); },
                                                 [this] () { midiSetupProperties.setPitchWheelSemi (uneditedMidiSetupProperties.getPitchWheelSemi (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    pitchWheelSemiComboBox.setTooltip ("");
    for (auto pitchBendAmount { 0 }; pitchBendAmount < 49; ++pitchBendAmount)
        pitchWheelSemiComboBox.addItem ("+/- " + juce::String (pitchBendAmount), pitchBendAmount + 1);
    pitchWheelSemiComboBox.onChange = [this] () { pitchWheelSemiUiChanged (pitchWheelSemiComboBox.getSelectedId (), false); };
    addAndMakeVisible (pitchWheelSemiComboBox);

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
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setVelocityDepth (midiSetupProperties.getVelocityDepth (), false); },
                                                 [this] () { midiSetupProperties.setVelocityDepth (32, true); },
                                                 [this] () { midiSetupProperties.setVelocityDepth (uneditedMidiSetupProperties.getVelocityDepth (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    velocityDepthTextEditor.setTooltip ("");
    addAndMakeVisible (velocityDepthTextEditor);

    // NOTIFICATIONS
    setupLabel (notificationsLabel, "Notifications");
    notificationsComboBox.onDragCallback = [this] ([[maybe_unused]] DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { 1 * direction };
        notificationsUiChanged (std::clamp (notificationsComboBox.getSelectedId () + scrollAmount, 1, notificationsComboBox.getNumItems ()), true);
    };
    notificationsComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createMidiSetupEditMenu ([this] (MidiSetupProperties& destMidiSetupProperties) { destMidiSetupProperties.setNotifications (midiSetupProperties.getNotifications (), false); },
                                                 [this] () { midiSetupProperties.setNotifications (1, true); },
                                                 [this] () { midiSetupProperties.setNotifications (uneditedMidiSetupProperties.getNotifications (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    notificationsComboBox.setTooltip ("");
    setupComboBox (notificationsComboBox, { "Off", "On" }, [this] () { notificationsUiChanged (notificationsComboBox.getSelectedId (), false); });

    toolsButton.setButtonText ("TOOLS");
    toolsButton.onClick = [this] ()
    {
        juce::PopupMenu toolsMenu;
        toolsMenu.addSubMenu ("Clone", createMidiSetupCloneMenu ([this] (MidiSetupProperties& destMidiSetupProperties)
        {
            destMidiSetupProperties.copyFrom(midiSetupProperties.getValueTree());
        }), true);
        toolsMenu.addItem ("Default", true, false, [this] ()
        {
            MidiSetupProperties defaultMidiSetupProperties ({}, MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no);
            midiSetupProperties.copyFrom (defaultMidiSetupProperties.getValueTree ());
        });
        toolsMenu.addItem ("Revert", true, false, [this] ()
        {
            midiSetupProperties.copyFrom (uneditedMidiSetupProperties.getValueTree ());
        });
        toolsMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (toolsButton);

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

juce::PopupMenu MidiSetupEditorComponent::createMidiSetupCloneMenu (std::function <void (MidiSetupProperties&)> setter)
{
    jassert (setter != nullptr);
    juce::PopupMenu cloneMenu;
    for (auto destMidiSetupIndex { 0 }; destMidiSetupIndex < 9; ++destMidiSetupIndex)
    {
        if (destMidiSetupIndex != midiSetupIndex)
        {
            cloneMenu.addItem ("To Midi Setup " + juce::String (destMidiSetupIndex + 1), true, false, [this, destMidiSetupIndex, setter] ()
            {
                MidiSetupProperties destMidiSetupProperties (midiSetupPropertiesListVT.getChild (destMidiSetupIndex), MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::no);
                setter (destMidiSetupProperties);
            });
        }
    }
    cloneMenu.addItem ("To All", true, false, [this, setter] ()
    {
        // clone to other midi setups
        for (auto destMidiSetupIndex { 0 }; destMidiSetupIndex < 9; ++destMidiSetupIndex)
        {
            if (destMidiSetupIndex != midiSetupIndex)
            {
                MidiSetupProperties destMidiSetupProperties (midiSetupPropertiesListVT.getChild (destMidiSetupIndex), MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::no);
                setter (destMidiSetupProperties);
            }
            }
    });
    return cloneMenu;
}

juce::PopupMenu MidiSetupEditorComponent::createMidiSetupEditMenu (std::function <void (MidiSetupProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter)
{
    juce::PopupMenu editMenu;
    editMenu.addSubMenu ("Clone", createMidiSetupCloneMenu (setter), true);
    if (resetter != nullptr)
        editMenu.addItem ("Default", true, false, [this, resetter] () { resetter (); });
    if (reverter != nullptr)
        editMenu.addItem ("Revert", true, false, [this, reverter] () { reverter (); });
    return editMenu;
};

void MidiSetupEditorComponent::init (int theMidiSetupIndex, juce::ValueTree theMidiSetupPropertiesListVT, juce::ValueTree uneditedMidiSetupPropertiesListVT)
{
    midiSetupPropertiesListVT = theMidiSetupPropertiesListVT;
    midiSetupIndex = theMidiSetupIndex;
    
    uneditedMidiSetupProperties.wrap (uneditedMidiSetupPropertiesListVT.getChild (midiSetupIndex), MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::yes);
    midiSetupProperties.wrap (midiSetupPropertiesListVT.getChild (midiSetupIndex), MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::yes);
    midiSetupProperties.onAssignChange = [this] (int assign) { assignDataChanged (assign); };
    midiSetupProperties.onBasicChannelChange = [this] (int basciChannel) { basicChannelDataChanged (basciChannel); };
    midiSetupProperties.onColACCChange = [this] (int colAcc) { colACCDataChanged (colAcc); };
    midiSetupProperties.onColBCCChange = [this] (int colBcc) { colBCCDataChanged (colBcc); };
    midiSetupProperties.onColCCCChange = [this] (int colCcc) { colCCCDataChanged (colCcc); };
    midiSetupProperties.onIndexBaseKeyChange = [this] (int baseKey) { indexBaseKeyDataChanged (baseKey); };
    midiSetupProperties.onModeChange = [this] (int mode) { modeDataChanged (mode); };
    midiSetupProperties.onNotificationsChange = [this] (int notifications) { notificationsDataChanged (notifications); };
    midiSetupProperties.onRcvProgramChangeChange = [this] (int rcvProgramChange) { rcvProgramChangeDataChanged (rcvProgramChange); };
    midiSetupProperties.onPitchWheelSemiChange = [this] (int pitchWheelSmemi) { pitchWheelSemiDataChanged (pitchWheelSmemi); };
    midiSetupProperties.onVelocityDepthChange = [this] (int velocityDepth) { velocityDepthDataChanged (velocityDepth); };
    midiSetupProperties.onXmtProgramChangeChange = [this] (int xmtProgramChange) { xmtProgramChangeDataChanged (xmtProgramChange); };

    assignDataChanged (midiSetupProperties.getAssign ());
    basicChannelDataChanged (midiSetupProperties.getBasicChannel ());
    colACCDataChanged (midiSetupProperties.getColACC ());
    colBCCDataChanged (midiSetupProperties.getColBCC ());
    colCCCDataChanged (midiSetupProperties.getColCCC ());
    indexBaseKeyDataChanged (midiSetupProperties.getIndexBaseKey ());
    modeDataChanged (midiSetupProperties.getMode ());
    notificationsDataChanged (midiSetupProperties.getNotifications ());
    pitchWheelSemiDataChanged (midiSetupProperties.getPitchWheelSemi ());
    rcvProgramChangeDataChanged (midiSetupProperties.getRcvProgramChange ());
    velocityDepthDataChanged (midiSetupProperties.getVelocityDepth ());
    xmtProgramChangeDataChanged (midiSetupProperties.getXmtProgramChange ());

}

void MidiSetupEditorComponent::populateAssignmentComboBox (int mode)
{
    auto populateComboBox = [this] (int idOffset, const std::vector<juce::String>& comboBoxItems)
    {
        assignComboBox.clear ();
        for (auto itemIndex { 0 }; itemIndex < comboBoxItems.size (); ++itemIndex)
            assignComboBox.addItem (comboBoxItems [itemIndex], idOffset + itemIndex + 1);
    };

    if (mode == 0 || mode == 1) // Omni || Uni
        populateComboBox (0, { "Dynamic", "Dynamic II", "Chroma 8", "Chroma VZ", "Chroma KZ", "SP - 1200", "SP - 1200 X" });
    else // Multi
        populateComboBox (10, { "Multi", "Multi VZ", "Multi KZ", "No Trigger", "Indexed KZ" });
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

void MidiSetupEditorComponent::indexBaseKeyDataChanged (int baseKey)
{
    indexBaseKeyTextEditor.setText (juce::String (baseKey), juce::NotificationType::dontSendNotification);
}

void MidiSetupEditorComponent::indexBaseKeyUiChanged (int baseKey, bool updateUi)
{
    midiSetupProperties.setIndexBaseKey (baseKey, updateUi);
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
    displayComponentPair (indexBaseKeyLabel, indexBaseKeyTextEditor);
    displayComponentPair (basicChannelLabel, basicChannelComboBox);
    displayComponentPair (rcvProgramChangeLabel, rcvProgramChangeComboBox);
    displayComponentPair (xmtProgramChangeLabel, xmtProgramChangeComboBox);
    displayComponentPair (colACCLabel, colACCComboBox);
    displayComponentPair (colBCCLabel, colBCCComboBox);
    displayComponentPair (colCCCLabel, colCCCComboBox);
    displayComponentPair (pitchWheelSemiLabel, pitchWheelSemiComboBox);
    displayComponentPair (velocityDepthLabel, velocityDepthTextEditor);
    displayComponentPair (notificationsLabel, notificationsComboBox);

    localBounds.removeFromLeft (10);
    toolsButton.setBounds (localBounds.removeFromTop (20).removeFromLeft (40));
}