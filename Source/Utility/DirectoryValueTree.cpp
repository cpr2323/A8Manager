#include "DirectoryValueTree.h"
#include "../Assimil8or/FileTypeHelpers.h"
#include "../Utility/RuntimeRootProperties.h"

#define LOG_DIRECTORY_VALUE_TREE 1
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

    //ddpMonitor.assign (directoryDataProperties.getValueTreeRef ());

    directoryDataProperties.onRootFolderChange = [this] (juce::String rootFolder) { setRootFolder (rootFolder); };
    directoryDataProperties.onScanDepthChange = [this] (int scanDepth) { setScanDepth (scanDepth); };
    directoryDataProperties.onStartScanChange = [this] () { startScan (); };

    // attach the actual DirectoryValueTree data to the container
    updateDirectoryData (makeFolderEntry (""));
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
    updateDirectoryData (makeFolderEntry (""));
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
        //juce::Logger::outputDebugString ("setting scanStatus: " + juce::String(static_cast<int>(scanStatus)));
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
            updateDirectoryData (makeFolderEntry (""));
        }
        else
        {
            updateDirectoryData (newFolderScanVT);
        }
        scanning = false;
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::done);
    }
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - exit");
}

juce::ValueTree DirectoryValueTree::doScan ()
{
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootEntry { juce::File (rootFolderName) };
    // do one initial progress update to fill in the first one
    // TODO - do we want to route this message through DirectoryDataProperties
    //doStatusUpdate ("Reading File System", rootEntry.getFileName ());
    timer.start (100000);
    auto newDirectoryListVT { getContentsOfFolder (rootFolderName, 0) };
    juce::Logger::outputDebugString ("DirectoryValueTree::doScan - getContentOfFolder - elapsed time: " + juce::String (timer.getElapsedTime ()));

    timer.start (100000);
    if (! shouldCancelOperation ())
        sortContentsOfFolder (newDirectoryListVT);
    juce::Logger::outputDebugString ("DirectoryValueTree::doScan - sortContentOfFolder - elapsed time: " + juce::String (timer.getElapsedTime ()));

    return newDirectoryListVT;
}

juce::ValueTree DirectoryValueTree::getContentsOfFolder (juce::File folder, int curDepth)
{
    auto folderVT { makeFolderEntry (folder) };
    if (scanDepth == -1 || curDepth <= scanDepth)
    {
        for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
        {
            if (shouldCancelOperation ())
                break;

            // TODO - do we want to route this message through DirectoryDataProperties
            //doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] () { doStatusUpdate ("Reading File System", fileName); });
            if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
                folderVT.addChild (getContentsOfFolder (curFile, curDepth + 1), -1, nullptr);
            else
                folderVT.addChild (makeFileEntry (curFile), -1, nullptr);
        }
    }
    return folderVT;
}

juce::ValueTree DirectoryValueTree::makeFileEntry (juce::File file)
{
    juce::ValueTree fileVT { "File" };
    fileVT.setProperty ("name", file.getFullPathName (), nullptr);
    return fileVT;
}

bool DirectoryValueTree::isFolderEntry (juce::ValueTree folderVT)
{
    return folderVT.getType ().toString () == "Folder";
}

bool DirectoryValueTree::isFileEntry (juce::ValueTree fileVT)
{
    return fileVT.getType ().toString () == "File";
}

juce::String DirectoryValueTree::getEntryName (juce::ValueTree fileVT)
{
    return fileVT.getProperty ("name").toString ();
}

juce::ValueTree DirectoryValueTree::makeFolderEntry (juce::String filePath)
{
    juce::ValueTree fileVT { "Folder" };
    fileVT.setProperty ("name", filePath, nullptr);
    return fileVT;
}

juce::ValueTree DirectoryValueTree::makeFolderEntry (juce::File folder)
{
    return makeFolderEntry (folder.getFullPathName ());
}

void DirectoryValueTree::sortContentsOfFolder (juce::ValueTree folderVT)
{
    jassert (isFolderEntry (folderVT));

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

    auto insertSorted = [this, &sections, &folderVT] (int itemIndex, int sectionIndex)
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
        const auto fileName { getEntryName (folderVT.getChild (itemIndex)).toLowerCase () };
        for (auto sectionEntryIndex { section.startIndex }; sectionEntryIndex < section.startIndex + section.length; ++sectionEntryIndex)
        {
            if (fileName < getEntryName (folderVT.getChild (sectionEntryIndex)).toLowerCase ())
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
        // TODO - do we want to route this message through DirectoryDataProperties
        // doIfProgressTimeElapsed ([this, fileName = folderEntryVT.getProperty ("name").toString ()] () { doStatusUpdate ("Sorting File System", fileName); });
        if (isFolderEntry (folderEntryVT))
        {
            insertSorted (folderIndex, SectionIndex::folders);
            sortContentsOfFolder (folderEntryVT);
        }
        else if (isFileEntry (folderEntryVT))
        {
            // alphabetize in these lists
            //   System files
            //   Preset files
            //   Audio files
            //   unknown files
            auto curFile { juce::File (getEntryName (folderEntryVT)) };
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
