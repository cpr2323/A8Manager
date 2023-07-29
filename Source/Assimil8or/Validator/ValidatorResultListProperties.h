#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ValidatorResultListProperties : public ValueTreeWrapper
{
public:
    ValidatorResultListProperties () noexcept : ValueTreeWrapper (ValidatorResultListTypeId) {}

    juce::ValueTree addResult (juce::ValueTree validatorResultVT);
    void forEachResult (std::function<bool (juce::ValueTree validatorResultVT)> validatorResultVTCallback);
    int getNumResults ();

    static inline const juce::Identifier ValidatorResultListTypeId { "VaildatorResultList" };

private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
