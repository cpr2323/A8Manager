#include "DirectoryValueTree.h"
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
    : Thread ("DirectoryValueTree")
{
    startThread ();
}

DirectoryValueTree::~DirectoryValueTree ()
{
    stopThread (500);
}

void DirectoryValueTree::init (juce::ValueTree rootPropertiesVT)
{
    audioFormatManager.registerBasicFormats ();

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::owner, DirectoryDataProperties::EnableCallbacks::yes);

    //ddpMonitor.assign (directoryDataProperties.getValueTreeRef ());

    directoryDataProperties.onScanDepthChange = [this] (int scanDepth) { setScanDepth (scanDepth); };
    directoryDataProperties.onStartScanChange = [this] () { startScan (); };

    startTimer (250);
}

juce::ValueTree DirectoryValueTree::getDirectoryDataPropertiesVT ()
{
    return directoryDataProperties.getValueTree ();
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
        FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
        jassert (! rootFolderProperties.getName ().isEmpty ());
        notify ();
    }
    else
    {
        LogDirectoryValueTree (true, "DirectoryValueTree::startScan - still scanning - cancelling and starting timer");
        restarting = true;
        cancel ();
        startTimer (1);
    }
    LogDirectoryValueTree (true, "DirectoryValueTree::startScan - exit");
}

void DirectoryValueTree::timerCallback ()
{
    LogDirectoryValueTree (true, "DirectoryValueTree::timerCallback - enter");
    if (restarting)
    {
        startScan ();
        restarting = false;
    }
    else if (! isScanning() && ! checkFolderForChanges (directoryDataProperties.getRootFolderVT ()))
        startScan ();
    LogDirectoryValueTree (true, "DirectoryValueTree::timerCallback - exit");
    startTimer (250);
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
    auto fileVT { FileProperties::create (file.getFullPathName (), fileType) };
    if (scanType == ScanType::fullScan)
    {
        switch (fileType)
        {
            case DirectoryDataProperties::TypeIndex::audioFile:
            {
                if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file)); reader == nullptr)
                {
                    fileVT.setProperty ("error", "invalid format", nullptr);
                }
                else
                {
                    fileVT.setProperty ("dataType", (reader->usesFloatingPointData == true ? "floating point" : "integer"), nullptr);
                    fileVT.setProperty ("bitDepth", static_cast<int>(reader->bitsPerSample), nullptr);
                    fileVT.setProperty ("numChannels", static_cast<int>(reader->numChannels), nullptr);
                    fileVT.setProperty ("sampleRate", static_cast<int>(reader->sampleRate), nullptr);
                    fileVT.setProperty ("lengthSamples", static_cast<int64_t>(reader->lengthInSamples), nullptr);
                }
            }
            break;
            case DirectoryDataProperties::TypeIndex::folder:
            case DirectoryDataProperties::TypeIndex::systemFile:
            case DirectoryDataProperties::TypeIndex::presetFile:
            case DirectoryDataProperties::TypeIndex::unknownFile:
            {
                // TODO - implement specific properties where it makes sense. display entire contents of system files? display some stats about the preset files?
            }
            break;
            default: jassertfalse; break;
        }
    }
    return fileVT;
}

void DirectoryValueTree::run ()
{
    LogDirectoryValueTree (true, "DirectoryValueTree::run - enter");
    while (wait (-1) && ! threadShouldExit ())
    {
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::scanning);
        scanning = true;
        cancelScan = false;
        LogDirectoryValueTree (true, "DirectoryValueTree::run");

        doScan ();
        checkFolderForChanges (directoryDataProperties.getRootFolderVT ());
        // reset the output if scan was canceled
        if (shouldCancelOperation ())
        {
            LogDirectoryValueTree (true, "DirectoryValueTree::run - operation cancelled, removing all data");
            directoryDataProperties.getRootFolderVT ().removeAllChildren (nullptr);
        }
        scanning = false;
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::done);
        doProgressUpdate ("");
    }
    LogDirectoryValueTree (true, "DirectoryValueTree::run - exit");
}

juce::String DirectoryValueTree::getPathFromCurrentRoot (juce::String fullPath)
{
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    const auto partialPath { fullPath.fromLastOccurrenceOf (rootFolderProperties .getName (), false, true) };
    return partialPath;
}

