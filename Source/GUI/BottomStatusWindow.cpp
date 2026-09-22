#include "BottomStatusWindow.h"
#include "Theme/A8ColourIds.h"
#include "Theme/A8Fonts.h"
#include "oolib/Properties/RuntimeRootProperties.h"

BottomStatusWindow::BottomStatusWindow ()
{
    setOpaque (true);

    progressUpdateLabel.setFont (A8Type::statusMessage ());
    progressUpdateLabel.setBorderSize ({ 0, 0, 0, 0 });
    addAndMakeVisible (progressUpdateLabel);

    // Settings now lives in the path bar at the top, alongside the output device;
    // this strip is left for transient status messages.
}

void BottomStatusWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
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

void BottomStatusWindow::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    // status messages are secondary to the work, so they are set in the dim ink
    progressUpdateLabel.setColour (juce::Label::textColourId, findColour (A8Colours::textDim));
}

void BottomStatusWindow::paint (juce::Graphics& g)
{
    g.fillAll (findColour (A8Colours::listBackground));
    g.setColour (findColour (A8Colours::outline));
    g.drawHorizontalLine (0, 0.0f, static_cast<float> (getWidth ()));
}

void BottomStatusWindow::resized ()
{
    progressUpdateLabel.setBounds (getLocalBounds ().withTrimmedTop (1).reduced (9, 0));
}
