#include "FileViewComponent.h"
#include "../../../SystemServices.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/WatchDogTimer.h"

#define LOG_FILE_VIEW 0
#if LOG_FILE_VIEW
#define LogFileView(text) juce::Logger::outputDebugString (text);
#else
#define LogFileView(text) ;
#endif

const auto kDialogTextEditorName { "foldername" };

FileViewComponent::FileViewComponent ()
{
    optionsButton.setButtonText ("OPTIONS");
    optionsButton.setTooltip ("Folder and File options");
    optionsButton.onClick = [this] ()
    {
        juce::PopupMenu optionsMenu;
        optionsMenu.addItem ("Open Folder", true, false, [this] () { openFolder (); });
        optionsMenu.addItem ("New Folder", true, false, [this] () { newFolder (); });
        optionsMenu.addItem ("Remove Unused Samples", true, false, [this] ()
        {
            juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "REMOVE UNUSED SAMPLES",
                                                "Unused Samples are samples that are not used by any presets in the current preset folder.\r\n\r\nAre you sure you want to delete the unused samples in'" + appProperties.getMostRecentFolder () + "'", "YES", "NO", nullptr,
                                                juce::ModalCallbackFunction::create ([this] (int option)
                                                                                    {
                                                                                        if (option == 0) // no
                                                                                            return;
                                                                                        deleteUnusedSamples ();
                                                                                    }));
        });
        optionsMenu.showMenuAsync ({}, [this] (int) {});
    };
    addAndMakeVisible (optionsButton);
    addAndMakeVisible (directoryContentsListBox);
    showAllFiles.setToggleState (false, juce::NotificationType::dontSendNotification);
    showAllFiles.setButtonText ("Show All");
    showAllFiles.setTooltip ("Show all files, or show just Assimil8or files");
    showAllFiles.onClick = [this] () { updateFromNewDataThread.start (); };
    addAndMakeVisible (showAllFiles);

    updateFromNewDataThread.onThreadLoop = [this] ()
    {
        updateFromNewData ();
        return false;
    };
}

void FileViewComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogFileView ("FileViewComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    audioManager = systemServices.getAudioManager ();

    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onRootScanComplete = [this] ()
    {
        LogFileView ("FileViewComponent/onRootScanComplete");
        isRootFolder = juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory () == juce::File (directoryDataProperties.getRootFolder ());
        updateFromNewDataThread.start ();
    };

//     directoryDataProperties.onStatusChange = [this] (DirectoryDataProperties::ScanStatus status)
//     {
//         switch (status)
//         {
//             case DirectoryDataProperties::ScanStatus::empty:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::scanning:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::canceled:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::done:
//             {
//                 isRootFolder = juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory () == juce::File (directoryDataProperties.getRootFolder ());
//                 updateFromNewDataThread.start ();
//             }
//             break;
//         }
//     };

    updateFromNewDataThread.start ();
}

void FileViewComponent::updateFromNewData ()
{
    LogFileView ("FileViewComponent::updateFromNewData ()");
    WatchdogTimer timer;
    timer.start (10000);
    buildQuickLookupList ();
    juce::MessageManager::callAsync ([this] ()
    {
        directoryContentsListBox.updateContent ();
        directoryContentsListBox.repaint ();
    });
    //juce::Logger::outputDebugString ("FileViewComponent::updateFromNewData () - elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void FileViewComponent::timerCallback ()
{
    const auto elapsedTime { juce::Time::currentTimeMillis () - curBlinkTime };
    if (elapsedTime > 1500)
    {
        doubleClickedRow = -1;
        curBlinkTime = 0;
        stopTimer ();
    }
    repaint ();
}

void FileViewComponent::buildQuickLookupList ()
{
    updateDirectoryListQuickLookupList->clear ();
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this] (juce::ValueTree child)
    {
        const auto typeIndex { static_cast<int> (child.getProperty ("type")) };
        if (showAllFiles.getToggleState ())
        {
            updateDirectoryListQuickLookupList->emplace_back (child);
        }
        else if (typeIndex == DirectoryDataProperties::TypeIndex::audioFile ||
                 typeIndex == DirectoryDataProperties::TypeIndex::folder)
        {
            updateDirectoryListQuickLookupList->emplace_back (child);
        }
        return true;
    });
    juce::ScopedLock sl (directoryListQuickLookupListLock);
    if (curDirectoryListQuickLookupList == &directoryListQuickLookupListA)
    {
        curDirectoryListQuickLookupList = &directoryListQuickLookupListB;
        updateDirectoryListQuickLookupList = &directoryListQuickLookupListA;
    }
    else
    {
        curDirectoryListQuickLookupList = &directoryListQuickLookupListA;
        updateDirectoryListQuickLookupList = &directoryListQuickLookupListB;
    }
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

