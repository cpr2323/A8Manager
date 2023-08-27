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
    openFolderButton.setButtonText ("Open Folder");
    openFolderButton.onClick = [this] () { openFolder (); };
    addAndMakeVisible (openFolderButton);
    addAndMakeVisible (directoryContentsListBox);

    directoryValueTree.setScanDepth (0); // no depth, only scan root folder
    directoryValueTree.onComplete = [this] (bool success)
    {
        if (success)
        {
            listOffset = juce::File (directoryValueTree.getRootFolder ()).getParentDirectory () != juce::File (directoryValueTree.getRootFolder ()) ? 1 : 0;
            juce::MessageManager::callAsync ([this] ()
            {
                // the call to buildQuickLookupList does not need to happen on the MM thread, but that protects us from some threading issues
                buildQuickLookupList ();
                directoryContentsListBox.updateContent ();
                directoryContentsListBox.scrollToEnsureRowIsOnscreen (0);
                directoryContentsListBox.repaint ();
            });
        }
    };
    directoryValueTree.onStatusChange = [this] (juce::String operation, juce::String fileName)
    {
        //validatorProperties.setProgressUpdate (operation + ": " + fileName, false);
    };

    startThread ();
}

FileViewComponent::~FileViewComponent ()
{
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
        while (directoryValueTree.isScanning());
        directoryListQuickLookupList.clear ();
        directoryValueTree.setRootFolder (rootFolder.getFullPathName ());
        directoryValueTree.startScan ();
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

void FileViewComponent::buildQuickLookupList ()
{
    ValueTreeHelpers::forEachChild (directoryValueTree.getDirectoryVT (), [this] (juce::ValueTree child)
    {
        directoryListQuickLookupList.emplace_back (child);
        return true;
    });
}

int FileViewComponent::getNumRows ()
{
    return static_cast<int> (directoryListQuickLookupList.size () + listOffset);
}

void FileViewComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (row >= getNumRows ())
        return;

    g.setColour (juce::Colours::darkslategrey);
    g.fillRect (width - 1, 0, 1, height);

    g.setColour (juce::Colours::whitesmoke);
    if (listOffset == 1 && row == 0)
    {
        g.drawText (" >  ..", juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
    else
    {
        auto file { juce::File (directoryListQuickLookupList [row - listOffset].getProperty ("name").toString ()) };
        auto filePrefix = [&file] () -> juce::String
        {
            if (file.isDirectory ())
                return "> ";
            else
                return "   ";
        };
        g.drawText (" " + filePrefix () + file.getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

juce::String FileViewComponent::getTooltipForRow (int row)
{
    if (row >= getNumRows ())
        return {};

    if (listOffset == 1 && row == 0)
        return juce::File (directoryValueTree.getRootFolder ()).getParentDirectory ().getFullPathName ();
    else
        return juce::File (directoryListQuickLookupList [row - listOffset].getProperty ("name").toString ()).getFileName ();
}

void FileViewComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows ())
        return;

    if (listOffset == 1 && row == 0)
        appProperties.setMostRecentFolder (juce::File (directoryValueTree.getRootFolder ()).getParentDirectory ().getFullPathName ());
    else
    if (auto folder { juce::File (directoryListQuickLookupList [row - listOffset].getProperty ("name").toString ()) }; folder.isDirectory ())
        appProperties.setMostRecentFolder (folder.getFullPathName ());
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };

    localBounds.reduce (3, 3);
    auto navigationRow { localBounds.removeFromTop (25) };
    openFolderButton.setBounds (navigationRow.removeFromLeft (100));
    localBounds.removeFromTop (3);
    directoryContentsListBox.setBounds (localBounds);
}
