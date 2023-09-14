#pragma once

#include <JuceHeader.h>
#include "DirectoryDataProperties.h"
#include "../Utility/ValueTreeMonitor.h"

class DirectoryValueTree : public juce::Thread,
                           private juce::Timer
{
public:
    DirectoryValueTree ();
    DirectoryValueTree (juce::String theRootFolderName);
    ~DirectoryValueTree ();

    void init (juce::ValueTree rootPropertiesVT);
    juce::ValueTree getDirectoryDataPropertiesVT ();

    // since we are now controlling this with DirectoryDataProperties 
    // considering removal or making private
    void clear ();
    void cancel ();
    juce::ValueTree getDirectoryVT ();
    juce::String getRootFolder ();
    bool isScanning ();
    void setRootFolder (juce::String theRootFolderName);
    void setScanDepth (int theScanDepth);
    void startScan ();
    void tempEnableLogging () { doLogging = true; }

    std::function<void (bool success)> onComplete;
    std::function<void (juce::String operation, juce::String fileName)> onStatusChange;

private:
    DirectoryDataProperties directoryDataProperties;
    juce::String rootFolderName;
    int scanDepth { -1 };
    int64_t lastScanInProgressUpdate {};
    bool doLogging { false };
    std::atomic<bool> scanning { false };
    std::atomic<bool> cancelScan { false };

    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    juce::ValueTree doScan ();
    void doStatusUpdate (juce::String operation, juce::String fileName);
    juce::ValueTree getContentsOfFolder (juce::File folder, int curDepth);
    juce::ValueTree getFolderEntry ();
    void updateDirectoryData (juce::ValueTree newDirectoryData);
    void sendStatusUpdate (DirectoryDataProperties::ScanStatus scanStatus);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    bool shouldCancelOperation ();

    ValueTreeMonitor ddpMonitor;
    ValueTreeMonitor rootFolderMonitor;

    void run () override;
    void timerCallback () override;
};