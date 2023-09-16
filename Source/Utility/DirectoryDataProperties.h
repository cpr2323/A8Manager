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

    enum class ScanStatus
    {
        empty,
        scanning,
        canceled,
        done
    };

    enum TypeIndex
    {
        folder,
        systemFile,
        presetFile,
        audioFile,
        unknownFile,
        size
    };

    void setProgress (juce::String progressString, bool includeSelfCallback);
    void setRootFolder (juce::String rootFolder, bool includeSelfCallback);
    void setScanDepth (int scanDepth, bool includeSelfCallback);
    void setStatus (ScanStatus status, bool includeSelfCallback);
    void triggerStartScan (bool includeSelfCallback);

    juce::String getProgress ();
    juce::String getRootFolder ();
    int getScanDepth ();
    DirectoryDataProperties::ScanStatus getStatus ();

    std::function<void (juce::String progressString)> onProgressChange;
    std::function<void (juce::String rootFolder)> onRootFolderChange;
    std::function<void (int scanDepth)> onScanDepthChange;
    std::function<void (ScanStatus status)> onStatusChange;
    std::function<void ()> onStartScanChange;

    juce::ValueTree getDirectoryValueTreeVT ();
    juce::ValueTree getDirectoryValueTreeContainerVT ();

    static inline const juce::Identifier DirectoryDataTypeId { "DirectoryData" };
    static inline const juce::Identifier ProgressPropertyId   { "progress" };
    static inline const juce::Identifier RootFolderPropertyId { "rootFolder" };
    static inline const juce::Identifier ScanDepthPropertyId  { "scanDepth" };
    static inline const juce::Identifier StartScanPropertyId  { "startScan" };
    static inline const juce::Identifier StatusPropertyId     { "status" };

    static inline const juce::Identifier DirectoryValueTreeTypeId { "directoryValueTree" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};

