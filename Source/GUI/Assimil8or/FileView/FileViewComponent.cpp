#include "FileViewComponent.h"
#include "../../../Utility/PersistentRootProperties.h"

#define LOG_FILE_VIEW 0
#if LOG_FILE_VIEW
#define LogFileView(text) juce::Logger::outputDebugString (text);
#else
#define LogFileView(text) ;
#endif

FileViewComponent::FileViewComponent () : Thread ("FileViewComponentThread")
{
    audioFormatManager.registerBasicFormats ();

    openFolderButton.setButtonText ("Open");
    openFolderButton.onClick = [this] () { openFolder (); };
    addAndMakeVisible (openFolderButton);
    newFolderButton.setButtonText ("New");
    newFolderButton.onClick = [this] () { newFolder (); };
    addAndMakeVisible (newFolderButton);
    addAndMakeVisible (directoryContentsListBox);

    directoryValueTree.setScanDepth (0); // no depth, only scan root folder
    directoryValueTree.onComplete = [this] (bool success)
    {
        if (success)
        {
            isRootFolder = juce::File (directoryValueTree.getRootFolder ()).getParentDirectory () == juce::File (directoryValueTree.getRootFolder ());
            juce::MessageManager::callAsync ([this] ()
            {
                // the call to buildQuickLookupList does not need to happen on the MM thread, but that protects us from some threading issues
                buildQuickLookupList ();
                directoryContentsListBox.updateContent ();
                directoryContentsListBox.repaint ();
            });
        }
    };
    directoryValueTree.onStatusChange = [this] (juce::String operation, juce::String fileName)
    {
        //validatorProperties.setProgressUpdate (operation + ": " + fileName, false);
    };

    startThread ();
    startTimer (333);
}

FileViewComponent::~FileViewComponent ()
{
    stopTimer ();
    stopThread (100);
}

void FileViewComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startScan (juce::File (folderName));
    };
    startScan (appProperties.getMostRecentFolder ());
}

void FileViewComponent::startScan (juce::File folderToScan)
{
    {
        juce::ScopedLock sl (queuedFolderLock);
        queuedFolderToScan = folderToScan;
        directoryValueTree.cancel ();
        LogFileView ("FileViewComponent::startScan: " + queuedFolderToScan.getFullPathName ());
    }
    notify ();
}

void FileViewComponent::run ()
{
    while (wait (-1) && ! threadShouldExit ())
    {
        juce::File rootFolder;
        {
            juce::ScopedLock sl (queuedFolderLock);
            rootFolder = queuedFolderToScan;
            queuedFolderToScan = juce::File ();
            LogFileView ("FileViewComponent::run: " + rootFolder.getFullPathName ());
        }
        // TODO - probably want to do something else to not get deadlocked, ie. track time and try and catch deadlock
        //        maybe request an exit again here
        while (directoryValueTree.isScanning ());
        directoryListQuickLookupList.clear ();
        directoryValueTree.setRootFolder (rootFolder.getFullPathName ());
        directoryValueTree.startScan ();
    }
}

void FileViewComponent::timerCallback ()
{
    startScan (appProperties.getMostRecentFolder ());
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

bool FileViewComponent::isSupportedAudioFile (juce::File file)
{
    if (file.isDirectory () || file.getFileExtension () != ".wav")
        return false;
    std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;
    // check for any format settings that are unsupported
    if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample < 8 || reader->bitsPerSample > 32) || (reader->numChannels == 0 || reader->numChannels > 2) || (reader->sampleRate > 192000))
        return false;

    return true;
}

void FileViewComponent::buildQuickLookupList ()
{
    ValueTreeHelpers::forEachChild (directoryValueTree.getDirectoryVT (), [this] (juce::ValueTree child)
    {
        directoryListQuickLookupList.emplace_back (child);
        return true;
    });
}

void FileViewComponent::newFolder ()
{
    newAlertWindow = std::make_unique<juce::AlertWindow> ("NEW FOLDER", "Enter the name for new folder", juce::MessageBoxIconType::NoIcon);
    newAlertWindow->addTextEditor ("foldername", {}, {});
    newAlertWindow->addButton ("CREATE", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
    newAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
    newAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this] (int option)
    {
        newAlertWindow->exitModalState (option);
        newAlertWindow->setVisible (false);
        if (option == 1) // ok
        {
            auto newFolderName { newAlertWindow->getTextEditorContents ("foldername") };
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
        else if (isSupportedAudioFile (file))
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
        return juce::File (directoryValueTree.getRootFolder ()).getParentDirectory ().getFullPathName ();
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
            renameAlertWindow->addTextEditor ("foldername", folder.getFileName (), {});
            renameAlertWindow->addButton ("RENAME", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
            renameAlertWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
            renameAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, folder] (int option)
            {
                renameAlertWindow->exitModalState (option);
                renameAlertWindow->setVisible (false);
                if (option == 1) // ok
                {
                    auto newFolderName { renameAlertWindow->getTextEditorContents ("foldername")};
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
                appProperties.setMostRecentFolder (juce::File (directoryValueTree.getRootFolder ()).getParentDirectory ().getFullPathName ());
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
    if (! file.isDirectory () && isSupportedAudioFile (file) && onAudioFileSelected != nullptr)
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
