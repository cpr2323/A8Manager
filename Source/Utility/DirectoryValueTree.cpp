#include "DirectoryValueTree.h"
#include "../Assimil8or/FileTypeHelpers.h"
#include "../SystemServices.h"
#include "../Utility/DebugLog.h"
#include "../Utility/RuntimeRootProperties.h"
#include "../Utility/ValueTreeHelpers.h"

#define LOG_DIRECTORY_VALUE_TREE 0
#if LOG_DIRECTORY_VALUE_TREE
#define LogDirectoryValueTree(cond, text) if (cond) { DebugLog ("DirectoryValueTree", text); }
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
        setTaskCompleteIfNoScanPending ();
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
        if (hasFolderChanged ())
        {
            scanRequestPending = true;
            setRequestedTaskManagementState (TaskManagementState::startScan);
            wakeUpTaskManagmentThread ();
        }
        else
        {
            setTaskCompleteIfNoScanPending ();
            wakeUpTaskManagmentThread ();
        }

        return true;
    };
    checkThread.start ();
}

DirectoryValueTree::~DirectoryValueTree ()
{
    // the scan and check threads use members of this object, including taskManagementCS and rootFolderNameCS, which are
    // declared after them and are therefore destroyed before them. left to the LambdaThread destructors, those threads
    // would still be running while those members were being torn down, and a task reporting its completion would take a
    // lock that no longer exists. so everything that can call back into this object is shut down here, in order, while
    // all of it is still alive
    stopTimer ();
    // let any scan/check that is part way through give up promptly, instead of running to completion
    cancelScan = true;
    cancelCheck = true;
    // the task management thread first, so that it cannot wake the workers back up after they have been stopped
    stopThread (500);
    scanThread.stop ();
    checkThread.stop ();
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

    // capture a handle to the live root folder tree while on the message thread. the scan thread only uses it
    // to pass the live tree to publish operations that run on the message thread
    rootFolderVTForTask = directoryDataProperties.getRootFolderVT ();

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
    // called on the message thread, where reading the live tree is safe. capture the root folder name here,
    // since the scan/check threads cannot safely read it from the live tree themselves
    jassert (juce::MessageManager::existsAndIsCurrentThread ());
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    jassert (! rootFolderProperties.getName ().isEmpty ());
    {
        juce::ScopedLock sl (rootFolderNameCS);
        rootFolderTaskName = rootFolderProperties.getName ();
    }
    scanRequestPending = true;
    setRequestedTaskManagementState (TaskManagementState::startScan);
    wakeUpTaskManagmentThread ();
}

void DirectoryValueTree::setTaskCompleteIfNoScanPending ()
{
    // a scan that was requested while this task was running has not been started yet, so its request must be
    // left in place. without this, a task finishing right after a request would report idle over it, and the
    // requested scan would never happen
    if (scanRequestPending)
        return;
    setRequestedTaskManagementState (TaskManagementState::idle);
}

juce::String DirectoryValueTree::getRootFolderTaskName ()
{
    juce::ScopedLock sl (rootFolderNameCS);
    return rootFolderTaskName;
}

