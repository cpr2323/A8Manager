#include "FileViewComponent.h"
#include "../../../Assimil8or/Preset/PresetHelpers.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

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
}

FileViewComponent::~FileViewComponent ()
{
}

void FileViewComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onStatusChange = [this] (DirectoryDataProperties::ScanStatus status)
    {
        switch (status)
        {
            case DirectoryDataProperties::ScanStatus::empty:
            {
                juce::Logger::outputDebugString ("ScanStatus::empty");
            }
            break;
            case DirectoryDataProperties::ScanStatus::scanning:
            {
                juce::Logger::outputDebugString ("ScanStatus::scanning");
            }
            break;
            case DirectoryDataProperties::ScanStatus::canceled:
            {
                juce::Logger::outputDebugString ("ScanStatus::canceled");
            }
            break;
            case DirectoryDataProperties::ScanStatus::done:
            {
                juce::Logger::outputDebugString ("ScanStatus::done");
                jassert (juce::MessageManager::getInstance()->isThisTheMessageThread ());
                isRootFolder = juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory () == juce::File (directoryDataProperties.getRootFolder ());
                updateFromData ();
            }
            break;
        }
    };

    updateFromData ();
}

void FileViewComponent::updateFromData ()
{
    // the call to buildQuickLookupList does not need to happen on the MM thread, but that protects us from some threading issues
    buildQuickLookupList ();
    directoryContentsListBox.updateContent ();
    directoryContentsListBox.repaint ();
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

void FileViewComponent::buildQuickLookupList ()
{
    ValueTreeHelpers::forEachChild (directoryDataProperties.getDirectoryValueTreeVT (), [this] (juce::ValueTree child)
    {
        directoryListQuickLookupList.emplace_back (child);
        return true;
    });
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
    return static_cast<int> (directoryListQuickLookupList.size () + (isRootFolder ? 0 : 1));
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
        auto file { juce::File (directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)].getProperty ("name").toString ()) };
        juce::String filePrefix;
        if (file.isDirectory ())
        {
            filePrefix = "> ";
        }
        else if (PresetHelpers::isSupportedAudioFile (file))
        {
            filePrefix = "-  ";
            textColor = juce::Colours::forestgreen;
        }
        else
        {
            filePrefix = "   ";
            textColor = textColor.darker (0.4f);
        }
        fileListItem = " " + filePrefix+ file.getFileName ();
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
        return juce::File (directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)].getProperty ("name").toString ()).getFileName ();
}

void FileViewComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows ())
        return;

    if (! isRootFolder && row != 0)
    {
        auto folder { juce::File (directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)].getProperty ("name").toString ()) };
        if (! folder.isDirectory ())
            return;
    }

    if (me.mods.isPopupMenu ())
    {
        if (! isRootFolder && row == 0)
            return;
        auto folder { juce::File (directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)].getProperty ("name").toString ()) };
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
                folder.deleteFile ();
                // TODO handle delete error
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
                auto folder { juce::File (directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)].getProperty ("name").toString ()) };
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

    auto file { juce::File (directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)].getProperty ("name").toString ()) };
    if (! file.isDirectory () && PresetHelpers::isSupportedAudioFile (file) && onAudioFileSelected != nullptr)
            onAudioFileSelected (file);
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (3, 3);
    auto toolRow { localBounds.removeFromTop (25) };
    openFolderButton.setBounds (toolRow.removeFromLeft (50));
    toolRow.removeFromLeft (5);
    newFolderButton.setBounds (toolRow.removeFromLeft (50));
    localBounds.removeFromTop (3);
    directoryContentsListBox.setBounds (localBounds);
}
