#include "DirectoryValueTree.h"
#include "../Assimil8or/FileTypeHelpers.h"
#include "../SystemServices.h"
#include "../Utility/DebugLog.h"
#include "../Utility/RuntimeRootProperties.h"

#define LOG_DIRECTORY_VALUE_TREE 0
#if LOG_DIRECTORY_VALUE_TREE
#define LogDirectoryValueTree(cond, text) if (cond) {DebugLog ("DirectoryValueTree", text); }
#else
#define LogDirectoryValueTree(cond, text) ;
#endif

#define SHOW_CHECK_STATE_LOG false
#define SHOW_TASK_MANAGEMENT_LOG false

DirectoryValueTree::DirectoryValueTree () : Thread ("DirectoryValueTree")
{
    startThread ();
    scanThread.onThreadLoop = [this] ()
    {
        if (! scanThread.waitForNotification (-1) || scanThread.shouldExit ())
            return false;

        LogDirectoryValueTree (true, "scanThread.onThreadLoop - calling scanDirectory ()");
        scanDirectory ();
        setRequestedTaskManagementState (TaskManagementState::idle);
        wakeUpTaskManagmentThread ();
        sendStatusUpdate (DirectoryDataProperties::ScanStatus::done);
        doProgressUpdate ("");

        return true;
    };
    scanThread.start ();
    checkThread.onThreadLoop = [this] ()
    {
        if (! checkThread.waitForNotification (-1) || checkThread.shouldExit ())
            return false;

        LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "checkThread.onThreadLoop - TaskManagementState::checking");
        if (hasFolderChanged (directoryDataProperties.getRootFolderVT ()))
        {
            setRequestedTaskManagementState (TaskManagementState::startScan);
            wakeUpTaskManagmentThread ();
        }
        else
        {
            setRequestedTaskManagementState (TaskManagementState::idle);
            wakeUpTaskManagmentThread ();
        }

        return true;
    };
    checkThread.start ();
}

DirectoryValueTree::~DirectoryValueTree ()
{
    stopThread (500);
}

void DirectoryValueTree::wakeUpTaskManagmentThread ()
{
    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "wakeUpTaskManagmentThread");
    notify ();
}

