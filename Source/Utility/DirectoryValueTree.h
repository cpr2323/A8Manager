#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

class DirectoryDataProperties : public ValueTreeWrapper<DirectoryDataProperties>
{
public:
    DirectoryDataProperties () noexcept : ValueTreeWrapper<DirectoryDataProperties> (DirectoryDataTypeId)
    {
    }
    DirectoryDataProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<DirectoryDataProperties> (DirectoryDataTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setRootFolder (juce::String rootFolder, bool includeSelfCallback);
    void setScanDepth(int scanDepth, bool includeSelfCallback);
    void triggerStartScan (bool includeSelfCallback);

    juce::String getRootFolder ();
    int getScanDepth ();

    std::function<void (juce::String rootFolder)> onRootFolderChange;
    std::function<void (int scanDepth)> onScanDepthChange;
    std::function<void ()> onStartScanChange;

    static inline const juce::Identifier DirectoryDataTypeId { "DirectoryData" };
    static inline const juce::Identifier RootFolderPropertyId { "rootFolder" };
    static inline const juce::Identifier ScanDepthPropertyId  { "scanDepth" };
    static inline const juce::Identifier StatusPropertyId     { "status" };
    static inline const juce::Identifier StartScanPropertyId  { "startScan" };

    void initValueTree () {}
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};

class DirectoryValueTree : public juce::Thread
{
public:
    DirectoryValueTree ();
    DirectoryValueTree (juce::String theRootFolderName);
    ~DirectoryValueTree ();

    void init (juce::ValueTree rootPropertiesVT);
    void tempEnableLogging () { doLogging = true; }
    void clear ();
    void cancel ();
    juce::ValueTree getDirectoryVT ();
    juce::String getRootFolder ();
    bool isScanning ();
    void setRootFolder (juce::String theRootFolderName);
    void setScanDepth (int theScanDepth);
    void startScan ();

    std::function<void (bool success)> onComplete;
    std::function<void (juce::String operation, juce::String fileName)> onStatusChange;

private:
    DirectoryDataProperties directoryDataProperties;
    juce::String rootFolderName;
    int scanDepth { 0 };
    int64_t lastScanInProgressUpdate {};
    juce::ValueTree rootFolderVT;
    bool doLogging { false };
    std::atomic<bool> scanning { false };
    std::atomic<bool> cancelScan { false };

    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    juce::ValueTree doScan ();
    void doStatusUpdate (juce::String operation, juce::String fileName);
    juce::ValueTree getContentsOfFolder (juce::File folder, int curDepth);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    bool shouldCancelOperation ();

    void run () override;
};
