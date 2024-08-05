#include "BottomStatusWindow.h"
#include "../Utility/RuntimeRootProperties.h"

BottomStatusWindow::BottomStatusWindow ()
{
    progressUpdateLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
    addAndMakeVisible (progressUpdateLabel);

    settingsButton.setButtonText ("SETTINGS");
    settingsButton.onClick = [this] ()
    {
        audioPlayerProperties.showConfigDialog (false);
    };
    addAndMakeVisible (settingsButton);
}

void BottomStatusWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::owner, AudioPlayerProperties::EnableCallbacks::yes);

    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorProperties::WrapperType::client, ValidatorProperties::EnableCallbacks::yes);
    validatorProperties.onProgressUpdateChanged = [this] (juce::String progressUpdate)
    {
        juce::MessageManager::callAsync ([this, progressUpdate] ()
        {
            updateProgress (progressUpdate);
        });
    };
}

void BottomStatusWindow::updateProgress (juce::String progressUpdate)
{
    progressUpdateLabel.setText (progressUpdate, juce::NotificationType::dontSendNotification);
}

void BottomStatusWindow::paint (juce::Graphics& g)
{
    g.fillAll (progressUpdateLabel.findColour (juce::Label::ColourIds::backgroundColourId).brighter (0.9f));
}

void BottomStatusWindow::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (5, 3);

    progressUpdateLabel.setBounds (localBounds);
    const auto buttonWidth { 70 };
    settingsButton.setBounds (getWidth () - 5 - buttonWidth, getHeight () / 2 - 10, buttonWidth, 20);
}
