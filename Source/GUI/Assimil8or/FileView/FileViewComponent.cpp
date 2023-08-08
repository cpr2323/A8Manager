#include "FileViewComponent.h"
#include "../../../Utility/PersistentRootProperties.h"

FileViewComponent::FileViewComponent ()
{
    navigateUpButton.setButtonText ("..");
    navigateUpButton.onClick = [this] ()
    {
        appProperties.setMostRecentFolder (juce::File (directoryValueTree.getRootFolder ()).getParentDirectory ().getFullPathName ());
    };
    addAndMakeVisible (navigateUpButton);
    openFolderButton.setButtonText ("Open Folder");
    openFolderButton.onClick = [this] () { openFolder (); };
    addAndMakeVisible (openFolderButton);
    addAndMakeVisible (directoryContentsListBox);

    directoryValueTree.setScanDepth (0); // no depth, only scan root folder
    directoryValueTree.onComplete = [this] ()
    {
        buildQuickLookupList ();
        directoryContentsListBox.updateContent ();
        directoryContentsListBox.scrollToEnsureRowIsOnscreen (0);
        directoryContentsListBox.repaint ();
    };
    directoryValueTree.onStatusChange = [this] (juce::String operation, juce::String fileName)
    {
        //validatorProperties.setProgressUpdate (operation + ": " + fileName, false);
    };
}

void FileViewComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startFolderScan (juce::File (folderName));
    };
    startFolderScan (appProperties.getMostRecentFolder ());
}

void FileViewComponent::startFolderScan (juce::File folderToScan)
{
    // TODO - need to cancel any current scan and start
    directoryListQuickLookupList.clear ();
    directoryValueTree.setRootFolder (folderToScan.getFullPathName ());
    directoryValueTree.startAsyncScan ();
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
    return static_cast<int> (directoryListQuickLookupList.size ());
}

void FileViewComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (row >= getNumRows ())
        return;

    g.setColour (juce::Colours::darkslategrey);
    g.fillRect (width - 1, 0, 1, height);

    g.setColour (juce::Colours::whitesmoke);
    auto file { juce::File (directoryListQuickLookupList[row].getProperty ("name").toString ())};
    juce::String filePrefix;
    if (file.isDirectory ())
        filePrefix = "> ";
    else
        filePrefix = "  ";
    g.drawText (" " + filePrefix + file.getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
}

juce::String FileViewComponent::getTooltipForRow (int row)
{
    if (row >= getNumRows ())
        return {};

    return juce::File (directoryListQuickLookupList [row].getProperty ("name").toString ()).getFileName ();
}

void FileViewComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (row >= getNumRows ())
        return;

    if (auto file { juce::File (directoryListQuickLookupList [row].getProperty ("name").toString ()) }; file.isDirectory ())
        appProperties.setMostRecentFolder (file.getFullPathName ());
}

void FileViewComponent::resized ()
{
    auto localBounds { getLocalBounds () };

    localBounds.reduce (3, 3);
    auto navigationRow { localBounds.removeFromTop (25) };
    navigateUpButton.setBounds (navigationRow.removeFromLeft (25));
    navigationRow.removeFromLeft (5);
    openFolderButton.setBounds (navigationRow.removeFromLeft (100));
    localBounds.removeFromTop (3);
    directoryContentsListBox.setBounds (localBounds);
}