void FileViewComponent::newFolder ()
{
    newAlertWindow = std::make_unique<juce::AlertWindow> ("NEW FOLDER", "Enter the name for new folder", juce::MessageBoxIconType::NoIcon);
    newAlertWindow->addTextEditor (kDialogTextEditorName, {}, {});
    newAlertWindow->addButton ("CREATE", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
    newAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
    auto* textEdtitor { newAlertWindow->getTextEditor (kDialogTextEditorName) };
    auto* createButton { newAlertWindow->getButton ("CREATE") };
    auto* cancelButton { newAlertWindow->getButton ("CANCEL") };
    textEdtitor->setExplicitFocusOrder (1);
    createButton->setExplicitFocusOrder (2);
    cancelButton->setExplicitFocusOrder (3);
    newAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this] (int option)
    {
        newAlertWindow->exitModalState (option);
        newAlertWindow->setVisible (false);
        if (option == 1) // ok
        {
            auto newFolderName { newAlertWindow->getTextEditorContents (kDialogTextEditorName) };
            auto newFolder { juce::File (appProperties.getMostRecentFolder ()).getChildFile (newFolderName) };
            newFolder.createDirectory ();
            // TODO handle error
        }
        newAlertWindow.reset ();
    }));
}

int FileViewComponent::getNumRows ()
{
    juce::ScopedLock sl (directoryListQuickLookupListLock);
    return static_cast<int> (curDirectoryListQuickLookupList->size () + (isRootFolder ? 0 : 1));
}

juce::ValueTree FileViewComponent::getDirectoryEntryVT (int row)
{
    juce::ScopedLock sl (directoryListQuickLookupListLock);
    const auto quickLookupIndex { row - (isRootFolder ? 0 : 1) };
    return (*curDirectoryListQuickLookupList) [quickLookupIndex];
}

void FileViewComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (row >= getNumRows ())
        return;

    if (rowIsSelected)
        lastSelectedRow = row;

    juce::Colour textColor { juce::Colours::whitesmoke };
    juce::String fileListItem;
    if (! isRootFolder && row == 0)
    {
        fileListItem = " >  ..";
    }
    else
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        juce::String filePrefix;
        if (directoryEntryVT.getType ().toString () == "Folder")
        {
            filePrefix = "> ";
        }
        else if (static_cast<int> (directoryEntryVT.getProperty ("type")) == DirectoryDataProperties::TypeIndex::audioFile)
        {
            filePrefix = "-  ";
            textColor = juce::Colours::forestgreen;
            if (curBlinkTime != 0 && doubleClickedRow == row)
            {
                filePrefix += "Loading ";
                textColor = textColor.brighter (0.7f);
            }
        }
        else
        {
            filePrefix = "   ";
            textColor = textColor.darker (0.4f);
        }
        auto file { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
        fileListItem = " " + filePrefix + file.getFileName ();
    }

    g.setColour (juce::Colours::darkslategrey);
    g.fillRect (width - 1, 0, 1, height);
    g.setColour (textColor);
    g.drawText (fileListItem, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
}

