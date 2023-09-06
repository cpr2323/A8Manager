#include "DirectoryValueTree.h"
#include "../Assimil8or/FileTypeHelpers.h"

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
    return rootFolderVT;
}

void DirectoryValueTree::clear ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::clear - enter");
    rootFolderVT = {};
}

void DirectoryValueTree::cancel ()
{
    cancelScan = true;
}

bool DirectoryValueTree::isScanning ()
{
    return scanning;
}

void DirectoryValueTree::startScan ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::startScan - enter");
    jassert (! scanning);
    notify ();
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::startScan - exit");
}

void DirectoryValueTree::run ()
{
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - enter");
    while (wait (-1) && ! threadShouldExit ())
    {
        scanning = true;
        cancelScan = false;
        LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - rootFolderVT = {}");

        rootFolderVT = {};
        doScan ();
        // reset the output if scan was canceled
        if (threadShouldExit ())
        {
            LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - threadShouldExit = true, rootFolderVT = {}");
            rootFolderVT = {};
        }
        if (onComplete != nullptr)
            onComplete (! threadShouldExit ());
        scanning = false;
    }
    LogDirectoryValueTree (doLogging, "DirectoryValueTree::run - exit");
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

void DirectoryValueTree::doScan ()
{
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootEntry { juce::File (rootFolderName) };
    // do one initial progress update to fill in the first one
    doStatusUpdate ("Reading File System", rootEntry.getFileName ());
    if (! shouldCancelOperation ())
        rootFolderVT = getContentsOfFolder (rootFolderName, 0);

    if (! shouldCancelOperation ())
        sortContentsOfFolder (rootFolderVT);
}
bool DirectoryValueTree::shouldCancelOperation ()
{
    return threadShouldExit () || cancelScan;
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

            doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] ()
            {
                doStatusUpdate ("Reading File System", fileName);
            });
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
        for (auto sectionEntryIndex { section.startIndex }; sectionEntryIndex < section.startIndex + section.length; ++sectionEntryIndex)
        {
            if (folderVT.getChild (itemIndex).getProperty ("name").toString ().toLowerCase () < folderVT.getChild (sectionEntryIndex).getProperty ("name").toString ().toLowerCase ())
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
        doIfProgressTimeElapsed ([this, fileName = folderEntryVT.getProperty ("name").toString ()] ()
        {
            doStatusUpdate ("Sorting File System", fileName);
        });
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
