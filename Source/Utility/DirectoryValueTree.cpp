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

void DirectoryValueTree::setScanDepth (int theScanDepth)
{
    scanDepth = theScanDepth;
}

void DirectoryValueTree::cancel ()
{
    cancelScan = true;
    sendStatusUpdate (DirectoryDataProperties::ScanStatus::canceled);
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
    LogDirectoryValueTree (true, "DirectoryValueTree::startScan - enter");
    if (! isScanning ())
    {
        LogDirectoryValueTree (true, "DirectoryValueTree::startScan - waking up scan thread");
        jassert (! rootFolderName.isEmpty ());
        notify ();
    }
    else
    {
        LogDirectoryValueTree (true, "DirectoryValueTree::startScan - still scanning - cancelling and starting timer");
        cancel ();
        startTimer (1);
    }
    LogDirectoryValueTree (true, "DirectoryValueTree::startScan - exit");
}

void DirectoryValueTree::timerCallback ()
{
    LogDirectoryValueTree (true, "DirectoryValueTree::timerCallback - enter");
    stopTimer ();
    startScan ();
    LogDirectoryValueTree (true, "DirectoryValueTree::timerCallback - exit");
}

void DirectoryValueTree::sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus)
{
    juce::MessageManager::callAsync ([this, scanStatus] ()
    {
        //juce::Logger::outputDebugString ("setting scanStatus: " + juce::String(static_cast<int>(scanStatus)));
        directoryDataProperties.setStatus (scanStatus, false);
    });
}

juce::ValueTree DirectoryValueTree::makeFileEntry (juce::File file, DirectoryDataProperties::TypeIndex fileType)
{
    juce::ValueTree fileVT { "File" };
    fileVT.setProperty ("name", file.getFullPathName (), nullptr);
    fileVT.setProperty ("type", static_cast<int>(fileType), nullptr);
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

void DirectoryValueTree::run ()
{
    LogDirectoryValueTree (true, "DirectoryValueTree::run - enter");
    while (wait (-1) && ! threadShouldExit ())
    {
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::scanning);
        scanning = true;
        cancelScan = false;
        LogDirectoryValueTree (true, "DirectoryValueTree::run - rootFolderVT = {}");

        auto newFolderScanVT = doScan ();
        // reset the output if scan was canceled
        if (shouldCancelOperation ())
        {
            LogDirectoryValueTree (true, "DirectoryValueTree::run - threadShouldExit = true, rootFolderVT = {}");
            updateDirectoryData (makeFolderEntry (""));
        }
        else
        {
            updateDirectoryData (newFolderScanVT);
        }
        scanning = false;
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::done);
    }
    LogDirectoryValueTree (true, "DirectoryValueTree::run - exit");
}

juce::ValueTree DirectoryValueTree::doScan ()
{
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
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
                folderVT.addChild (makeFileEntry (curFile, FileTypeHelpers::getFileType (curFile)), -1, nullptr);
        }
    }
    return folderVT;
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
    std::array<SectionInfo, DirectoryDataProperties::TypeIndex::size> sections;
    const auto numFolderEntries { folderVT.getNumChildren () };

    auto insertSorted = [this, &sections, &folderVT] (int itemIndex, int sectionIndex)
    {
        jassert (sectionIndex < DirectoryDataProperties::TypeIndex::size);
        auto& section { sections [sectionIndex] };
        jassert (itemIndex >= section.startIndex + section.length);
        auto startingSectionLength { section.length };
        auto insertItem = [&folderVT, &section, &sections] (int itemIndex, int insertIndex)
        {
            auto tempVT { folderVT.getChild (itemIndex) };
            folderVT.removeChild (itemIndex, nullptr);
            folderVT.addChild (tempVT, insertIndex, nullptr);
            ++section.length;
            for (auto curSectionIndex { 1 }; curSectionIndex < DirectoryDataProperties::TypeIndex::size; ++curSectionIndex)
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
            insertSorted (folderIndex, DirectoryDataProperties::TypeIndex::folder);
            sortContentsOfFolder (folderEntryVT);
        }
        else if (isFileEntry (folderEntryVT))
        {
            // alphabetize in these lists
            //   System files
            //   Preset files
            //   Audio files
            //   unknown files
            switch (static_cast<int> (folderEntryVT.getProperty("type")))
            {
                case DirectoryDataProperties::TypeIndex::systemFile:  insertSorted (folderIndex, DirectoryDataProperties::TypeIndex::systemFile); break;
                case DirectoryDataProperties::TypeIndex::presetFile:  insertSorted (folderIndex, DirectoryDataProperties::TypeIndex::presetFile); break;
                case DirectoryDataProperties::TypeIndex::audioFile:   insertSorted (folderIndex, DirectoryDataProperties::TypeIndex::audioFile); break;
                case DirectoryDataProperties::TypeIndex::unknownFile:
                default:                                              insertSorted (folderIndex, DirectoryDataProperties::TypeIndex::unknownFile); break;
            }
        }
        else
        {
            jassertfalse;
        }
    }
}
