#pragma once

#include <JuceHeader.h>
#include "DirectoryDataProperties.h"
#include "../Utility/ValueTreeMonitor.h"
#include "../Utility/WatchDogTimer.h"

class DirectoryValueTree : public juce::Thread,
                           private juce::Timer
{
public:
    DirectoryValueTree ();
    DirectoryValueTree (juce::String theRootFolderName);
    ~DirectoryValueTree ();

    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getDirectoryDataPropertiesVT ();

private:
    WatchdogTimer timer;
    DirectoryDataProperties directoryDataProperties;
    juce::AudioFormatManager audioFormatManager;
    int scanDepth { -1 };
    int64_t lastScanInProgressUpdate {};
    std::atomic<bool> scanning { false };
    std::atomic<bool> cancelScan { false };

    void cancel ();
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void doProgressUpdate (juce::String progressString);
    void doScan ();
    void getContentsOfFolder (juce::ValueTree folderVT, int curDepth);
    juce::String getPathFromCurrentRoot (juce::String fullPath);
    bool isFolderEntry (juce::ValueTree folderVT);
    bool isFileEntry (juce::ValueTree fileVT);
    bool isScanning ();
    juce::String getEntryName (juce::ValueTree fileVT);
    juce::ValueTree makeFileEntry (juce::File file, DirectoryDataProperties::TypeIndex fileType);
    juce::ValueTree makeFolderEntry (juce::File folder);
    juce::ValueTree makeFolderEntry (juce::String filePath);
    void sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus);
    void setScanDepth (int theScanDepth);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    void startScan ();
    bool shouldCancelOperation ();

    ValueTreeMonitor ddpMonitor;
    ValueTreeMonitor rootFolderMonitor;

    void run () override;
    void timerCallback () override;
};
