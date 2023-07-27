#include "Assimil8orPresetComponent.h"
#include "../../Utility/RuntimeRootProperties.h"

Assimil8orPresetComponent::Assimil8orPresetComponent ()
{
    setOpaque (true);

    nameEditor.onFocusLost = [this] () { updateName (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { updateName (nameEditor.getText ()); };
    addAndMakeVisible (nameEditor);
}

void Assimil8orPresetComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::yes);
    presetProperties.onNameChange = [this] (juce::String name) { refreshName (name); };
    refreshName (presetProperties.getName ());
}

void Assimil8orPresetComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orPresetComponent::refreshName (juce::String name)
{
    nameEditor.setText (name, false);
}

void Assimil8orPresetComponent::updateName (juce::String name)
{
    presetProperties.setName (name, false);
}

void Assimil8orPresetComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    nameEditor.setBounds (10, 10, 150, 25);
}
