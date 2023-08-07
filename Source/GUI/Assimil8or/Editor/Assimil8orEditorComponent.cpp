#include "Assimil8orEditorComponent.h"
#include "../../../Utility/RuntimeRootProperties.h"

Assimil8orEditorComponent::Assimil8orEditorComponent ()
{
    setOpaque (true);

    nameEditor.setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::navajowhite);
    nameEditor.setColour (juce::TextEditor::ColourIds::textColourId, juce::Colours::black);
    nameEditor.onFocusLost = [this] () { updateName (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { updateName (nameEditor.getText ()); };
    addAndMakeVisible (nameEditor);
}

void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.onNameChange = [this] (juce::String name) { refreshName (name); };

    refreshName (presetProperties.getName ());
}

void Assimil8orEditorComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey.darker(0.7f));
}

void Assimil8orEditorComponent::refreshName (juce::String name)
{
    nameEditor.setText (name, false);
}

void Assimil8orEditorComponent::updateName (juce::String name)
{
    presetProperties.setName (name, false);
}

void Assimil8orEditorComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    nameEditor.setBounds (10, 10, 150, 25);
}
