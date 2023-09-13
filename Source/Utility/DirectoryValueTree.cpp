#include "DirectoryValueTree.h"
#include "../Assimil8or/FileTypeHelpers.h"
#include "../Utility/RuntimeRootProperties.h"

#define LOG_DIRECTORY_VALUE_TREE 0
#if LOG_DIRECTORY_VALUE_TREE
#define LogDirectoryValueTree(cond, text) if (cond) {juce::Logger::outputDebugString (text); }
#else
#define LogDirectoryValueTree(cond, text) ;
#endif

DirectoryValueTree::DirectoryValueTree () : Thread ("DirectoryValueTree")
{
    startThread ();
}

DirectoryValueTree::DirectoryValueTree (juce::String theRootFolderName)
    : Thread ("DirectoryValueTree"), rootFolderName { theRootFolderName }
{
    startThread ();
}

DirectoryValueTree::~DirectoryValueTree ()
{
    stopThread (500);
}

juce::ValueTree DirectoryValueTree::getFolderEntry ()
{
    auto folderEntryVT { juce::ValueTree ("Folder") };
    folderEntryVT.setProperty ("name", "", nullptr);
    return folderEntryVT;
}

void DirectoryValueTree::updateDirectoryData (juce::ValueTree newDirectoryData)
{
    // TODO - hard coding index 0 for now, should give some more thought
    directoryDataProperties.getDirectoryValueTreeContainerVT ().removeChild (0, nullptr);
    directoryDataProperties.getDirectoryValueTreeContainerVT ().addChild (newDirectoryData, -1, nullptr);
}

void DirectoryValueTree::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::owner, DirectoryDataProperties::EnableCallbacks::yes);

    ddpMonitor.assign (directoryDataProperties.getValueTreeRef ());

    directoryDataProperties.onRootFolderChange = [this] (juce::String rootFolder) { setRootFolder (rootFolder); };
    directoryDataProperties.onScanDepthChange= [this] (int scanDepth) { setScanDepth (scanDepth); };
    directoryDataProperties.onStartScanChange= [this] () { startScan (); };

    // attach the actual DirectoryValueTree data to the container
    updateDirectoryData (getFolderEntry ());
}

juce::ValueTree DirectoryValueTree::getDirectoryDataPropertiesVT ()
{
    return directoryDataProperties.getValueTree ();
}

void DirectoryValueTree::setRootFolder (juce::String theRootFolderName)
{
    rootFolderName = theRootFolderName;
}

juce::String DirectoryValueTree::getRootFolder ()
{
    return rootFolderName;
}

void DirectoryValueTree::setScanDepth (int theScanDepth)
{
    scanDepth = theScanDepth;
}

juce::ValueTree DirectoryValueTree::getDirectoryVT ()
{
    return {}; //rootFolderVT;
}

void DirectoryValueTree::clear ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::clear - enter");
    // TODO - we need to tell clients
    updateDirectoryData (getFolderEntry ());
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::clear - exit");
}

void DirectoryValueTree::cancel ()
{
    cancelScan = true;
}

bool DirectoryValueTree::isScanning ()
{
    return scanning;
}

void DirectoryValueTree::doStatusUpdate (juce::String operation, juce::String fileName)
{
    if (onStatusChange != nullptr)
        onStatusChange (operation, fileName);
}

void DirectoryValueTree::doIfProgressTimeElapsed (std::function<void ()> functionToDo)
{
    jassert (functionToDo != nullptr);
    if (juce::Time::currentTimeMillis () - lastScanInProgressUpdate > 250)
    {
        lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
        functionToDo ();
    }
}

bool DirectoryValueTree::shouldCancelOperation ()
{
    return threadShouldExit () || cancelScan;
}

void DirectoryValueTree::startScan ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::startScan - enter");
    if (! isScanning ())
    {
        LogDirectoryValueTree (doLogging, "DirectoryValueTree::startScan - waking up scan thread");
        jassert (! rootFolderName.isEmpty ());
        notify ();
    }
    else
    {
        LogDirectoryValueTree (doLogging, "DirectoryValueTree::startScan - still scanning - cancelling and starting timer");
        cancel ();
        startTimer (1);
    }
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::startScan - exit");
}

void DirectoryValueTree::timerCallback ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::timerCallback - enter");
    stopTimer ();
    startScan ();
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::timerCallback - exit");
}

void DirectoryValueTree::sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus)
{
    juce::MessageManager::callAsync ([this, scanStatus] ()
        {
            juce::Logger::outputDebugString ("setting scanStatus: " + juce::String(static_cast<int>(scanStatus)));

            directoryDataProperties.setStatus (scanStatus, false);
        });
}

void DirectoryValueTree::run ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - enter");
    while (wait (-1) && ! threadShouldExit ())
    {
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::scanning);
        scanning = true;
        cancelScan = false;
        LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - rootFolderVT = {}");

        auto newFolderScanVT = doScan ();
        // reset the output if scan was canceled
        if (shouldCancelOperation ())
        {
            LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - threadShouldExit = true, rootFolderVT = {}");
            updateDirectoryData (getFolderEntry ());
        }
        juce::MessageManager::callAsync ([this, newFolderScanVT, wasCanceled = shouldCancelOperation ()] ()
        {
            if (! wasCanceled)
                updateDirectoryData (newFolderScanVT);
            else
                updateDirectoryData (getFolderEntry ());
            if (onComplete != nullptr)
                onComplete (! wasCanceled);
        });
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::done);
    }
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - exit");
}

