#include "Assimil8orEditorComponent.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/PersistentRootProperties.h"

Assimil8orEditorComponent::Assimil8orEditorComponent ()
{
    setOpaque (true);

    nameEditor.setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::navajowhite);
    nameEditor.setColour (juce::TextEditor::ColourIds::textColourId, juce::Colours::black);
    nameEditor.onFocusLost = [this] () { updateName (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { updateName (nameEditor.getText ()); };
    addAndMakeVisible (nameEditor);

    auto setupButton = [this] (juce::TextButton& button, juce::String text, std::function<void()> buttonFunction)
    {
        button.setButtonText (text);
        button.onClick = buttonFunction;
        addAndMakeVisible (button);
    };
    setupButton (saveButton, "Save", [this] () { savePreset ();  });
    setupButton (importButton, "Import", [this] () { importPreset ();  });
    setupButton (exportButton, "Export", [this] () { exportPreset (); });
}

void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.onNameChange = [this] (juce::String name) { refreshName (name); };
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    refreshName (presetProperties.getName ());
}

void Assimil8orEditorComponent::importPreset ()
{
    jassertfalse;
}

void Assimil8orEditorComponent::exportPreset ()
{
    jassertfalse;
}

juce::String Assimil8orEditorComponent::getPresetFileName (int presetIndex)
{
    const auto rawPresetIndexString { juce::String (presetIndex) };
    const auto presetIndexString { juce::String ("000").substring (0, 3 - rawPresetIndexString.length ()) + rawPresetIndexString };
    return "prst" + presetIndexString;
}

void Assimil8orEditorComponent::savePreset ()
{
    auto presetFile { juce::File (appProperties.getMRUList () [0]) };
    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.write (presetFile, presetProperties.getValueTree ());
}

void Assimil8orEditorComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey.darker(0.7f));
}

void Assimil8orEditorComponent::resized ()
{
    auto localBounds { getLocalBounds () };

    auto buttonRow { localBounds.removeFromBottom (28).withTrimmedLeft (3).withTrimmedBottom (3)};
    saveButton.setBounds (buttonRow.removeFromLeft (100));
    buttonRow.removeFromLeft (3);
    importButton.setBounds (buttonRow.removeFromLeft (100));
    buttonRow.removeFromLeft (3);
    exportButton.setBounds (buttonRow.removeFromLeft (100));

    nameEditor.setBounds (10, 10, 150, 25);
}

void Assimil8orEditorComponent::refreshName (juce::String name)
{
    nameEditor.setText (name, false);
}

void Assimil8orEditorComponent::updateName (juce::String name)
{
    presetProperties.setName (name, false);
}
