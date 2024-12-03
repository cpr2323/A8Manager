#pragma once

#include <JuceHeader.h>
#include "DirectoryDataProperties.h"
#include "../Assimil8or/Audio/AudioManager.h"
#include "../Utility/LambdaThread.h"
#include "../Utility/ValueTreeMonitor.h"
#include "../Utility/WatchDogTimer.h"

class DirectoryValueTree : public juce::Thread,
                           private juce::Timer,
                           private juce::AsyncUpdater
{
public:
    DirectoryValueTree ();
    ~DirectoryValueTree ();

    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getDirectoryDataPropertiesVT ();

private:
    enum class ScanType
    {
        checkForUpdate,
        fullScan
    };
    enum class TaskManagementState
    {
        idle,
        startScan,
        scanning,
        startCheck,
        checking,
    };
    WatchdogTimer timer; // TODO - remove when not needed, ie. when done measuring things
    DirectoryDataProperties directoryDataProperties;
    AudioManager* audioManager { nullptr };
    LambdaThread scanThread { "ScanThread", 1000 };
    LambdaThread checkThread { "CheckThread", 1000 };

    int scanDepth { -1 };
    juce::int64 lastScanInProgressUpdate {};
    std::atomic<bool> cancelScan { false };
    std::atomic<bool> cancelCheck { false };
    juce::CriticalSection taskManagementCS;
    TaskManagementState requestedTaskManagementState { TaskManagementState::idle };
    TaskManagementState currentTaskManagementState { TaskManagementState::idle };
    std::atomic<ScanType> scanType { ScanType::fullScan };

    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void doProgressUpdate (juce::String progressString);
    void getContentsOfFolder (juce::ValueTree folderVT, int curDepth, std::function<bool ()> shouldCancelFunc);
    juce::String getPathFromCurrentRoot (juce::String fullPath);
    TaskManagementState getCurrentTaskManagementState ();
    juce::String getTaskManagementStateString (TaskManagementState theThreadState);
    TaskManagementState getRequestedTaskManagementState ();
    bool hasFolderChanged (juce::ValueTree directoryVT);
    juce::ValueTree makeFileEntry (juce::File file, juce::int64 createTime, juce::int64 modificationTime, DirectoryDataProperties::TypeIndex fileType);
    void scanDirectory ();
    void sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus);
    void setCurrentTaskManagementState (DirectoryValueTree::TaskManagementState newThreadState);
    void setScanDepth (int theScanDepth);
    bool setRequestedTaskManagementState (DirectoryValueTree::TaskManagementState newThreadState);
    bool shouldCancelOperation (LambdaThread& whichTaskThread, std::atomic<bool>& whichTaskCancelToCheck);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT, std::function<bool ()> shouldCancelFunc);
    void startScan ();
    void wakeUpTaskManagmentThread ();

    ValueTreeMonitor ddpMonitor;
    ValueTreeMonitor rootFolderMonitor;

    void run () override;
    void timerCallback () override;
    void handleAsyncUpdate () override;
};