juce::ValueTree DirectoryValueTree::doScan ()
{
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootEntry { juce::File (rootFolderName) };
    // do one initial progress update to fill in the first one
    doStatusUpdate ("Reading File System", rootEntry.getFileName ());
    auto newDirectoryListVT { getContentsOfFolder (rootFolderName, 0) };

    if (! shouldCancelOperation ())
        sortContentsOfFolder (newDirectoryListVT);

    return newDirectoryListVT;
}

juce::ValueTree DirectoryValueTree::getContentsOfFolder (juce::File folder, int curDepth)
{
    juce::ValueTree folderVT {"Folder"};
    folderVT.setProperty ("name", folder.getFullPathName (), nullptr);
    if (scanDepth == -1 || curDepth <= scanDepth)
    {
        for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
        {
            if (shouldCancelOperation ())
                break;

            doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] () { doStatusUpdate ("Reading File System", fileName); });
            if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
            {
                folderVT.addChild (getContentsOfFolder (curFile, curDepth + 1), -1, nullptr);
            }
            else
            {
                juce::ValueTree fileVT {"File"};
                fileVT.setProperty ("name", curFile.getFullPathName (), nullptr);
                folderVT.addChild (fileVT, -1, nullptr);
            }
        }
    }
    return folderVT;
}

void DirectoryValueTree::sortContentsOfFolder (juce::ValueTree folderVT)
{
    jassert (folderVT.getType ().toString () == "Folder");

    // Folders
    // System files (folderprefs, lastfolder, lastpreset)
    // Preset files
    // Audio files
    // unknown files
    struct SectionInfo
    {
        int startIndex { 0 };
        int length { 0 };
    };
    enum SectionIndex
    {
        folders,
        systemFiles,
        presetFiles,
        audioFiles,
        unknownFiles,
        size
    };
    std::array<SectionInfo, SectionIndex::size> sections;
    const auto numFolderEntries { folderVT.getNumChildren () };

    auto insertSorted = [&sections, &folderVT] (int itemIndex, int sectionIndex)
    {
        jassert (sectionIndex < SectionIndex::size);
        auto& section { sections [sectionIndex] };
        jassert (itemIndex >= section.startIndex + section.length);
        auto startingSectionLength { section.length };
        auto insertItem = [&folderVT, &section, &sections] (int itemIndex, int insertIndex)
        {
            auto tempVT { folderVT.getChild (itemIndex) };
            folderVT.removeChild (itemIndex, nullptr);
            folderVT.addChild (tempVT, insertIndex, nullptr);
            ++section.length;
            for (auto curSectionIndex { 1 }; curSectionIndex < SectionIndex::size; ++curSectionIndex)
                sections [curSectionIndex].startIndex = sections [curSectionIndex - 1].startIndex + sections [curSectionIndex - 1].length;
        };
        const auto fileName { folderVT.getChild (itemIndex).getProperty ("name").toString ().toLowerCase () };
        for (auto sectionEntryIndex { section.startIndex }; sectionEntryIndex < section.startIndex + section.length; ++sectionEntryIndex)
        {
            if (fileName < folderVT.getChild (sectionEntryIndex).getProperty ("name").toString ().toLowerCase ())
            {
                insertItem (itemIndex, sectionEntryIndex);
                break;
            }
        }
        if (section.length == startingSectionLength)
            insertItem (itemIndex, section.startIndex + section.length);
    };
    for (auto folderIndex { 0 }; folderIndex < numFolderEntries && ! shouldCancelOperation (); ++folderIndex)
    {
        auto folderEntryVT { folderVT.getChild (folderIndex) };
        doIfProgressTimeElapsed ([this, fileName = folderEntryVT.getProperty ("name").toString ()] () { doStatusUpdate ("Sorting File System", fileName); });
        if (folderEntryVT.getType ().toString () == "Folder")
        {
            insertSorted (folderIndex, SectionIndex::folders);
            sortContentsOfFolder (folderEntryVT);
        }
        else if (folderEntryVT.getType ().toString () == "File")
        {
            // alphabetize in these lists
            //   System files
            //   Preset files
            //   Audio files
            //   unknown files
            auto curFile { juce::File (folderEntryVT.getProperty ("name").toString ()) };
            //jassert (curFile.exists ());
            if (FileTypeHelpers::isSystemFile (curFile))
                insertSorted (folderIndex, SectionIndex::systemFiles);
            else if (FileTypeHelpers::isPresetFile (curFile))
                insertSorted (folderIndex, SectionIndex::presetFiles);
            else if (FileTypeHelpers::isAudioFile (curFile))
                insertSorted (folderIndex, SectionIndex::audioFiles);
            else // unknown file
                insertSorted (folderIndex, SectionIndex::unknownFiles);
        }
        else
        {
            jassertfalse;
        }
    }
}
