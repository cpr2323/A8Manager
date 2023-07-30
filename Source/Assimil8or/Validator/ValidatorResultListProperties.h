#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ValidatorResultListProperties : public ValueTreeWrapper<ValidatorResultListProperties>
{
public:
    ValidatorResultListProperties () noexcept : ValueTreeWrapper<ValidatorResultListProperties> (ValidatorResultListTypeId) {}

    juce::ValueTree addResult (juce::ValueTree validatorResultVT);
    void forEachResult (std::function<bool (juce::ValueTree validatorResultVT)> validatorResultVTCallback);
    int getNumResults ();

    static inline const juce::Identifier ValidatorResultListTypeId { "VaildatorResultList" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
