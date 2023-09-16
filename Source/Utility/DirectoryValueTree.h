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
    juce::String rootFolderName;
    int scanDepth { -1 };
    int64_t lastScanInProgressUpdate {};
    std::atomic<bool> scanning { false };
    std::atomic<bool> cancelScan { false };

    void cancel ();
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    juce::ValueTree doScan ();
    juce::ValueTree getContentsOfFolder (juce::File folder, int curDepth);
    bool isFolderEntry (juce::ValueTree folderVT);
    bool isFileEntry (juce::ValueTree fileVT);
    bool isScanning ();
    juce::String getEntryName (juce::ValueTree fileVT);
    juce::ValueTree makeFileEntry (juce::File file, DirectoryDataProperties::TypeIndex fileType);
    juce::ValueTree makeFolderEntry (juce::File folder);
    juce::ValueTree makeFolderEntry (juce::String filePath);
    void updateDirectoryData (juce::ValueTree newDirectoryData);
    void sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus);
    void setRootFolder (juce::String theRootFolderName);
    void setScanDepth (int theScanDepth);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    void startScan ();
    bool shouldCancelOperation ();

    ValueTreeMonitor ddpMonitor;
    ValueTreeMonitor rootFolderMonitor;

    void run () override;
    void timerCallback () override;
};
