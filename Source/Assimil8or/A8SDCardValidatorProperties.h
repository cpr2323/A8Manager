#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class A8SDCardValidatorProperties : public ValueTreeWrapper
{
public:
    A8SDCardValidatorProperties () noexcept : ValueTreeWrapper (Assimil8orSDCardValidatorId) {}

    void setRootFolder (juce::String rootFolder, bool includeSelfCallback);
    void setScanStatus (juce::String scanStatus, bool includeSelfCallback);
    void startAsyncScan (bool includeSelfCallback);

    juce::String getRootFolder ();
    juce::String getScanStatus ();

    std::function<void (juce::String rootFolder)> onRootFolderChanged;
    std::function<void (juce::String scanStatus)> onScanStatusChanged;
    std::function<void ()> onStartScanAsync;

    juce::ValueTree getValidationStatusVT ()
    {
        return data.getChildWithName (ValidationsStatusId);
    }

    static inline const juce::Identifier Assimil8orSDCardValidatorId { "Assimil8orSDCardValidator" };
    static inline const juce::Identifier RootFolderPropertyId { "rootFolder" };
    static inline const juce::Identifier ScanStatusPropertyId { "scanStatus" };
    static inline const juce::Identifier StartScanAsyncPropertyId { "startScan" };

    static inline const juce::Identifier ValidationsStatusId { "ValidationStatus" };
private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};