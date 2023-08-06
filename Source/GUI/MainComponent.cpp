#include "MainComponent.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

const auto toolWindowHeight { 30 };

//  +----------------+-------------+-----------------------------------+
//  |Folder Contents | Preset List | Preset Editor                     |
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
    setSize (800, 600);

    assimil8orEditorComponent.init (rootPropertiesVT);
    assimil8orValidatorComponent.init (rootPropertiesVT);
    // presetList.init (rootPropertiesVT);
    toolWindow.init (rootPropertiesVT);

    presetListEditorSplitter.setComponents (&presetList, &assimil8orEditorComponent);
    presetListEditorSplitter.setHorizontalSplit (false);

    folderBrowserEditorSplitter.setComponents (&folderContentsTree, &presetListEditorSplitter);
    folderBrowserEditorSplitter.setHorizontalSplit (false);

    topAndBottomSplitter.setComponents (&folderBrowserEditorSplitter, &assimil8orValidatorComponent);
    topAndBottomSplitter.setHorizontalSplit (true);

    addAndMakeVisible (topAndBottomSplitter);
    addAndMakeVisible (toolWindow);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startFolderScan (juce::File (folderName));
    };
    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorProperties::WrapperType::client, ValidatorProperties::EnableCallbacks::yes);

    startFolderScan (appProperties.getMostRecentFolder ());
}

void MainComponent::startFolderScan (juce::File folderToScan)
{
    juce::Logger::outputDebugString ("MainComponent::startFolderScan : " + folderToScan.getFileName ());
    folderContentsThread.startThread ();
    folderContentsDirectoryList.clear ();
    folderContentsDirectoryList.setDirectory (folderToScan, true, true);
    folderContentsDirectoryList.refresh ();
    startTimer (5);

    validatorProperties.setRootFolder (folderToScan.getFullPathName (), false);
    validatorProperties.startAsyncScan (false);

}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
    g.drawRect (folderContentsTree.getBounds ());
}

void MainComponent::timerCallback ()
{
    if (folderContentsDirectoryList.isStillLoading ())
        return;
    juce::Logger::outputDebugString ("MainComponent::timerCallback : scan complete");
    for (auto directoryIndex { 0 }; directoryIndex < folderContentsDirectoryList.getNumFiles (); ++directoryIndex)
        juce::Logger::outputDebugString (juce::String (directoryIndex + 1) + ": " + folderContentsDirectoryList.getFile (directoryIndex).getFullPathName ());
    stopTimer ();
    folderContentsThread.stopThread (100);
    folderContentsTree.refresh ();
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    toolWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
    topAndBottomSplitter.setBounds (localBounds);
}