void DirectoryValueTree::timerCallback ()
{
    LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "timerCallback - doChangeCheck");
    if (setRequestedTaskManagementState (TaskManagementState::startCheck))
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
    while (! threadShouldExit ())
    {
        // while we are waiting for a task to notice that it has been cancelled, the wait is bounded so that we come
        // back and re-check. the task can finish of its own accord before it ever sees the flag, in which case there
        // is no wake up coming, and a pending scan request would sit here forever
        wait ((cancelScan || cancelCheck) ? 10 : -1);
        if (threadShouldExit ())
            break;
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
                    // the flag stays set until the check has actually stopped. clearing it before then, as the
                    // async update used to, means the check never sees it and can never be cancelled
                    cancelCheck = true;
                }
                else if (! scanThread.isWaiting ())
                {
                    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - scan thread is still running. waiting for completion");
                    cancelScan = true;
                }
                else
                {
                    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - starting scan thread");
                    // nothing is running any more, so the cancel requests have been served
                    cancelCheck = false;
                    cancelScan = false;
                    scanRequestPending = false;
                    // the request has been acted on. left as startScan, the next wake up of this thread would come
                    // back through here, find the scan thread busy, and cancel the scan that was just started
                    setRequestedTaskManagementState (TaskManagementState::scanning);
                    setCurrentTaskManagementState (TaskManagementState::scanning);
                    sendStatusUpdate (DirectoryDataProperties::ScanStatus::scanning);
                    scanThread.wake ();
                }
            }
            break;
            case TaskManagementState::scanning:
            {
                // the scan thread is running. it wakes this thread again when it is done
            }
            break;
            case TaskManagementState::startCheck:
            {
                // as this is a time repeated task, we can skip it if anything else is already going on
                if (scanThread.isWaiting () && checkThread.isWaiting ())
                {
                    LogDirectoryValueTree (SHOW_TASK_MANAGEMENT_LOG, "run - starting check thread");
                    setRequestedTaskManagementState (TaskManagementState::checking);
                    setCurrentTaskManagementState (TaskManagementState::checking);
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
    LogDirectoryValueTree (true, "run - exit");
}

juce::String DirectoryValueTree::getPathFromCurrentRoot (juce::String fullPath)
{
    const auto partialPath { fullPath.fromLastOccurrenceOf (getRootFolderTaskName (), false, true) };
    return partialPath;
}

void DirectoryValueTree::scanDirectory ()
{
    LogDirectoryValueTree (true, "scanDirectory ()");
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootFolderName { getRootFolderTaskName () };
    // do one initial progress update to fill in the first one
    doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (juce::File (rootFolderName).getFileName ()));
    timer.start (100000);
    // scan into a detached tree, since the live tree (which has listeners) may only be modified on the message thread
    FolderProperties scannedFolderProperties ({}, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
    scannedFolderProperties.setName (rootFolderName, false);
    scanType = ScanType::fullScan;
    getContentsOfFolder (scannedFolderProperties.getValueTree (), 0, [this] () { return shouldCancelOperation (scanThread, cancelScan); });
    if (! shouldCancelOperation (scanThread, cancelScan))
    {
        // keep a detached copy for the check thread to compare against
        lastScanResultVT = scannedFolderProperties.getValueTree ().createCopy ();
        // publish the scanned data into the live tree on the message thread. this thread must not touch the scanned tree after this
        ValueTreeHelpers::replaceChildrenOnMessageThread (rootFolderVTForTask, scannedFolderProperties.getValueTree ());
    }
    else
    {
        LogDirectoryValueTree (true, "scanDirectory - operation cancelled, removing all data");
        lastScanResultVT = {};
        ValueTreeHelpers::callOnMessageThread ([liveRootFolderVT = rootFolderVTForTask] () mutable { liveRootFolderVT.removeAllChildren (nullptr); });
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

bool DirectoryValueTree::hasFolderChanged ()
{
    // runs on the check thread, so it compares the file system against a detached copy of the last scan result,
    // since the live tree may only be safely accessed from the message thread
    if (! lastScanResultVT.isValid ())
        return false;
    FolderProperties rootFolderProperties (lastScanResultVT, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    // the folder being viewed can change while we are idle, in which case the last scan result describes some other
    // folder, and the contents need to be rescanned
    const auto currentRootFolderName { getRootFolderTaskName () };
    if (rootFolderProperties.getName () != currentRootFolderName)
    {
        LogDirectoryValueTree (SHOW_CHECK_STATE_LOG, "hasFolderChanged - root folder changed - do rescan");
        return true;
    }
    FolderProperties newCopyOfFolderProperties ({}, FolderProperties::WrapperType::owner, FolderProperties::EnableCallbacks::no);
    newCopyOfFolderProperties.setName (currentRootFolderName, false);
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
        {
            // publish a snapshot of the root level results right away (they were previously available immediately,
            // since the scan used to write directly into the live tree). copied, because this thread continues
            // to fill in the subfolders of folderVT
            ValueTreeHelpers::replaceChildrenOnMessageThread (rootFolderVTForTask, folderVT.createCopy (), [this] ()
            {
                directoryDataProperties.triggerRootScanComplete (false);
            });
        }

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
