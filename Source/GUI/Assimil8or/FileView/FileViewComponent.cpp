#include "FileViewComponent.h"
#include "../../../Utility/PersistentRootProperties.h"

FileViewComponent::FileViewComponent ()
{
    navigateUpButton.setButtonText ("..");
    navigateUpButton.onClick = [this] ()
    {
        appProperties.setMostRecentFolder (folderContentsDirectoryList.getDirectory ().getParentDirectory ().getFullPathName ());
    };
    addAndMakeVisible (navigateUpButton);
    openFolderButton.setButtonText ("Open Folder");
    openFolderButton.onClick = [this] () { openFolder (); };
    addAndMakeVisible (openFolderButton);
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

void FileViewComponent::openFolder ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the folder to scan as an Assimil8or SD Card...",
                                               appProperties.getMostRecentFolder (), ""));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            appProperties.setMostRecentFolder (fc.getURLResults () [0].getLocalFile ().getFullPathName ());
    }, nullptr);
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };

    localBounds.reduce (3, 3);
    auto navigationRow { localBounds.removeFromTop (25) };
    navigateUpButton.setBounds (navigationRow.removeFromLeft (25));
    navigationRow.removeFromLeft (5);
    openFolderButton.setBounds (navigationRow.removeFromLeft (100));
    localBounds.removeFromTop (3);
    // place list
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
}
