#include "DirectoryValueTree.h"
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
    fileVT.setProperty ("status", "unscanned", nullptr);
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

        doScan ();
        // reset the output if scan was canceled
        if (shouldCancelOperation ())
        {
            LogDirectoryValueTree (true, "DirectoryValueTree::run - threadShouldExit = true, rootFolderVT = {}");
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
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    FolderProperties rootFolderProperties (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    // do one initial progress update to fill in the first one
    doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (juce::File (rootFolderProperties.getName ()).getFileName ()));
    timer.start (100000);
    directoryDataProperties.getRootFolderVT ().removeAllChildren (nullptr);
    getContentsOfFolder (directoryDataProperties.getRootFolderVT (), 0);
    juce::Logger::outputDebugString ("DirectoryValueTree::doScan - getContentOfFolder - elapsed time: " + juce::String (timer.getElapsedTime ()));

    timer.start (100000);
    if (! shouldCancelOperation ())
        sortContentsOfFolder (directoryDataProperties.getRootFolderVT ());
    juce::Logger::outputDebugString ("DirectoryValueTree::doScan - sortContentOfFolder - elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void DirectoryValueTree::doProgressUpdate (juce::String progressString)
{
    juce::MessageManager::callAsync ([this, progressString] ()
    {
        directoryDataProperties.setProgress (progressString, false);
    });
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

            doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] () { doProgressUpdate ("Reading File System: " + getPathFromCurrentRoot (fileName)); });
            if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
                folderVT.addChild (makeFolderEntry (curFile.getFullPathName ()), -1, nullptr);
            else
                folderVT.addChild (makeFileEntry (curFile, FileTypeHelpers::getFileType (curFile)), -1, nullptr);
        }
        if (curDepth == 0)
            directoryDataProperties.triggerRootScanComplete (false);
        ValueTreeHelpers::forEachChildOfType (folderVT, FolderProperties::FolderTypeId, [this, curDepth] (juce::ValueTree childFolderVT)
        {
            getContentsOfFolder (childFolderVT, curDepth + 1);
            return true;
        });
    }
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
        doIfProgressTimeElapsed ([this, fileName = folderEntryVT.getProperty ("name").toString ()] () { doProgressUpdate ("Sorting File System: " + getPathFromCurrentRoot (fileName)); });
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
