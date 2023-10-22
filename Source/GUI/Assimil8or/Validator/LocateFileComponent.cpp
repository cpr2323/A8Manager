#include "LocateFileComponent.h"

LocateFileComponent::LocateFileComponent (std::vector<juce::File> theMissingFiles, juce::File startingFolder,
                                          std::function<void (std::vector<std::tuple <juce::File, juce::File>>)> theLocatedFilesCallback, std::function<void ()> theCancelCallback)
{
    jassert (theLocatedFilesCallback != nullptr);
    jassert (theCancelCallback != nullptr);

    locatedFilesCallback = theLocatedFilesCallback;
    cancelCallback = theCancelCallback;

    curFolderLabel.setText (startingFolder.getFullPathName (), juce::NotificationType::dontSendNotification);
    addAndMakeVisible (curFolderLabel);
    directoryViewerComponent.setCurrentFolder (startingFolder);
    directoryViewerComponent.onFolderChange = [this] (juce::File folder) { curFolderLabel.setText (folder.getFullPathName (), juce::NotificationType::dontSendNotification); };
    addAndMakeVisible (directoryViewerComponent);
    missingFileComponent.assignMissingFileList (theMissingFiles);
    addAndMakeVisible (missingFileComponent);

    lookHereButton.onClick = [this] () { locateFiles (); };
    lookHereButton.setButtonText ("Look Here");
    addAndMakeVisible (lookHereButton);
    cancelButton.onClick = [this] () { cancelCallback (); };
    cancelButton.setButtonText ("Cancel");
    addAndMakeVisible (cancelButton);
    directoryViewerComponent.startScan ();
}

void LocateFileComponent::locateFiles ()
{
    const auto sourceDirectory { directoryViewerComponent.getCurrentFolder () };
    auto& missingFileList { missingFileComponent.getMissingFileList () };
    jassert (missingFileList.size () > 0);

    std::vector<std::tuple <juce::File, juce::File>> locatedFiles;
    for (const auto& fileToLocate : missingFileList)
        if (const auto sourceFile { sourceDirectory.getChildFile (fileToLocate.getFileName ()) }; sourceFile.exists ())
            locatedFiles.emplace_back (sourceFile, fileToLocate);

    locatedFilesCallback (locatedFiles);
}

void LocateFileComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
//     g.setColour (juce::Colours::black);
//     g.drawRect (directoryViewerComponent.getBounds ());
//     g.drawRect (missingFileComponent.getBounds ());
}

void LocateFileComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (3, 3);
    curFolderLabel.setBounds (localBounds.removeFromTop(20));
    localBounds.removeFromTop (5);
    auto bottomRow { localBounds.removeFromBottom (20) };
    localBounds.removeFromBottom (5);
    bottomRow.removeFromLeft (5);
    lookHereButton.setBounds (bottomRow.removeFromLeft(60));
    bottomRow.removeFromLeft (5);
    cancelButton.setBounds (bottomRow.removeFromLeft (60));
    directoryViewerComponent.setBounds (localBounds.removeFromLeft ((localBounds.getWidth () / 2) - 2));
    localBounds.removeFromLeft (4);
    missingFileComponent.setBounds (localBounds);
}
