#include "FileViewComponent.h"
#include "../../Theme/A8ColourIds.h"
#include "../../../SystemServices.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"
#include "oolib/ValueTree/ValueTreeHelpers.h"
#include "oolib/Debug/WatchDogTimer.h"

#define LOG_FILE_VIEW 0
#if LOG_FILE_VIEW
#define LogFileView(text) juce::Logger::outputDebugString (text);
#else
#define LogFileView(text) ;
#endif

const auto kDialogTextEditorName { "foldername" };
constexpr auto kRowHeight { 24 };
constexpr auto kToolGap { 6 };

FileViewComponent::FileViewComponent ()
{
    setOpaque (true);
    addAndMakeVisible (paneHeader);
    optionsButton.setShowsCaret (true);
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
        optionsMenu.showMenuAsync (juce::PopupMenu::Options ().withTargetComponent (&optionsButton), [this] (int) {});
    };
    addAndMakeVisible (optionsButton);
    directoryContentsListBox.setRowHeight (kRowHeight);
    directoryContentsListBox.setOutlineThickness (0);
    addAndMakeVisible (directoryContentsListBox);
    showAllFiles.setClickingTogglesState (true);
    showAllFiles.setToggleState (false, juce::NotificationType::dontSendNotification);
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
    audioFileTypeId = directoryDataProperties.getFileTypeId (FileTypeHelpers::kAudioFileTypeName);
    presetFileTypeId = directoryDataProperties.getFileTypeId (FileTypeHelpers::kPresetFileTypeName);
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
    // this runs on the update from new data thread, so we work from a detached snapshot of the live tree
    const auto rootFolderSnapshotVT { ValueTreeHelpers::getMessageThreadSnapshot (directoryDataProperties.getRootFolderVT ()) };
    if (! rootFolderSnapshotVT.isValid ())
        return;
    ValueTreeHelpers::forEachChild (rootFolderSnapshotVT, [this] (juce::ValueTree child)
    {
        if (showAllFiles.getToggleState ())
        {
            updateDirectoryListQuickLookupList->emplace_back (child);
        }
        else if (FolderProperties::isFolderVT (child) ||
                 static_cast<int> (child.getProperty (FileProperties::TypePropertyId)) == audioFileTypeId)
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

void FileViewComponent::selectedRowsChanged (int lastRowSelected)
{
    // The list is for navigating, not choosing: clicking a folder opens it, and
    // nothing in the list stays marked as active afterwards. The ListBox selects a
    // row on every click before telling the model, so the selection is undone here.
    if (lastRowSelected >= 0)
        directoryContentsListBox.deselectAllRows ();
}

void FileViewComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (row >= getNumRows ())
        return;

    // A folder row is marked with a folder, as a place that opens; an audio file
    // with a small waveform, and a preset file with a gear, as the settings it holds.
    // Other files keep the same indent so the names line up.
    auto isFolder { true };
    auto isAudio { false };
    auto isPreset { false };
    auto isLoading { false };
    auto textColourId { static_cast<int> (A8Colours::textDim) };
    juce::String name;
    if (! isRootFolder && row == 0)
    {
        name = "..";
    }
    else
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        isFolder = FolderProperties::isFolderVT (directoryEntryVT);
        const auto fileTypeId { static_cast<int> (directoryEntryVT.getProperty (FileProperties::TypePropertyId)) };
        isAudio = ! isFolder && fileTypeId == audioFileTypeId;
        isPreset = ! isFolder && fileTypeId == presetFileTypeId;
        isLoading = isAudio && curBlinkTime != 0 && doubleClickedRow == row;
        if (! isFolder && ! isPreset)
            textColourId = isAudio ? A8Colours::textSupported : A8Colours::textGhost;
        name = juce::File (directoryEntryVT.getProperty ("name").toString ()).getFileName ();
    }

    const auto hovered { row == rowHover.getRow () };

    // a double clicked sample is flagged while it is sent to the editor
    if (isLoading)
        g.fillAll (findColour (A8Colours::selectedRow));

    auto rowBounds { juce::Rectangle<int> { 0, 0, width, height }.reduced (8, 0) };
    const auto iconArea { rowBounds.removeFromLeft (9).toFloat () };
    rowBounds.removeFromLeft (6);
    if (isFolder)
    {
        g.setColour (findColour (A8Colours::accentDeep));
        A8Paint::folder (g, iconArea.withSizeKeepingCentre (9.0f, 8.0f));
    }
    else if (isAudio)
    {
        g.setColour (findColour (A8Colours::textSupported));
        A8Paint::audioFile (g, iconArea.withSizeKeepingCentre (9.0f, 10.0f));
    }
    else if (isPreset)
    {
        g.setColour (findColour (A8Colours::accentDeep));
        A8Paint::gear (g, iconArea.withSizeKeepingCentre (10.0f, 10.0f));
    }

    if (isLoading)
    {
        const auto loadingText { juce::String ("LOADING") };
        g.setFont (A8Type::statusTag ());
        g.setColour (findColour (A8Colours::accentText));
        const auto loadingWidth { A8Paint::textWidth (A8Type::statusTag (), loadingText) };
        g.drawText (loadingText, rowBounds.removeFromRight (loadingWidth), juce::Justification::centredRight, false);
        rowBounds.removeFromRight (6);
    }

    g.setFont (A8Type::body ());
    g.setColour (findColour (hovered || isLoading ? A8Colours::text : textColourId));
    g.drawText (name, rowBounds, juce::Justification::centredLeft, true);

    if (hovered)
        ListRowHover::paintOutline (g, *this, width, height);
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
        if (static_cast<int> (directoryEntryVT.getProperty (FileProperties::TypePropertyId)) == audioFileTypeId)
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
        return static_cast<int> (directoryEntryVT.getProperty (FileProperties::TypePropertyId));
    };
    auto isEntryAFolder = [this, row] ()
    {
        return FolderProperties::isFolderVT (getDirectoryEntryVT (row));
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

        if (isEntryAFolder ())
        {
            auto directoryEntry { getEntryFile () };
            juce::PopupMenu pm;
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
                                                                                             if (! directoryEntry.deleteFile ())
                                                                                             {
                                                                                                // TODO handle delete error
                                                                                             }
                                                                                         }));
            });
            pm.showMenuAsync ({});
        }
        else if (getEntryType () == audioFileTypeId)
        {
            auto directoryEntry { getEntryFile () };
            juce::PopupMenu pm;
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
            if (getEntryType () == audioFileTypeId)
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

            pm.showMenuAsync ({});
        }
    }
    else
    {
        if (! isUpFolder () && ! isEntryAFolder ())
            return;

        auto completeSelection = [this, row, isUpFolder, getEntryFile] ()
        {
            if (isUpFolder ())
            {
                appProperties.setMostRecentFolder (juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName ());
            }
            else
            {
                auto folder { getEntryFile () };
                appProperties.setMostRecentFolder (folder.getFullPathName ());
            }
        };

        if (overwritePresetOrCancel != nullptr)
        {
            // nothing was marked when the folder was clicked, so there is nothing to put back
            overwritePresetOrCancel (completeSelection, [] () {});
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
    if (FileProperties::isFileVT (directoryEntryVT) && static_cast<int> (directoryEntryVT.getProperty (FileProperties::TypePropertyId)) == audioFileTypeId)
    {
        doubleClickedRow = row;
        curBlinkTime = juce::Time::currentTimeMillis ();
        startTimer (125);
        if (onAudioFileSelected != nullptr)
            onAudioFileSelected (juce::File (directoryEntryVT.getProperty ("name").toString ()));
    }
}

int FileViewComponent::getMinimumWidth () const
{
    const auto toolsWidth { optionsButton.getIdealWidth () + showAllFiles.getIdealWidth () + kToolGap };
    // plus the outline on either side
    return paneHeader.getRequiredWidth (toolsWidth) + 2;
}

void FileViewComponent::resized ()
{
    // inside the pane's outline
    auto localBounds { getLocalBounds ().reduced (1) };
    auto headerBounds { localBounds.removeFromTop (PaneHeader::kHeight) };
    paneHeader.setBounds (headerBounds);

    // the pane tools live in the header strip, right aligned
    auto toolRow { paneHeader.getFreeBounds () + headerBounds.getPosition () };
    auto placeTool = [&toolRow] (ChromeButton& tool)
    {
        tool.setBounds (toolRow.removeFromRight (tool.getIdealWidth ()));
        toolRow.removeFromRight (kToolGap);
    };
    placeTool (showAllFiles);
    placeTool (optionsButton);

    directoryContentsListBox.setBounds (localBounds);
}

void FileViewComponent::paint (juce::Graphics& g)
{
    // opaque, so the background behind the rounded corners is this component's to paint
    g.fillAll (findColour (A8Colours::windowBackground));
    A8Paint::card (g, *this, getLocalBounds ());
}

void FileViewComponent::paintOverChildren (juce::Graphics& g)
{
    if (draggingFilesCount > 0)
    {
        const auto listBounds { directoryContentsListBox.getBounds () };
        g.setColour (findColour (A8Colours::dropOverlay));
        g.fillRect (listBounds);

        // the message sits on the same plate a tooltip does
        const auto messageFont { A8Type::body () };
        const auto plateWidth { std::min (listBounds.getWidth () - 16, A8Paint::textWidth (messageFont, dropMsg) + 24) };
        const auto plateBounds { listBounds.withSizeKeepingCentre (plateWidth, 28).toFloat () };
        A8Paint::messagePlate (g, *this, plateBounds, 3.0f);
        g.setFont (messageFont);
        g.setColour (A8Paint::messageInk (*this, ! supportedFile));
        g.drawFittedText (dropMsg, plateBounds.toNearestInt ().reduced (6, 0), juce::Justification::centred, 2);
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
        if (static_cast<int> (child.getProperty (FileProperties::TypePropertyId)) == audioFileTypeId)
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
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Import Failed", message, {}, nullptr,
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
            if (destinationFile.existsAsFile ())
            {
                errorDialog ("The file '" + destinationFile.getFileName () + "' already exists. Import was skipped.");
                continue;
            }

            auto sampleRate { reader->sampleRate };
            auto numChannels { reader->numChannels };
            auto bitsPerSample { reader->bitsPerSample };

            if (bitsPerSample < 8)
                bitsPerSample = 8;
            else if (bitsPerSample > 24) // the wave writer supports int 8/16/24
                bitsPerSample = 24;
            if (numChannels == 0)
            {
                errorDialog ("The file '" + file.getFileName () + "' contains no audio channels.");
                continue;
            }
            if (numChannels > 2)
                numChannels = 2;
            if (reader->sampleRate > 192000)
            {
                errorDialog ("The sample rate of '" + file.getFileName () + "' exceeds 192 kHz.");
                continue;
            }

            juce::TemporaryFile temporaryDestination (destinationFile);
            auto destinationOutputFileStream { temporaryDestination.getFile ().createOutputStream () };
            if (destinationOutputFileStream == nullptr || destinationOutputFileStream->failedToOpen ())
            {
                errorDialog ("Unable to create a temporary file for '" + destinationFile.getFileName () + "'.");
                continue;
            }

            // JUCE's writer takes ownership through a base-typed unique_ptr reference.
            std::unique_ptr<juce::OutputStream> destinationFileStream { std::move (destinationOutputFileStream) };
            auto writeSucceeded { false };
            juce::WavAudioFormat wavAudioFormat;
            // on success, the writer takes ownership of the output stream, and will delete it when done
            if (auto writer { wavAudioFormat.createWriterFor (destinationFileStream, juce::AudioFormatWriterOptions {}.withSampleRate (sampleRate)
                                                                                                                     .withNumChannels (static_cast<int> (numChannels))
                                                                                                                     .withBitsPerSample (bitsPerSample)) }; writer != nullptr)
            {
                // copy the whole thing
                // TODO - two things
                //   a) this needs to be done in a thread
                //   b) we should locally read into a buffer and then write that, so we can display progress if needed
                writeSucceeded = writer->writeFromAudioReader (*reader.get (), 0, -1);
            }
            else
            {
                //failure to create writer
                errorDialog ("Failure to create a writer for '" + destinationFile.getFileName () + "'.");
                continue;
            }

            if (! writeSucceeded)
            {
                errorDialog ("Failure to write '" + destinationFile.getFileName () + "'.");
                continue;
            }

            if (destinationFile.existsAsFile () || ! temporaryDestination.overwriteTargetFileWithTemporary ())
                errorDialog ("Unable to complete the import of '" + destinationFile.getFileName () + "'.");
        }
        else
        {
            // failure to create reader
            errorDialog ("Failure to read '" + file.getFileName () + "'.");
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
