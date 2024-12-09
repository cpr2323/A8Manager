#include "MidiConfigComponent.h"
#include "../../../Utility/RuntimeRootProperties.h"

MidiConfigComponent::MidiConfigComponent ()
{
    addAndMakeVisible (midiConfigComponentDialog);
}

void MidiConfigComponent::init (juce::ValueTree rootPropertiesVT)
{
    midiConfigComponentDialog.init (rootPropertiesVT);
}

MidiConfigComponent::MidiConfigComponentDialog::MidiConfigComponentDialog ()
{
    setOpaque (true);
    for (auto curMidiSetupIndex { 0 }; curMidiSetupIndex < 8; ++curMidiSetupIndex)
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
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    guiControlProperties.wrap (runtimeRootProperties.getValueTree (), GuiControlProperties::WrapperType::client, GuiControlProperties::EnableCallbacks::no);
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