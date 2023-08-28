#include "MainComponent.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

const auto toolWindowHeight { 30 };

//  +----------------+-------------+-----------------------------------+
//  |Current Path                                                      |
//  +----------------+-------------+-----------------------------------+
//  | ..             | Preset 1    |                                   |
//  | folderX        | Preset 2    |                                   |
//  | folder34       | Preset 3    |                                   |
//  | fileAbc        | Preset 4    |                                   |
//  | fileElif       | Preset ...  |                                   |
// ...              ...           ...                                 ...
//  |                | Preset 199  |                                   |
//  +----------------+-------------+-----------------------------------+
//  | Type | Fix | Message (X items)                                   |
//  +------+-----+-----------------------------------------------------+
//  |      |     |                                                     |
//  |      |     |                                                     |
//  |      |     |                                                     |
//  |      |     |                                                     |
//  |      |     |                                                     |
//  +------+-----+-----------------------------------------------------+
//  |  Tool Bar                                                        |
//  +------------------------------------------------------------------+

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1085, 585);

    addAndMakeVisible (currentFolder);

    assimil8orEditorComponent.init (rootPropertiesVT);
    assimil8orValidatorComponent.init (rootPropertiesVT);
    fileViewComponent.init (rootPropertiesVT);
    presetListComponent.init (rootPropertiesVT);
    toolWindow.init (rootPropertiesVT);

    presetListEditorSplitter.setComponents (&presetListComponent, &assimil8orEditorComponent);
    presetListEditorSplitter.setHorizontalSplit (false);
    presetListEditorSplitter.setLayout (0, -0.06);

    folderBrowserEditorSplitter.setComponents (&fileViewComponent, &presetListEditorSplitter);
    folderBrowserEditorSplitter.setHorizontalSplit (false);
    folderBrowserEditorSplitter.setLayout (0, -0.10);

    topAndBottomSplitter.setComponents (&folderBrowserEditorSplitter, &assimil8orValidatorComponent);
    topAndBottomSplitter.setHorizontalSplit (true);
    topAndBottomSplitter.setLayout (2, -0.032);

    addAndMakeVisible (topAndBottomSplitter);
    addAndMakeVisible (toolWindow);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        currentFolder.setText (folderName, juce::NotificationType::dontSendNotification);
    };

    currentFolder.setText (appProperties.getMostRecentFolder (), juce::NotificationType::dontSendNotification);
}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    currentFolder.setBounds (localBounds.removeFromTop (30));
    toolWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
    localBounds.reduce (3, 3);
    topAndBottomSplitter.setBounds (localBounds);
}
