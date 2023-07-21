#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class A8SDCardValidatorProperties : public ValueTreeWrapper
{
public:
    A8SDCardValidatorProperties () noexcept : ValueTreeWrapper (Assimil8orSDCardValidatorId) {}

    void setScanStatus (juce::String scanStatus, bool includeSelfCallback);
    juce::String getScanStatus ();

    std::function<void (juce::String scanStatus)> onScanStatusChanged;

    juce::ValueTree getValidationStatusVT ()
    {
        return data.getChildWithName (ValidationsStatusId);
    }

    static inline const juce::Identifier Assimil8orSDCardValidatorId { "Assimil8orSDCardValidator" };
    static inline const juce::Identifier ScanStatusPropertyId { "scanStatus" };

    static inline const juce::Identifier ValidationsStatusId { "ValidationStatus" };
private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};