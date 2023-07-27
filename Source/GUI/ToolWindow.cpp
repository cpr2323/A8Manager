#include "ToolWindow.h"
#include "../Utility/RuntimeRootProperties.h"
#include "../Assimil8or/Assimil8or.h"

#define SCAN_ONLY 0

ToolWindow::ToolWindow ()
{
    fileMenuButton.setButtonText ("File");
    fileMenuButton.onClick = [this] ()
    {
        juce::PopupMenu pm;
        pm.addItem ("Load Preset", true, false, [this] () { loadPresetUi (); });
        pm.addItem ("Validate Directory", true, false, [this] () { verifySdCardImage (); });
        pm.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (fileMenuButton);
    toolMenuButton.setButtonText ("Tools");
    toolMenuButton.onClick = [this] ()
    {
        juce::PopupMenu pm;
#if !SCAN_ONLY
        pm.addItem ("Verify File", true, false, [this] () { verifyFileUi (); });
        pm.addItem ("Verify Folders", true, false, [this] () { verifyFoldersUi (); });
#endif
        pm.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (toolMenuButton);
    scanningStatusLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    addAndMakeVisible (scanningStatusLabel);
    progressUpdateLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::darkgreen);
    addAndMakeVisible (progressUpdateLabel);
}

void ToolWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    persistentRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::yes);
    validatorProperties.onScanStatusChanged = [this] (juce::String scanStatus) { updateScanStatus (scanStatus); };
    validatorProperties.onProgressUpdateChanged = [this] (juce::String progressUpdate) { updateProgress (progressUpdate); };
    presetProperties.wrap (runtimeRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
}


void ToolWindow::loadPresetUi ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the Assimil8or Preset file you want to load...",
                                              appProperties.getMostRecentFolder (), "*.yml"));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            loadPreset (fc.getURLResults () [0].getLocalFile ());
    }, nullptr);
}

void ToolWindow::loadPreset (juce::File presetFile)
{
    jassert (presetProperties.isValid ());
    juce::StringArray fileContents;
    presetFile.readLines (fileContents);

    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.parse (fileContents);

    PresetProperties newPresetProperties;
    newPresetProperties.wrap (assimil8orPreset.getPresetVT (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);

    // TODO - we need to deal with the 'defaults' behavior
    //   1. Clear presetProperties
    //   2. Copy only the parameters that exist
    presetProperties.setName (newPresetProperties.getName (), false);
}

void ToolWindow::updateScanStatus (juce::String scanStatus)
{
    if (scanStatus == "scanning")
    {
        scanningStatusLabel.setText ("Scanning file system", juce::NotificationType::dontSendNotification);
    }
    else if (scanStatus == "idle")
    {
        scanningStatusLabel.setText ("", juce::NotificationType::dontSendNotification);
    }
    else
    {
        jassertfalse;
    }
}

void ToolWindow::updateProgress (juce::String progressUpdate)
{
    progressUpdateLabel.setText (progressUpdate, juce::NotificationType::dontSendNotification);
}

void ToolWindow::verifySdCardImage ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the folder to scan as an Assimil8or SD Card...",
                                              appProperties.getMostRecentFolder (), ""));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {
                validatorProperties.setRootFolder (fc.getURLResults () [0].getLocalFile ().getFullPathName (), false);
                validatorProperties.startAsyncScan (false);
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
    localBounds.removeFromLeft (5);
    toolMenuButton.setBounds (localBounds.removeFromLeft (100));
    progressUpdateLabel.setBounds (localBounds.removeFromRight (100));
    scanningStatusLabel.setBounds (localBounds.removeFromRight (100));
}
