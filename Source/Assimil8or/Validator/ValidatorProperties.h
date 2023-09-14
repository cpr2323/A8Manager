#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"
#include "ValidatorResultListProperties.h"

class ValidatorProperties : public ValueTreeWrapper<ValidatorProperties>
{
public:
    ValidatorProperties () noexcept : ValueTreeWrapper<ValidatorProperties> (Assimil8orValidatorId) {}

    void setScanStatus (juce::String scanStatus, bool includeSelfCallback);
    void setProgressUpdate (juce::String progressUpdate, bool includeSelfCallback);

    juce::String getScanStatus ();
    juce::String getProgressUpdate ();

    std::function<void (juce::String scanStatus)> onScanStatusChanged;
    std::function<void (juce::String progressUpdate)> onProgressUpdateChanged;

    juce::ValueTree getValidatorResultListVT ();

    static inline const juce::Identifier Assimil8orValidatorId { "Assimil8orValidator" };
    static inline const juce::Identifier ScanStatusPropertyId     { "scanStatus" };
    static inline const juce::Identifier ProgressUpdatePropertyId { "progressUpdate" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};