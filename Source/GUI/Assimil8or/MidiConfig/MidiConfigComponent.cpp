#include "MidiConfigComponent.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

MidiConfigComponent::MidiConfigComponent ()
{
    addAndMakeVisible (midiConfigComponentDialog);
}

void MidiConfigComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    guiControlProperties.wrap (runtimeRootProperties.getValueTree (), GuiControlProperties::WrapperType::client, GuiControlProperties::EnableCallbacks::yes);
    guiControlProperties.onShowMidiConfigWindowChange = [this] (bool show) { handleShowChange (show); };
    midiConfigComponentDialog.init (rootPropertiesVT);
}

void MidiConfigComponent::handleShowChange (bool show)
{
    if (show)
        midiConfigComponentDialog.loadMidiSetups ();

    setVisible (show);
}

MidiConfigComponent::MidiConfigComponentDialog::MidiConfigComponentDialog ()
{
    setOpaque (true);
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
        midiSetupTabs.addTab (juce::String::charToString ('1' + curMidiSetupIndex), juce::Colours::darkgrey, &midiSetupComponents [curMidiSetupIndex], false);
    addAndMakeVisible (midiSetupTabs);

    saveButton.setButtonText ("SAVE");
    addAndMakeVisible (saveButton);
    saveButton.onClick = [this] () { saveClicked (); };
    cancelButton.setButtonText ("CANCEL");
    addAndMakeVisible (cancelButton);
    cancelButton.onClick = [this] () { cancelClicked (); };
}

void MidiConfigComponent::MidiConfigComponentDialog::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    guiControlProperties.wrap (runtimeRootProperties.getValueTree (), GuiControlProperties::WrapperType::client, GuiControlProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
}

void MidiConfigComponent::MidiConfigComponentDialog::loadMidiSetups ()
{
    // get current folder
    juce::File currentFolder { appProperties.getMostRecentFolder () };
    // iterate over possible midi setup files
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 9; ++curMidiSetupIndex)
    {
        auto midiSetupRawFile { currentFolder.getChildFile ("midi" + juce::String (curMidiSetupIndex + 1)).withFileExtension("yml") };
        if (midiSetupRawFile.exists ())
        {
            MidiSetupFile midiSetupFile;
            juce::StringArray midiSetupFileLines;
            midiSetupRawFile.readLines (midiSetupFileLines);
            midiSetupPropertiesList [curMidiSetupIndex].wrap (midiSetupFile.parse (midiSetupFileLines),MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no);
        }
        else
        {
            midiSetupPropertiesList [curMidiSetupIndex].wrap ({}, MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no);
        }
    }
}

void MidiConfigComponent::MidiConfigComponentDialog::cancelClicked ()
{
    // query if data has changed but not saved
    closeDialog ();
}

void MidiConfigComponent::MidiConfigComponentDialog::closeDialog ()
{
    guiControlProperties.showMidiConfigWindow (false);
}

void MidiConfigComponent::MidiConfigComponentDialog::saveClicked ()
{
    closeDialog ();
}

void MidiConfigComponent::MidiConfigComponentDialog::resized ()
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

void MidiConfigComponent::MidiConfigComponentDialog::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds (), 1);
}