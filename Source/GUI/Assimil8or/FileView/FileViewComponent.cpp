#include "FileViewComponent.h"
#include "../../../Utility/PersistentRootProperties.h"

FileViewComponent::FileViewComponent ()
{
    navigateUpButton.setButtonText ("..");
    addAndMakeVisible (navigateUpButton);
    addAndMakeVisible (fileTreeView);
    treeViewMouseDown.onItemSelected = [this] (int row)
    {
        if (row >= fileTreeView.getNumRowsInTree ())
            return;
        juce::MessageManager::callAsync ([this] ()
        {
            if (fileTreeView.getSelectedFile (0).isDirectory ())
                appProperties.setMostRecentFolder (fileTreeView.getSelectedFile (0).getFullPathName ());
        });
    };
}

void FileViewComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startFolderScan (juce::File (folderName));
    };
    startFolderScan (appProperties.getMostRecentFolder ());
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };

    localBounds.reduce (3, 3);
    navigateUpButton.onClick = [this] ()
    {
        appProperties.setMostRecentFolder (folderContentsDirectoryList.getDirectory ().getParentDirectory ().getFullPathName ());
    };
    navigateUpButton.setBounds (localBounds.removeFromTop (25).removeFromLeft (25));
    localBounds.removeFromTop (3);
    fileTreeView.setBounds (localBounds);
}

void FileViewComponent::startFolderScan (juce::File folderToScan)
{
    folderContentsThread.startThread ();
    folderContentsDirectoryList.clear ();
    folderContentsDirectoryList.setDirectory (folderToScan, true, true);
    folderContentsDirectoryList.refresh ();
    startTimer (5);
}

void FileViewComponent::timerCallback ()
{
    if (folderContentsDirectoryList.isStillLoading ())
        return;
    folderContentsThread.stopThread (100);
    fileTreeView.refresh ();
}
