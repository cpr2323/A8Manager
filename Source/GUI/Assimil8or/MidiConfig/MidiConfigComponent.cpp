#include "MidiConfigComponent.h"
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
    midiConfigComponentDialog.handleShowChange (show);
    setVisible (show);
}

void MidiConfigComponent::resized ()
{
    constexpr auto kContentWidth { 400 };
    constexpr auto kContentHeight { 459 };
    midiConfigComponentDialog.setBounds (getWidth () / 2 - kContentWidth / 2, 13, kContentWidth, kContentHeight);
}

void MidiConfigComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::grey.withAlpha (0.5f));
}