juce::String FileViewComponent::getTooltipForRow (int row)
{
    if (row >= getNumRows ())
        return {};

    if (! isRootFolder && row == 0)
        return juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName ();
    else
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };

        juce::String toolTip { juce::File (directoryEntryVT.getProperty ("name").toString ()).getFileName () };
        if (static_cast<int> (directoryEntryVT.getProperty ("type")) == DirectoryDataProperties::TypeIndex::audioFile)
        {
            const auto sampleRate { static_cast<int> (directoryEntryVT.getProperty ("sampleRate")) };
            if (auto errorString { directoryEntryVT.getProperty ("error").toString () }; errorString != "")
                toolTip += juce::String ("\r") + "Error: " + errorString;
            toolTip += juce::String ("\r") + "DataType: " + directoryEntryVT.getProperty ("dataType").toString ();
            toolTip += juce::String ("\r") + "BitDepth: " + juce::String (static_cast<int> (directoryEntryVT.getProperty ("bitDepth")));
            toolTip += juce::String ("\r") + "Channels: " + juce::String (static_cast<int> (directoryEntryVT.getProperty ("numChannels")));
            toolTip += juce::String ("\r") + "SampleRate: " + juce::String (sampleRate);
            toolTip += juce::String ("\r") + "Length: " + juce::String (static_cast<double> (static_cast<juce::int64> (directoryEntryVT.getProperty ("lengthSamples"))) / sampleRate, 2);
        }
        return toolTip;
    }
}

void FileViewComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows ())
        return;

    auto getEntryType = [this, row] ()
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        return static_cast<int> (directoryEntryVT.getProperty ("type"));
    };
    auto getEntryFile = [this, row] ()
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        return juce::File (directoryEntryVT.getProperty ("name").toString ());
    };

    auto isUpFolder = [this, row] ()
    {
        return ! isRootFolder && row == 0;
    };

    if (me.mods.isPopupMenu ())
    {
        if (isUpFolder ())
            return;

        const auto entryType { getEntryType () };
        if (entryType == DirectoryDataProperties::TypeIndex::folder)
        {
            auto directoryEntry { getEntryFile () };
            auto* popupMenuLnF { new juce::LookAndFeel_V4 };
            popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
            juce::PopupMenu pm;
            pm.setLookAndFeel (popupMenuLnF);
            pm.addSectionHeader (directoryEntry.getFileName ());
            pm.addSeparator ();
            pm.addItem ("Rename", true, false, [this, directoryEntry] ()
            {
                renameAlertWindow = std::make_unique<juce::AlertWindow> ("RENAME FOLDER", "Enter the new name for '" + directoryEntry.getFileName () + "'", juce::MessageBoxIconType::NoIcon);
                renameAlertWindow->addTextEditor (kDialogTextEditorName, directoryEntry.getFileName (), {});
                renameAlertWindow->addButton ("RENAME", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
                renameAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
                auto* textEdtitor { renameAlertWindow->getTextEditor (kDialogTextEditorName) };
                auto* createButton { renameAlertWindow->getButton ("RENAME") };
                auto* cancelButton { renameAlertWindow->getButton ("CANCEL") };
                textEdtitor->setExplicitFocusOrder (1);
                createButton->setExplicitFocusOrder (2);
                cancelButton->setExplicitFocusOrder (3);

                renameAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, directoryEntry] (int option)
                                                                                                {
                                                                                                    renameAlertWindow->exitModalState (option);
                                                                                                    renameAlertWindow->setVisible (false);
                                                                                                    if (option == 1) // ok
                                                                                                    {
                                                                                                        auto newFolderName { renameAlertWindow->getTextEditorContents (kDialogTextEditorName) };
                                                                                                        directoryEntry.moveFileTo (directoryEntry.getParentDirectory ().getChildFile (newFolderName));
                                                                                                        // TODO handle error
                                                                                                    }
                                                                                                    renameAlertWindow.reset ();
                                                                                                }));
            });
            pm.addItem ("Delete", true, false, [this, directoryEntry] ()
            {
                juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE FOLDER",
                                                    "Are you sure you want to delete the folder '" + directoryEntry.getFileName () + "'", "YES", "NO", nullptr,
                                                    juce::ModalCallbackFunction::create ([this, directoryEntry] (int option)
                                                                                         {
                                                                                             if (option == 0) // no
                                                                                                 return;
                                                                                             if (!directoryEntry.deleteFile ())
                                                                                             {
                                                                                                // TODO handle delete error
                                                                                             }
                                                                                         }));
            });
            pm.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
        }
        else if (entryType == DirectoryDataProperties::TypeIndex::audioFile)
        {
            auto directoryEntry { getEntryFile () };
            auto* popupMenuLnF { new juce::LookAndFeel_V4 };
            popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
            juce::PopupMenu pm;
            pm.setLookAndFeel (popupMenuLnF);
            pm.addSectionHeader (directoryEntry.getFileName ());
            pm.addSeparator ();
            pm.addItem ("Rename", true, false, [this, directoryEntry] ()
            {
                renameAlertWindow = std::make_unique<juce::AlertWindow> ("RENAME FOLDER", "Enter the new name for '" + directoryEntry.getFileName () + "'", juce::MessageBoxIconType::NoIcon);
                renameAlertWindow->addTextEditor (kDialogTextEditorName, directoryEntry.getFileName (), {});
                renameAlertWindow->addButton ("RENAME", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
                renameAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
                auto* textEdtitor { renameAlertWindow->getTextEditor (kDialogTextEditorName) };
                auto* createButton { renameAlertWindow->getButton ("RENAME") };
                auto* cancelButton { renameAlertWindow->getButton ("CANCEL") };
                textEdtitor->setExplicitFocusOrder (1);
                createButton->setExplicitFocusOrder (2);
                cancelButton->setExplicitFocusOrder (3);

                renameAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, directoryEntry] (int option)
                                                                                                {
                                                                                                    renameAlertWindow->exitModalState (option);
                                                                                                    renameAlertWindow->setVisible (false);
                                                                                                    if (option == 1) // ok
                                                                                                    {
                                                                                                        auto newFolderName { renameAlertWindow->getTextEditorContents (kDialogTextEditorName) };
                                                                                                        directoryEntry.moveFileTo (directoryEntry.getParentDirectory ().getChildFile (newFolderName));
                                                                                                        // TODO handle error
                                                                                                    }
                                                                                                    renameAlertWindow.reset ();
                                                                                                }));
            });
            pm.addItem ("Delete", true, false, [this, directoryEntry] ()
            {
                juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE FILE",
                                                    "Are you sure you want to delete the file '" + directoryEntry.getFileName () + "'", "YES", "NO", nullptr,
                                                    juce::ModalCallbackFunction::create ([this, directoryEntry] (int option)
                                                                                            {
                                                                                                if (option == 0) // no
                                                                                                    return;
                                                                                                if (! directoryEntry.deleteFile ())
                                                                                                {
                                                                                                // TODO handle delete error
                                                                                                }
                                                                                            }));
            });
            if (getEntryType () == DirectoryDataProperties::TypeIndex::audioFile)
            {
                const auto directoryEntryVT { getDirectoryEntryVT (row) };
                if (static_cast<int> (directoryEntryVT.getProperty ("numChannels")) == 2)
                {
                    juce::PopupMenu stereoConvertMenu;
                    stereoConvertMenu.addItem ("To Mono", true, false, [this, directoryEntry] ()
                    {
                        // mix to mono
                        audioManager->mixStereoToMono (directoryEntry);
                    });
                    stereoConvertMenu.addItem ("Split", true, false, [this, directoryEntry] ()
                    {
                        // split into two mono L/R files
                        audioManager->splitStereoIntoTwoMono (directoryEntry);
                    });
                    pm.addSubMenu ("Stereo Convert", stereoConvertMenu, true);
                }
            }

            pm.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
        }
    }
    else
    {
        if (! isUpFolder () && getEntryType () != DirectoryDataProperties::TypeIndex::folder)
            return;

        auto completeSelection = [this, row, isUpFolder, getEntryFile] ()
        {
            if (isUpFolder ())
            {
                appProperties.setMostRecentFolder (juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName ());
            }
            else
            {
                auto folder { getEntryFile ()};
                appProperties.setMostRecentFolder (folder.getFullPathName ());
            }
        };

        if (overwritePresetOrCancel != nullptr)
        {
            auto cancelSelection = [this] ()
            {
                directoryContentsListBox.selectRow (lastSelectedRow, false, true);
            };

            overwritePresetOrCancel (completeSelection, cancelSelection);
        }
        else
        {
            completeSelection ();
        }
    }
}

