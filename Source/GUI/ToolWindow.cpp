#include "ToolWindow.h"
#include "../Utility/RuntimeRootProperties.h"

ToolWindow::ToolWindow ()
{
    scanningStatusLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    addAndMakeVisible (scanningStatusLabel);
    progressUpdateLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::darkgreen);
    addAndMakeVisible (progressUpdateLabel);
}

void ToolWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorProperties::WrapperType::client, ValidatorProperties::EnableCallbacks::yes);
    validatorProperties.onScanStatusChanged = [this] (juce::String scanStatus) { updateScanStatus (scanStatus); };
    validatorProperties.onProgressUpdateChanged = [this] (juce::String progressUpdate) { updateProgress (progressUpdate); };
}

void ToolWindow::updateScanStatus (juce::String scanStatus)
{
    if (scanStatus == "scanning")
    {
        scanningStatusLabel.setText ("Scanning file system", juce::NotificationType::dontSendNotification);
    }
    else if (scanStatus == "idle")
    {
        scanningStatusLabel.setText ("", juce::NotificationType::dontSendNotification);
    }
    else
    {
        jassertfalse;
    }
}

void ToolWindow::updateProgress (juce::String progressUpdate)
{
    progressUpdateLabel.setText (progressUpdate, juce::NotificationType::dontSendNotification);
}

void ToolWindow::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::cadetblue);
}

void ToolWindow::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (5, 3);

    scanningStatusLabel.setBounds (localBounds.removeFromLeft (100));
    progressUpdateLabel.setBounds (localBounds);
}
