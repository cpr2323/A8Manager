#include "MidiConfigDialogComponent.h"
#include "../../../Assimil8or/MidiSetup/MidiSetupFile.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

MidiConfigDialogComponent::MidiConfigDialogComponent ()
{
    setOpaque (true);
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
        midiSetupTabs.addTab (juce::String::charToString ('1' + curMidiSetupIndex), juce::Colours::darkgrey, &midiSetupEditorComponents [curMidiSetupIndex], false);
    addAndMakeVisible (midiSetupTabs);

    saveButton.setButtonText ("SAVE");
    saveButton.setEnabled (false);
    addAndMakeVisible (saveButton);
    saveButton.onClick = [this] () { saveClicked (); };
    cancelButton.setButtonText ("CANCEL");
    addAndMakeVisible (cancelButton);
    cancelButton.onClick = [this] () { cancelClicked (); };
}

void MidiConfigDialogComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    guiControlProperties.wrap (runtimeRootProperties.getValueTree (), GuiControlProperties::WrapperType::client, GuiControlProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);

    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
    {
        MidiSetupProperties midiSetupProperties { {}, MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no };
        MidiSetupProperties uneditedMidiSetupProperties { {}, MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no };
        midiSetupPropertiesListVT.addChild (midiSetupProperties.getValueTree (), -1, nullptr);
        uneditedMidiSetupPropertiesListVT.addChild (midiSetupProperties.getValueTree ().createCopy (), -1, nullptr);

        midiSetupEditorComponents [curMidiSetupIndex].init (curMidiSetupIndex, midiSetupPropertiesListVT, uneditedMidiSetupPropertiesListVT);
    }
}

void MidiConfigDialogComponent::handleShowChange (bool show)
{
    if (show)
    {
        loadMidiSetups ();
        startTimer (250);
    }
    else
    {
        stopTimer ();
    }
}

void MidiConfigDialogComponent::loadMidiSetups ()
{
    // get current folder
    juce::File currentFolder { appProperties.getMostRecentFolder () };
    // iterate over possible midi setup files
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
    {
        MidiSetupProperties midiSetupProperties { midiSetupPropertiesListVT.getChild (curMidiSetupIndex), MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no };
        MidiSetupProperties uneditedMidiSetupProperties { uneditedMidiSetupPropertiesListVT.getChild (curMidiSetupIndex), MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no };
        auto midiSetupRawFile { currentFolder.getChildFile ("midi" + juce::String (curMidiSetupIndex + 1)).withFileExtension ("yml") };
        if (midiSetupRawFile.exists ())
        {
            MidiSetupFile midiSetupFile;
            juce::StringArray midiSetupFileLines;
            midiSetupRawFile.readLines (midiSetupFileLines);
            midiSetupProperties.copyFrom (midiSetupFile.parse (midiSetupFileLines));
        }
        else
        {
            midiSetupProperties.copyFrom (MidiSetupProperties ({}, MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no).getValueTree ());
        }
        uneditedMidiSetupProperties.copyFrom (midiSetupProperties.getValueTree ());
    }
}

void MidiConfigDialogComponent::cancelClicked ()
{
    if (anyMidiSetupsEdited)
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "Midi Setups Have Been Edited",
            "You have not saved your edited Midi Setups.\n  Select Continue to lose your changes.\n  Select Cancel to go back and save.", "Continue (lose changes)", "Cancel", nullptr,
            juce::ModalCallbackFunction::create ([this] (int option)
            {
                juce::MessageManager::callAsync ([this, option] ()
                {
                    if (option == 1) // Continue
                        closeDialog ();
                });
            }));
    }
    else
    {
        closeDialog ();
    }
}

void MidiConfigDialogComponent::closeDialog ()
{
    guiControlProperties.showMidiConfigWindow (false);
}

void MidiConfigDialogComponent::saveClicked ()
{
    juce::File currentFolder { appProperties.getMostRecentFolder () };
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
    {
        auto midiSetupRawFile { currentFolder.getChildFile ("midi" + juce::String (curMidiSetupIndex + 1)).withFileExtension ("yml") };
        MidiSetupFile midiSetupFile;
        midiSetupFile.write (midiSetupRawFile, midiSetupPropertiesListVT.getChild (curMidiSetupIndex));
    }
    closeDialog ();
}

void MidiConfigDialogComponent::timerCallback ()
{
    auto areMidiSetupsEqual = [] (juce::ValueTree uneditedMidiSetupPropertiesVT, juce::ValueTree midiSetupPropertiesVT)
    {
        MidiSetupProperties uneditedMidiSetupProperties (uneditedMidiSetupPropertiesVT, MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::no);
        MidiSetupProperties midiSetupProperties (midiSetupPropertiesVT, MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::no);
        return uneditedMidiSetupProperties.getMode () == midiSetupProperties.getMode () &&
               uneditedMidiSetupProperties.getAssign () == midiSetupProperties.getAssign () &&
               uneditedMidiSetupProperties.getBasicChannel () == midiSetupProperties.getBasicChannel () &&
               uneditedMidiSetupProperties.getRcvProgramChange () == midiSetupProperties.getRcvProgramChange () &&
               uneditedMidiSetupProperties.getXmtProgramChange () == midiSetupProperties.getXmtProgramChange () &&
               uneditedMidiSetupProperties.getColACC () == midiSetupProperties.getColACC () &&
               uneditedMidiSetupProperties.getColBCC () == midiSetupProperties.getColBCC () &&
               uneditedMidiSetupProperties.getColCCC () == midiSetupProperties.getColCCC () &&
               uneditedMidiSetupProperties.getPitchWheelSemi () == midiSetupProperties.getPitchWheelSemi () &&
               uneditedMidiSetupProperties.getVelocityDepth () == midiSetupProperties.getVelocityDepth () &&
               uneditedMidiSetupProperties.getNotifications () == midiSetupProperties.getNotifications ();
    };
    anyMidiSetupsEdited = false;
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
    {
        const auto midiSetupEdited { ! areMidiSetupsEqual (uneditedMidiSetupPropertiesListVT.getChild (curMidiSetupIndex), midiSetupPropertiesListVT.getChild (curMidiSetupIndex)) };
        midiSetupTabs.setTabName (curMidiSetupIndex, juce::String::charToString ('1' + curMidiSetupIndex) + (midiSetupEdited ? "*" : ""));
        anyMidiSetupsEdited |= midiSetupEdited;
    }
    saveButton.setEnabled (anyMidiSetupsEdited);
}

void MidiConfigDialogComponent::resized ()
{
    constexpr auto kButtonHeight { 25 };
    constexpr auto kButtonBorder { 5 };
    constexpr auto kButtonWidth { 60 };
    constexpr auto kBetweenButtons { 5 };
    auto localBounds { getLocalBounds () };
    localBounds.reduce (5, 5);
    const auto tabArea { localBounds.removeFromTop (getHeight () - kButtonHeight - (kButtonBorder * 2)) };
    midiSetupTabs.setBounds (tabArea);
    localBounds.removeFromTop (kButtonBorder);
    saveButton.setBounds (localBounds.removeFromRight (kButtonWidth));
    localBounds.removeFromRight (kBetweenButtons);
    cancelButton.setBounds (localBounds.removeFromRight (kButtonWidth));
}

void MidiConfigDialogComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds (), 1);
}