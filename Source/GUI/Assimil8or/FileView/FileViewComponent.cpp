#include "FileViewComponent.h"
#include "../../../Assimil8or/Preset/PresetHelpers.h"
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
    openFolderButton.setButtonText ("Open");
    openFolderButton.onClick = [this] () { openFolder (); };
    addAndMakeVisible (openFolderButton);
    newFolderButton.setButtonText ("New");
    newFolderButton.onClick = [this] () { newFolder (); };
    addAndMakeVisible (newFolderButton);
    addAndMakeVisible (directoryContentsListBox);
    showAllFiles.setToggleState (false, juce::NotificationType::dontSendNotification);
    showAllFiles.setButtonText ("Show All");
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
                 typeIndex == DirectoryDataProperties::TypeIndex::folder ||
                 typeIndex == DirectoryDataProperties::TypeIndex::presetFile ||
                 typeIndex == DirectoryDataProperties::TypeIndex::systemFile)
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

    if (! isRootFolder && row != 0)
    {
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        if (static_cast<int> (directoryEntryVT.getProperty ("type")) != DirectoryDataProperties::TypeIndex::folder)
            return;
    }

    if (me.mods.isPopupMenu ())
    {
        if (! isRootFolder && row == 0)
            return;
        const auto directoryEntryVT { getDirectoryEntryVT (row) };
        auto folder { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
        juce::PopupMenu pm;
        pm.addItem ("Rename", true, false, [this, folder] ()
        {
            renameAlertWindow = std::make_unique<juce::AlertWindow> ("RENAME FOLDER", "Enter the new name for '" + folder.getFileName ()  + "'", juce::MessageBoxIconType::NoIcon);
            renameAlertWindow->addTextEditor (kDialogTextEditorName, folder.getFileName (), {});
            renameAlertWindow->addButton ("RENAME", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
            renameAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
            renameAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, folder] (int option)
            {
                renameAlertWindow->exitModalState (option);
                renameAlertWindow->setVisible (false);
                if (option == 1) // ok
                {
                    auto newFolderName { renameAlertWindow->getTextEditorContents (kDialogTextEditorName)};
                    folder.moveFileTo (folder.getParentDirectory ().getChildFile (newFolderName));
                    // TODO handle error
                }
                renameAlertWindow.reset ();
            }));
        });
        pm.addItem ("Delete", true, false, [this, folder] ()
        {
            juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE FOLDER",
            "Are you sure you want to delete the folder '" + folder.getFileName () + "'", "YES", "NO", nullptr,
            juce::ModalCallbackFunction::create ([this, folder] (int option)
            {
                if (option == 0) // no
                    return;
                if (! folder.deleteFile ())
                {
                    // TODO handle delete error
                }
            }));
        });
        pm.showMenuAsync ({}, [this] (int) {});
    }
    else
    {
        auto completeSelection = [this, row] ()
        {
            if (! isRootFolder && row == 0)
            {
                appProperties.setMostRecentFolder (juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName ());
            }
            else
            {
                const auto directoryEntryVT { getDirectoryEntryVT (row) };
                auto folder { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
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
        if (onAudioFileSelected != nullptr)
            onAudioFileSelected (juce::File (directoryEntryVT.getProperty ("name").toString ()));
    }
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (3, 3);
    auto toolRow { localBounds.removeFromTop (25) };
    openFolderButton.setBounds (toolRow.removeFromLeft (50));
    toolRow.removeFromLeft (5);
    newFolderButton.setBounds (toolRow.removeFromLeft (50));
    showAllFiles.setBounds (toolRow);

    localBounds.removeFromTop (3);
    directoryContentsListBox.setBounds (localBounds);
}
