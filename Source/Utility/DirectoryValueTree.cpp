#include "DirectoryValueTree.h"
#include "../Assimil8or/FileTypeHelpers.h"

DirectoryValueTree::DirectoryValueTree (juce::String theRootFolderName)
    : Thread ("Assimil8orValidator"), rootFolderName { theRootFolderName }
{
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
    rootFolderVT = {};
}

bool DirectoryValueTree::getScanStatus ()
{
    return isThreadRunning ();
}

void DirectoryValueTree::startAsyncScan ()
{
    if (isThreadRunning ())
        return;
    startThread ();
}

void DirectoryValueTree::run ()
{
    rootFolderVT = {};
    doScan ();
    // reset the output if scan was canceled
    if (threadShouldExit ())
        rootFolderVT = {};
    if (onComplete != nullptr)
        juce::MessageManager::callAsync ([this] () { onComplete (); });
}

void DirectoryValueTree::doStatusUpdate (juce::String operation, juce::String fileName)
{
    if (onStatusChange != nullptr)
        juce::MessageManager::callAsync ([this, operation, fileName] () { onStatusChange (operation, fileName); });
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
    juce::MessageManager::callAsync ([this, folderName = rootEntry.getFileName ()] ()
    {
        doStatusUpdate ("Reading File System", rootFolderName);
    });
    if (! threadShouldExit ())
        rootFolderVT = getContentsOfFolder (rootFolderName, 0);
    if (! threadShouldExit ())
        sortContentsOfFolder (rootFolderVT);
}

juce::ValueTree DirectoryValueTree::getContentsOfFolder (juce::File folder, int curDepth)
{
    juce::ValueTree folderVT {"Folder"};
    folderVT.setProperty ("name", folder.getFullPathName (), nullptr);
    if (scanDepth == -1 || curDepth <= scanDepth)
    {
        for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
        {
            if (threadShouldExit ())
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
            if (folderVT.getChild (itemIndex).getProperty ("name").toString () < folderVT.getChild (sectionEntryIndex).getProperty ("name").toString ())
            {
                insertItem (itemIndex, sectionEntryIndex);
                break;
            }
        }
        if (section.length == startingSectionLength)
            insertItem (itemIndex, section.startIndex + section.length);
    };
    for (auto folderIndex { 0 }; folderIndex < numFolderEntries; ++folderIndex)
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
            jassert (curFile.exists ());
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
