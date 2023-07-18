#include "ToolWindow.h"
#include "../Utility/RuntimeRootProperties.h"

ToolWindow::ToolWindow ()
{
    fileMenuButton.setButtonText ("Files");
    fileMenuButton.onClick = [this] ()
    {
        juce::PopupMenu pm;
        pm.addItem ("Verify SD Card Image", true, false, [this] () { verifySdCardImage (); });
        pm.addItem ("Verify File", true, false, [this] () { verifyFileUi (); });
        pm.addItem ("Verify Folders", true, false, [this] () { verifyFoldersUi (); });
        pm.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (fileMenuButton);
    sdCardImage.addListener (this);
}

void ToolWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    persistentRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    sdCardImage = runtimeRootProperties.getValueTree ().getChildWithName ("SDCardImage");
    jassert (sdCardImage.isValid ());
}

void ToolWindow::verifySdCardImage ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the folder to scan for Assimil8or Preset files you want to verify...",
                                              appProperties.getMostRecentFolder (), ""));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {
                assimil8orSDCardImage.setRootFolder (fc.getURLResults () [0].getLocalFile ());
                sdCardImage.setProperty ("scanStatus", "scanning", nullptr);
                assimil8orSDCardImage.validate ([this] ()
                {
                    juce::Logger::outputDebugString ("validation done");
                    juce::MessageManager::callAsync ([this] () { sdCardImage.setProperty ("scanStatus", "idle", nullptr); });
                });
            }
        }, nullptr);
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

    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.parse (fileContents);
}

void ToolWindow::verifyFoldersUi ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the folder to scan for Assimil8or Preset files you want to verify...",
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

void ToolWindow::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == sdCardImage)
    {
        if (property.toString () == "scanStatus")
        {
            juce::Logger::outputDebugString ("tool window received scanning update - " + sdCardImage.getProperty("scanStatus").toString());
        }
    }
}
