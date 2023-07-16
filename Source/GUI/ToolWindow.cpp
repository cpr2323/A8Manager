#include "ToolWindow.h"

ToolWindow::ToolWindow ()
{
    fileMenuButton.setButtonText ("Files");
    fileMenuButton.onClick = [this] ()
    {
        juce::PopupMenu pm;
        pm.addItem ("Verify", true, false, [this] () { loadFile (); });
        pm.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (fileMenuButton);

}

void ToolWindow::init (juce::ValueTree persistentRootPropertiesVT, juce::ValueTree runtimeRootPropertiesVT)
{
    appProperties.wrap (persistentRootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
}

void ToolWindow::loadFile ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the Assimil8or Preset file you want to verify...",
                                              appProperties.getMostRecentFolder (), "*.lpproject;*.pchains"));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
        {
//             if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
//                 lightPlugFiles.loadFile (fc.getURLResults () [0].getLocalFile ());
        }, nullptr);
}

void ToolWindow::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::cadetblue);
}

void ToolWindow::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (5, 3);

    fileMenuButton.setBounds (localBounds.removeFromLeft (100));
}