void DirectoryValueTree::doScan ()
{
    juce::Logger::outputDebugString ("DirectoryValueTree::doScan ()");
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    // do one initial progress update to fill in the first one
    doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (juce::File (rootFolderProperties.getName ()).getFileName ()));
    // clear old contents
    timer.start (100000);
    directoryDataProperties.getRootFolderVT ().removeAllChildren (nullptr);
    scanType = ScanType::fullScan;
    getContentsOfFolder (directoryDataProperties.getRootFolderVT (), 0);
    juce::Logger::outputDebugString ("DirectoryValueTree::doScan ()- elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void DirectoryValueTree::doProgressUpdate (juce::String progressString)
{
    juce::MessageManager::callAsync ([this, progressString] ()
    {
        directoryDataProperties.setProgress (progressString, false);
    });
}

bool DirectoryValueTree::checkFolderForChanges (juce::ValueTree rootFolderVT)
{
    FolderProperties rootFolderProperties (rootFolderVT, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
    FolderProperties newCopyOfFolderProperties ({}, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
    newCopyOfFolderProperties.setName (rootFolderProperties.getName (), false);
    scanType = ScanType::checkForUpdate;
    getContentsOfFolder (newCopyOfFolderProperties.getValueTree (), 0);
    if (rootFolderProperties.getValueTree ().getNumChildren () != newCopyOfFolderProperties.getValueTree ().getNumChildren ())
    {
        juce::Logger::outputDebugString ("DirectoryValueTree::checkFolderForChanges - number of children differ - do rescan");
        return false;
    }
    for (auto childIndex { 0 }; childIndex < rootFolderProperties.getValueTree ().getNumChildren (); ++childIndex)
    {
        auto rootChildVT { rootFolderProperties.getValueTree ().getChild (childIndex) };
        auto newChildVT { newCopyOfFolderProperties.getValueTree ().getChild (childIndex) };
        if (FolderProperties::isFolderVT (rootChildVT))
        {
            FolderProperties curRootFolderChildFolder (rootChildVT, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
            if (! FolderProperties::isFolderVT (newChildVT))
            {
                juce::Logger::outputDebugString ("DirectoryValueTree::checkFolderForChanges - item #" + juce::String (childIndex) + " differ in type - do rescan");
                return false;
            }
            else
            {
                FolderProperties curNewFolderChildFolder (newChildVT, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
                if (curRootFolderChildFolder.getName () != curNewFolderChildFolder.getName())
                {
                    juce::Logger::outputDebugString ("DirectoryValueTree::checkFolderForChanges - item #" + juce::String (childIndex) + " names differ - do rescan");
                    return false;
                }
            }
        }
        else
        {
            FileProperties curRootFolderChildFile (rootChildVT, FileProperties::WrapperType::owner, FileProperties::EnableCallbacks::no);
            if (! FileProperties::isFileVT (newChildVT))
            {
                juce::Logger::outputDebugString ("DirectoryValueTree::checkFolderForChanges - item #" + juce::String (childIndex) + " differ in type - do rescan");
                return false;
            }
            else
            {
                FileProperties curNewFolderChildFile (newChildVT, FileProperties::WrapperType::owner, FileProperties::EnableCallbacks::no);
                if (curRootFolderChildFile.getName () != curNewFolderChildFile.getName ())
                {
                    juce::Logger::outputDebugString ("DirectoryValueTree::checkFolderForChanges - item #" + juce::String (childIndex) + " names differ - do rescan");
                    return false;
                }
            }
        }
    }
    return true;
}

void DirectoryValueTree::getContentsOfFolder (juce::ValueTree folderVT, int curDepth)
{
    FolderProperties folderProperties (folderVT, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    if (scanDepth == -1 || curDepth <= scanDepth)
    {
        for (const auto& entry : juce::RangedDirectoryIterator (folderProperties.getName (), false, "*", juce::File::findFilesAndDirectories))
        {
            if (shouldCancelOperation ())
                break;

            if (scanType == ScanType::fullScan)
                doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] () { doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (fileName)); });
            if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
                folderVT.addChild (FolderProperties::create (curFile.getFullPathName ()), -1, nullptr);
            else
                folderVT.addChild (makeFileEntry (curFile, FileTypeHelpers::getFileType (curFile)), -1, nullptr);
        }
        sortContentsOfFolder (folderVT);
        if (scanType == ScanType::fullScan && curDepth == 0)
            directoryDataProperties.triggerRootScanComplete (false);

        // scan the subfolders
        ValueTreeHelpers::forEachChildOfType (folderVT, FolderProperties::FolderTypeId, [this, curDepth] (juce::ValueTree childFolderVT)
        {
            getContentsOfFolder (childFolderVT, curDepth + 1);
            return true;
        });
    }
}

void DirectoryValueTree::sortContentsOfFolder (juce::ValueTree rootFolderVT)
{
    jassert (FolderProperties::isFolderVT(rootFolderVT));

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
    const auto numFolderEntries { rootFolderVT.getNumChildren () };

    auto insertSorted = [this, &sections, &rootFolderVT] (int itemIndex, int sectionIndex)
    {
        auto getEntryName = [] (juce::ValueTree dirEntryVT)
        {
            return dirEntryVT.getProperty ("name").toString ();
        };

        jassert (sectionIndex < DirectoryDataProperties::TypeIndex::size);
        auto& section { sections [sectionIndex] };
        jassert (itemIndex >= section.startIndex + section.length);
        auto startingSectionLength { section.length };
        auto insertItem = [&rootFolderVT, &section, &sections] (int itemIndex, int insertIndex)
        {
            auto tempVT { rootFolderVT.getChild (itemIndex) };
            rootFolderVT.removeChild (itemIndex, nullptr);
            rootFolderVT.addChild (tempVT, insertIndex, nullptr);
            ++section.length;
            for (auto curSectionIndex { 1 }; curSectionIndex < DirectoryDataProperties::TypeIndex::size; ++curSectionIndex)
                sections [curSectionIndex].startIndex = sections [curSectionIndex - 1].startIndex + sections [curSectionIndex - 1].length;
        };

        const auto fileName { getEntryName (rootFolderVT.getChild (itemIndex)).toLowerCase () };
        for (auto sectionEntryIndex { section.startIndex }; sectionEntryIndex < section.startIndex + section.length; ++sectionEntryIndex)
        {
            if (fileName < getEntryName (rootFolderVT.getChild (sectionEntryIndex)).toLowerCase ())
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
        auto directoryEntryVT { rootFolderVT.getChild (folderIndex) };
        if (scanType == ScanType::fullScan)
            doIfProgressTimeElapsed ([this, fileName = directoryEntryVT.getProperty ("name").toString ()] () { doProgressUpdate ("Sorting File System: " + getPathFromCurrentRoot (fileName)); });
        if (FolderProperties::isFolderVT (directoryEntryVT))
        {
            insertSorted (folderIndex, DirectoryDataProperties::TypeIndex::folder);
        }
        else if (FileProperties::isFileVT (directoryEntryVT))
        {
            // alphabetize in these lists
            //   System files
            //   Preset files
            //   Audio files
            //   unknown files
            switch (static_cast<int> (directoryEntryVT.getProperty("type")))
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