void DirectoryValueTree::init (juce::ValueTree runtimeRootPropertiesVT)
{
    SystemServices systemServices { runtimeRootPropertiesVT, SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    audioManager = systemServices.getAudioManager ();

    directoryDataProperties.wrap (runtimeRootPropertiesVT, DirectoryDataProperties::WrapperType::owner, DirectoryDataProperties::EnableCallbacks::yes);
    //ddpMonitor.assign (directoryDataProperties.getValueTreeRef ());

    directoryDataProperties.onScanDepthChange = [this] (int scanDepth) { setScanDepth (scanDepth); };
    directoryDataProperties.onStartScanChange = [this] ()
    {
        FolderProperties fp (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
        LogDirectoryValueTree (true, "init - directoryDataProperties.onStartScanChange - " + fp.getName ());
        startScan ();
    };

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

void DirectoryValueTree::doIfProgressTimeElapsed (std::function<void ()> functionToDo)
{
    jassert (functionToDo != nullptr);
    if (juce::Time::currentTimeMillis () - lastScanInProgressUpdate > 250)
    {
        lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
        functionToDo ();
    }
}

void DirectoryValueTree::sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus)
{
    juce::MessageManager::callAsync ([this, scanStatus] ()
    {
        //juce::Logger::outputDebugString ("setting scanStatus: " + juce::String (static_cast<int> (scanStatus)));
        directoryDataProperties.setStatus (scanStatus, false);
    });
}

juce::ValueTree DirectoryValueTree::makeFileEntry (juce::File file, juce::int64 createTime, juce::int64 modificationTime, DirectoryDataProperties::TypeIndex fileType)
{
    auto fileVT { FileProperties::create (file.getFullPathName (), createTime, modificationTime, fileType) };
    if (scanType == ScanType::fullScan)
    {
        switch (fileType)
        {
        case DirectoryDataProperties::TypeIndex::audioFile:
        {
            if (auto reader { audioManager->getReaderFor (file) }; reader == nullptr)
            {
                fileVT.setProperty ("error", "invalid format", nullptr);
            }
            else
            {
                fileVT.setProperty ("dataType", (reader->usesFloatingPointData == true ? "floating point" : "integer"), nullptr);
                fileVT.setProperty ("bitDepth", static_cast<int> (reader->bitsPerSample), nullptr);
                fileVT.setProperty ("numChannels", static_cast<int> (reader->numChannels), nullptr);
                fileVT.setProperty ("sampleRate", static_cast<int> (reader->sampleRate), nullptr);
                fileVT.setProperty ("lengthSamples", static_cast<juce::int64> (reader->lengthInSamples), nullptr);
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

bool DirectoryValueTree::shouldCancelOperation (LambdaThread& whichTaskThread, std::atomic<bool>& whichTaskCancelToCheck)
{
    return whichTaskThread.shouldExit () || whichTaskCancelToCheck;
}

void DirectoryValueTree::startScan ()
{
    LogDirectoryValueTree (true, "startScan - waking up scan thread");
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    jassert (! rootFolderProperties.getName ().isEmpty ());
    setRequestedTaskManagementState (TaskManagementState::startScan);
    wakeUpTaskManagmentThread ();
}

void DirectoryValueTree::timerCallback ()
{
    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "timerCallback - doChangeCheck");
    if (setRequestedTaskManagementState (TaskManagementState::startCheck))
        wakeUpTaskManagmentThread ();
}

void DirectoryValueTree::handleAsyncUpdate ()
{
    LogDirectoryValueTree (true, "handleAsyncUpdate - restarting - startScan");
    cancelCheck = false;
    cancelScan = false;
    wakeUpTaskManagmentThread ();
}

DirectoryValueTree::TaskManagementState DirectoryValueTree::getCurrentTaskManagementState ()
{
    juce::ScopedLock sl (taskManagementCS);
    return currentTaskManagementState;
}

juce::String DirectoryValueTree::getTaskManagementStateString (TaskManagementState theTaskMangementState)
{
    switch (theTaskMangementState)
    {
        case TaskManagementState::idle: return "idle"; break;
        case TaskManagementState::startScan: return "startScan"; break;
        case TaskManagementState::scanning: return "scanning"; break;
        case TaskManagementState::startCheck: return "startCheck"; break;
        case TaskManagementState::checking: return "checking"; break;
        default: jassertfalse; return ""; break;
    }
}

void DirectoryValueTree::setCurrentTaskManagementState (TaskManagementState newTaskManagementState)
{
    juce::ScopedLock sl (taskManagementCS);
    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "setCurrentTaskManagementState: " + getTaskManagementStateString (newTaskManagementState));
    currentTaskManagementState = newTaskManagementState;
}

DirectoryValueTree::TaskManagementState DirectoryValueTree::getRequestedTaskManagementState ()
{
    juce::ScopedLock sl (taskManagementCS);
    return requestedTaskManagementState;
}

bool DirectoryValueTree::setRequestedTaskManagementState (DirectoryValueTree::TaskManagementState newTaskManagementState)
{
    juce::ScopedLock sl (taskManagementCS);
    // since the check process is running every X milliseconds, we can skip it if we aren't idle
    if (newTaskManagementState == TaskManagementState::startCheck && currentTaskManagementState != TaskManagementState::idle)
    {
        LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "setRequestedTaskManagementState - skipping TaskManagementState::startCheck because currentTaskMangementState == " +
                                     getTaskManagementStateString (currentTaskManagementState));
        return false;
    }
    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "setRequestedTaskManagementState - requesting: " + getTaskManagementStateString (newTaskManagementState));
    requestedTaskManagementState = newTaskManagementState;
    return true;
}

void DirectoryValueTree::run ()
{
    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - enter");
    while (wait (-1) && ! threadShouldExit ())
    {
        if (! threadShouldExit ())
        {
            const auto requestedTMS { getRequestedTaskManagementState () };
            setCurrentTaskManagementState (requestedTMS);
            LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - currentTaskMangementState == " + getTaskManagementStateString (requestedTMS));
            switch (requestedTMS)
            {
                case TaskManagementState::idle:
                {
                    // spurious wake up?
                    //jassertfalse;
                }
                break;
                case TaskManagementState::startScan:
                {
                    if (! checkThread.isWaiting ())
                    {
                        LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - check thread is still running. waiting for completion");
                        cancelCheck = true;
                        triggerAsyncUpdate ();
                    }
                    else if (! scanThread.isWaiting ())
                    {
                        LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - scan thread is still running. waiting for completion");
                        cancelScan = true;
                        triggerAsyncUpdate ();
                    }
                    else
                    {
                        LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - starting scan thread");
                        currentTaskManagementState = TaskManagementState::scanning;
                        sendStatusUpdate (DirectoryDataProperties::ScanStatus::scanning);
                        scanThread.wake ();
                    }
                }
                break;
                case TaskManagementState::scanning:
                {
                    // TODO - I don't think we should ever end up here
                    jassertfalse;
                }
                break;
                case TaskManagementState::startCheck:
                {
                    // as this is a time repeated task, we can skip it if anything else is already going on
                    if (scanThread.isWaiting () && checkThread.isWaiting ())
                    {
                        LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - starting check thread");
                        currentTaskManagementState = TaskManagementState::checking;
                        checkThread.wake ();
                    }
                }
                break;
                case TaskManagementState::checking:
                {
                    // TODO - I don't think we should ever end up here
                    //jassertfalse;
                }
                break;
            }
        }
    }
    LogDirectoryValueTree (true, "run - exit");
}

juce::String DirectoryValueTree::getPathFromCurrentRoot (juce::String fullPath)
{
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    const auto partialPath { fullPath.fromLastOccurrenceOf (rootFolderProperties .getName (), false, true) };
    return partialPath;
}

void DirectoryValueTree::scanDirectory ()
{
    LogDirectoryValueTree (true, "scanDirectory ()");
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    // do one initial progress update to fill in the first one
    doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (juce::File (rootFolderProperties.getName ()).getFileName ()));
    // clear old contents
    timer.start (100000);
    directoryDataProperties.getRootFolderVT ().removeAllChildren (nullptr);
    scanType = ScanType::fullScan;
    getContentsOfFolder (directoryDataProperties.getRootFolderVT (), 0, [this] () { return shouldCancelOperation (scanThread, cancelScan); });
    // reset the output if scan was canceled
    if (shouldCancelOperation (scanThread, cancelScan))
    {
        LogDirectoryValueTree (true, "scanDirectory - operation cancelled, removing all data");
        directoryDataProperties.getRootFolderVT ().removeAllChildren (nullptr);
    }
    //juce::Logger::outputDebugString ("DirectoryValueTree::scanDirectory ()- elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void DirectoryValueTree::doProgressUpdate (juce::String progressString)
{
    juce::MessageManager::callAsync ([this, progressString] ()
    {
        directoryDataProperties.setProgress (progressString, false);
    });
}

bool DirectoryValueTree::hasFolderChanged (juce::ValueTree rootFolderVT)
{
    FolderProperties rootFolderProperties (rootFolderVT, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
    FolderProperties newCopyOfFolderProperties ({}, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
    newCopyOfFolderProperties.setName (rootFolderProperties.getName (), false);
    scanType = ScanType::checkForUpdate;
    getContentsOfFolder (newCopyOfFolderProperties.getValueTree (), 0, [this] () { return shouldCancelOperation (checkThread, cancelCheck); });
    if (rootFolderProperties.getValueTree ().getNumChildren () != newCopyOfFolderProperties.getValueTree ().getNumChildren ())
    {
        LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - number of children differ - do rescan");
        return true;
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
                LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " differ in type - do rescan");
                return true;
            }
            else
            {
                FolderProperties curNewFolderChildFolder (newChildVT, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
                if (curRootFolderChildFolder.getName () != curNewFolderChildFolder.getName ())
                {
                    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " names differ - do rescan");
                    return true;
                }
                if (curRootFolderChildFolder.getCreateTime () != curNewFolderChildFolder.getCreateTime ())
                {
                    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " creation times differ - do rescan");
                    return true;
                }
                if (curRootFolderChildFolder.getModificationTime () != curNewFolderChildFolder.getModificationTime ())
                {
                    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " modification times differ - do rescan");
                    return true;
                }
            }
        }
        else
        {
            FileProperties curRootFolderChildFile (rootChildVT, FileProperties::WrapperType::owner, FileProperties::EnableCallbacks::no);
            if (! FileProperties::isFileVT (newChildVT))
            {
                LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " differ in type - do rescan");
                return true;
            }
            else
            {
                FileProperties curNewFolderChildFile (newChildVT, FileProperties::WrapperType::owner, FileProperties::EnableCallbacks::no);
                if (curRootFolderChildFile.getName () != curNewFolderChildFile.getName ())
                {
                    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " names differ - do rescan");
                    return true;
                }
                if (curRootFolderChildFile.getCreateTime () != curNewFolderChildFile.getCreateTime ())
                {
                    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " creation times differ - do rescan");
                    return true;
                }
                if (curRootFolderChildFile.getModificationTime () != curNewFolderChildFile.getModificationTime ())
                {
                    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - item #" + juce::String (childIndex) + " modification times differ - do rescan");
                    return true;
                }

            }
        }
    }
    return false;
}