void FileViewComponent::listBoxItemDoubleClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows () || (! isRootFolder && row == 0))
        return;

    const auto directoryEntryVT { getDirectoryEntryVT (row) };
    if (directoryEntryVT.getType ().toString () == "File" && static_cast<int> (directoryEntryVT.getProperty ("type")) == DirectoryDataProperties::TypeIndex::audioFile)
    {
        doubleClickedRow = row;
        curBlinkTime = juce::Time::currentTimeMillis ();
        startTimer (125);
        if (onAudioFileSelected != nullptr)
            onAudioFileSelected (juce::File (directoryEntryVT.getProperty ("name").toString ()));
    }
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (3, 3);
    auto toolRow { localBounds.removeFromTop (25) };
    optionsButton.setBounds (toolRow.removeFromLeft (70));
    toolRow.removeFromLeft (5);
    showAllFiles.setBounds (toolRow);

    localBounds.removeFromTop (3);
    directoryContentsListBox.setBounds (localBounds);
}

void FileViewComponent::paintOverChildren (juce::Graphics& g)
{
    if (draggingFilesCount > 0)
    {
        auto localBounds { getLocalBounds () };
        juce::Colour fillColor { juce::Colours::white };
        float activeAlpha { 0.7f };
        g.setColour (fillColor.withAlpha (activeAlpha));
        g.fillRect (directoryContentsListBox.getBounds ());
        g.setColour (supportedFile ? juce::Colours::black : juce::Colours::red);
        localBounds.reduce (5, 0);
        g.drawFittedText (dropMsg, localBounds, juce::Justification::centred, 10);
    }
}

void FileViewComponent::deleteUnusedSamples ()
{
    // build list of files in the preset files
    std::set<juce::File> samplesInPresets;
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, &samplesInPresets] (juce::ValueTree child)
    {
        const auto name { child.getProperty ("name").toString () };
        auto file { juce::File (name) };
        if (FileTypeHelpers::isPresetFile (file))
        {
            const auto presetNumber { FileTypeHelpers::getPresetNumberFromName (file) };
            if (presetNumber < 1 || presetNumber > 199 || file.getFileNameWithoutExtension ().startsWith (FileTypeHelpers::kPresetFileNamePrefix) == false)
                return true;

            juce::StringArray fileContents;
            file.readLines (fileContents);
            Assimil8orPreset assimil8orPreset;
            assimil8orPreset.parse (fileContents);

            // TODO - how should we handle errors in the preset file
            if (auto presetErrorList { assimil8orPreset.getParseErrorsVT () }; presetErrorList.getNumChildren () > 0)
            {
                ValueTreeHelpers::forEachChildOfType (presetErrorList, "ParseError", [this] (juce::ValueTree childVT)
                {
                    const auto parseErrorType { childVT.getProperty ("type").toString () };
                    const auto parseErrorDescription { childVT.getProperty ("description").toString () };
                    return true;
                });
            }
            PresetProperties presetProperties (assimil8orPreset.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
            if (presetProperties.isValid ())
            {
                presetProperties.forEachChannel ([this, &samplesInPresets, &file] (juce::ValueTree channelVT, int)
                {
                    ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    channelProperties.forEachZone ([this, &samplesInPresets, &file] (juce::ValueTree zoneVT, int)
                    {
                        ZoneProperties zoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                        const auto sampleFileName { zoneProperties.getSample () };
                        if (sampleFileName.isEmpty ())
                            return true;
                        const auto sampleFile { file.getParentDirectory ().getChildFile (sampleFileName) };
                        samplesInPresets.emplace (sampleFile);
                        return true;
                    });
                    return true;
                });
            }
            else
            {
                // TODO - how should we handle errors in the preset file
            }
        }
        return true;
    });

    // iterate over sample files, and remove any that aren't in the list
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, &samplesInPresets] (juce::ValueTree child)
    {
        const auto name { child.getProperty ("name").toString () };
        auto file { juce::File (name) };
        if (FileTypeHelpers::getFileType (file) == DirectoryDataProperties::audioFile)
        {
            if (samplesInPresets.find (file) == samplesInPresets.end ())
                file.moveToTrash ();
        }
        return true;
    });
}

