#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class ValidatorProperties : public ValueTreeWrapper
{
public:
    ValidatorProperties () noexcept : ValueTreeWrapper (Assimil8orValidatorId) {}

    void setRootFolder (juce::String rootFolder, bool includeSelfCallback);
    void setScanStatus (juce::String scanStatus, bool includeSelfCallback);
    void setProgressUpdate (juce::String progressUpdate, bool includeSelfCallback);
    void startAsyncScan (bool includeSelfCallback);

    juce::String getRootFolder ();
    juce::String getScanStatus ();
    juce::String getProgressUpdate ();

    std::function<void (juce::String rootFolder)> onRootFolderChanged;
    std::function<void (juce::String scanStatus)> onScanStatusChanged;
    std::function<void (juce::String progressUpdate)> onProgressUpdateChanged;
    std::function<void ()> onStartScanAsync;

    juce::ValueTree getValidationStatusVT ()
    {
        return data.getChildWithName (ValidationsStatusId);
    }

    static inline const juce::Identifier Assimil8orValidatorId { "Assimil8orValidator" };
    static inline const juce::Identifier RootFolderPropertyId { "rootFolder" };
    static inline const juce::Identifier ScanStatusPropertyId { "scanStatus" };
    static inline const juce::Identifier StartScanAsyncPropertyId { "startScan" };
    static inline const juce::Identifier ProgressUpdatePropertyId { "progressUpdate" };

    static inline const juce::Identifier ValidationsStatusId { "ValidationStatus" };
private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};