void DirectoryValueTree::getContentsOfFolder (juce::ValueTree folderVT, int curDepth, std::function<bool ()> shouldCancelFunc)
{
    FolderProperties folderProperties (folderVT, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    if (scanDepth == -1 || curDepth <= scanDepth)
    {
        for (const auto& entry : juce::RangedDirectoryIterator (folderProperties.getName (), false, "*", juce::File::findFilesAndDirectories))
        {
            if (shouldCancelFunc ())
                break;

            const auto creationTime { entry.getFile ().getCreationTime ().getMilliseconds () };
            const auto modificationTime { entry.getFile ().getLastModificationTime ().getMilliseconds () };
            if (scanType == ScanType::fullScan)
                doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] () { doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (fileName)); });
            if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
                folderVT.addChild (FolderProperties::create (curFile.getFullPathName (), creationTime, modificationTime), -1, nullptr);
            else
                folderVT.addChild (makeFileEntry (curFile, creationTime, modificationTime, FileTypeHelpers::getFileType (curFile)), -1, nullptr);
        }
        sortContentsOfFolder (folderVT, shouldCancelFunc);
        if (scanType == ScanType::fullScan && curDepth == 0)
            directoryDataProperties.triggerRootScanComplete (false);

        // scan the subfolders
        ValueTreeHelpers::forEachChildOfType (folderVT, FolderProperties::FolderTypeId, [this, curDepth, shouldCancelFunc] (juce::ValueTree childFolderVT)
        {
            getContentsOfFolder (childFolderVT, curDepth + 1, shouldCancelFunc);
            return true;
        });
    }
}

void DirectoryValueTree::sortContentsOfFolder (juce::ValueTree rootFolderVT, std::function<bool ()> shouldCancelFunc)
{
    jassert (FolderProperties::isFolderVT (rootFolderVT));

    // Folders
    // System files (folderprefs, lastfolder, lastpreset, midiX)
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
    for (auto folderIndex { 0 }; folderIndex < numFolderEntries && ! shouldCancelFunc (); ++folderIndex)
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
            switch (static_cast<int> (directoryEntryVT.getProperty ("type")))
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