void FileViewComponent::updateDropInfo (const juce::StringArray& files)
{
    supportedFile = true;
    for (auto& fileName : files)
    {
        auto draggedFile { juce::File (fileName) };
        if (! audioManager->isA8ManagerSupportedAudioFile (draggedFile))
            supportedFile = false;
    }
    if (supportedFile)
    {
        dropMsg = juce::String (draggingFilesCount) + " files to copy";
    }
    else
    {
        dropMsg = (draggingFilesCount == 1 ? "Unsupported file type" : "One, or more, unsupported file types");
    }
}

void FileViewComponent::resetDropInfo ()
{
    draggingFilesCount = 0;
    dropMsg = {};
}

void FileViewComponent::importSamples (const juce::StringArray& files)
{
    auto errorDialog = [this] (juce::String message)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Conversion Failed", message, {}, nullptr,
                                                juce::ModalCallbackFunction::create ([this] (int) {}));
    };

    for (auto& fileName : files)
    {
        auto file { juce::File { fileName } };
        // skip files in the preset folder
        if (file.getParentDirectory () == appProperties.getMostRecentFolder ())
            continue;
        if (auto reader { audioManager->getReaderFor (file) }; reader != nullptr)
        {
            auto destinationFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (file.getFileNameWithoutExtension ()).withFileExtension ("wav") };
            auto destinationFileStream { std::make_unique<juce::FileOutputStream> (destinationFile) };
            destinationFileStream->setPosition (0);
            destinationFileStream->truncate ();

            auto sampleRate { reader->sampleRate };
            auto numChannels { reader->numChannels };
            auto bitsPerSample { reader->bitsPerSample };

            if (bitsPerSample < 8)
                bitsPerSample = 8;
            else if (bitsPerSample > 24) // the wave writer supports int 8/16/24
                bitsPerSample = 24;
            jassert (numChannels != 0);
            if (numChannels > 2)
                numChannels = 2;
            if (reader->sampleRate > 192000)
            {
                // we need to do sample rate conversion
                jassertfalse;
            }

            juce::WavAudioFormat wavAudioFormat;
            if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (destinationFileStream.get (),
                                                                  sampleRate, numChannels, bitsPerSample, {}, 0) }; writer != nullptr)
            {
                // audioFormatWriter will delete the file stream when done
                destinationFileStream.release ();

                // copy the whole thing
                // TODO - two things
                //   a) this needs to be done in a thread
                //   b) we should locally read into a buffer and then write that, so we can display progress if needed
                if (! writer->writeFromAudioReader (*reader.get (), 0, -1) == true)
                {
                    // failure to convert
                    errorDialog ("Failure to write new file");
                    jassertfalse;
                }
            }
            else
            {
                //failure to create writer
                errorDialog ("Failure to create writer");
                jassertfalse;
            }
        }
        else
        {
            // failure to create reader
            errorDialog ("Failure to create reader");
            jassertfalse;
        }
    }
}


bool FileViewComponent::isInterestedInFileDrag ([[maybe_unused]] const juce::StringArray& files)
{
    // we do this check in the fileDragEnter and fileDragMove handlers, presenting more info regarding the drop operation
    return true;
}

void FileViewComponent::filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/)
{

    if (supportedFile)
        importSamples (files);
    resetDropInfo ();
    repaint ();
}

void FileViewComponent::fileDragEnter (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    draggingFilesCount = files.size ();
    updateDropInfo (files);
    repaint ();
}

void FileViewComponent::fileDragMove (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    updateDropInfo (files);
    repaint ();
}

void FileViewComponent::fileDragExit (const juce::StringArray&)
{
    resetDropInfo ();
    repaint ();
}
