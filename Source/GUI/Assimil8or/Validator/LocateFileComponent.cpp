#include "LocateFileComponent.h"

LocateFileComponent::LocateFileComponent (std::vector<juce::File> theMissingFiles, juce::File startingFolder,
                                          std::function<void (std::vector<std::tuple <juce::File, juce::File>>)> theLocatedFilesCallback, std::function<void ()> theCancelCallback)
{
    jassert (theLocatedFilesCallback != nullptr);
    jassert (theCancelCallback != nullptr);

    locatedFilesCallback = theLocatedFilesCallback;
    cancelCallback = theCancelCallback;

    missingFilesLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    missingFilesLabel.setText ("MISSING FILES", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (missingFilesLabel);
    openButton.onClick = [this] ()
    {
        fileChooser.reset (new juce::FileChooser ("Please select the folder to scan as an Assimil8or SD Card...",
                                                    directoryViewerComponent.getCurrentFolder (), ""));
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {

                curFolderLabel.setText (fc.getURLResults () [0].getLocalFile ().getFullPathName (), juce::NotificationType::dontSendNotification);
                directoryViewerComponent.setCurrentFolder (fc.getURLResults () [0].getLocalFile ());
                directoryViewerComponent.startScan ();
            }
        }, nullptr);
    };
    openButton.setButtonText ("Open");
    addAndMakeVisible (openButton);

    directoryViewerComponent.setCurrentFolder (startingFolder);
    directoryViewerComponent.onFolderChange = [this] (juce::File folder) { curFolderLabel.setText (folder.getFullPathName (), juce::NotificationType::dontSendNotification); };
    addAndMakeVisible (directoryViewerComponent);
    missingFileComponent.assignMissingFileList (theMissingFiles);
    addAndMakeVisible (missingFileComponent);

    curFolderLabel.setText (startingFolder.getFullPathName (), juce::NotificationType::dontSendNotification);
    addAndMakeVisible (curFolderLabel);

    lookHereButton.onClick = [this] () { locateFiles (); };
    lookHereButton.setButtonText ("Look Here");
    addAndMakeVisible (lookHereButton);
    cancelButton.onClick = [this] () { cancelCallback (); };
    cancelButton.setButtonText ("Cancel");
    addAndMakeVisible (cancelButton);
    directoryViewerComponent.startScan ();
}

juce::File LocateFileComponent::getCurFolder ()
{
    return curFolderLabel.getText ();
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
    localBounds.removeFromTop (5);

    auto topRow { localBounds.removeFromTop (20) };
    openButton.setBounds (topRow.removeFromLeft (topRow.getWidth () / 2).removeFromLeft (40));
    missingFilesLabel.setBounds (topRow);
    localBounds.removeFromTop (5);
    auto bottomRow { localBounds.removeFromBottom (20) };
    localBounds.removeFromBottom (5);
    bottomRow.removeFromLeft (5);
    lookHereButton.setBounds (bottomRow.removeFromLeft (60));
    bottomRow.removeFromLeft (5);
    cancelButton.setBounds (bottomRow.removeFromLeft (60));
    localBounds.removeFromBottom (5);
    curFolderLabel.setBounds (localBounds.removeFromBottom (20));
    directoryViewerComponent.setBounds (localBounds.removeFromLeft ((localBounds.getWidth () / 2) - 2));
    localBounds.removeFromLeft (4);
    missingFileComponent.setBounds (localBounds);
}
