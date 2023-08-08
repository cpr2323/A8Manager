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

juce::ValueTree DirectoryValueTree::getDirectoryVT ()
{
    return rootFolderVT;
}

void DirectoryValueTree::clear ()
{
    rootFolderVT = {};
}

bool DirectoryValueTree::getReadStatus ()
{
    return isThreadRunning ();
}

void DirectoryValueTree::startAsyncRead ()
{
    if (isThreadRunning ())
        return;
    startThread ();
    if (onReadStatusChange != nullptr)
        onReadStatusChange (true);
}

void DirectoryValueTree::run ()
{
    rootFolderVT = {};
    validateRootFolder ();
    if (threadShouldExit ())
        rootFolderVT = {};
    if (onReadStatusChange != nullptr)
        juce::MessageManager::callAsync ([this] () { onReadStatusChange (false); });
}

void DirectoryValueTree::validateRootFolder ()
{
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootEntry { juce::File (rootFolderName) };
    // do one initial progress update to fill in the first one
    juce::MessageManager::callAsync ([this, folderName = rootEntry.getFileName ()] ()
        {
            // TODO provide progress updates
            // validatorProperties.setProgressUpdate (rootFolderName, false);
        });
    if (! threadShouldExit ())
        rootFolderVT = getContentsOfFolder (rootFolderName);
    if (! threadShouldExit ())
        sortContentsOfFolder (rootFolderVT);

    juce::MessageManager::callAsync ([this] ()
        {
            // TODO provide progress updates
            // validatorProperties.setProgressUpdate ("", false);
            // validatorProperties.setScanStatus ("idle", false);
        });
}

juce::ValueTree DirectoryValueTree::getContentsOfFolder (juce::File folder)
{
    juce::ValueTree folderVT {"Folder"};
    folderVT.setProperty ("name", folder.getFullPathName (), nullptr);
    for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
    {
        if (threadShouldExit ())
            break;

        // TODO provide progress updates
        // doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] ()
        // {
        //     juce::MessageManager::callAsync ([this, fileName] ()
        //         {
        //             validatorProperties.setProgressUpdate (fileName, false);
        //         });
        // });
        if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
        {
            folderVT.addChild (getContentsOfFolder (curFile), -1, nullptr);
        }
        else
        {
            juce::ValueTree fileVT {"File"};
            fileVT.setProperty ("name", curFile.getFullPathName (), nullptr);
            folderVT.addChild (fileVT, -1, nullptr);
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
