#include "ToolWindow.h"
#include "../Assimil8or/Assimil8or.h"

ToolWindow::ToolWindow ()
{
    fileMenuButton.setButtonText ("Files");
    fileMenuButton.onClick = [this] ()
    {
        juce::PopupMenu pm;
        pm.addItem ("Verify File", true, false, [this] () { verifyFileUi (); });
        pm.addItem ("Verify Folders", true, false, [this] () { verifyFoldersUi (); });
        pm.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (fileMenuButton);

}

void ToolWindow::init (juce::ValueTree rootPropertiesVT)
{
    persistentRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
}

void ToolWindow::verifyFileUi ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the Assimil8or Preset file you want to verify...",
                                              appProperties.getMostRecentFolder (), "*.yml"));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
                verifyFile (fc.getURLResults () [0].getLocalFile ());
        }, nullptr);
}

void ToolWindow::verifyFile (juce::File presetFile)
{
    juce::StringArray fileContents;
    presetFile.readLines (fileContents);

    Assimil8orPresets assimil8orPresets;
    assimil8orPresets.parse (fileContents);
}

void ToolWindow::verifyFoldersUi ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the Assimil8or Preset file you want to verify...",
                                              appProperties.getMostRecentFolder (), ""));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
                verifyFolders (fc.getURLResults () [0].getLocalFile (), true);
        }, nullptr);
}

void ToolWindow::verifyFolders (juce::File folder, bool verifySubFolders)
{
    //juce::Logger::outputDebugString (folder.getFullPathName ());

    tst.startThread ();
    directoryContentsList.clear ();
    directoryContentsList.setDirectory (folder, verifySubFolders, true);
    directoryContentsList.refresh ();
    startTimer (2);
}

void ToolWindow::timerCallback ()
{
    if (directoryContentsList.isStillLoading ())
        return;
    stopTimer ();
    tst.stopThread (100);

    for (auto curEntryIndex { 0 }; curEntryIndex < directoryContentsList.getNumFiles (); ++curEntryIndex)
    {
        auto entry { directoryContentsList.getFile (curEntryIndex) };
        if (entry.isDirectory ())
        {
            foldersToScan.emplace_back (entry);
        }
        else if (entry.hasFileExtension ("yml"))
            verifyFile (entry);
    }
    if (! foldersToScan.empty ())
    {
        auto nextFolderToScan { foldersToScan.back () };
        foldersToScan.pop_back ();
        verifyFolders (nextFolderToScan, true);
    }
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
