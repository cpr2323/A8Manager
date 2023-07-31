#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ValidatorResultListProperties : public ValueTreeWrapper<ValidatorResultListProperties>
{
public:
    ValidatorResultListProperties () noexcept : ValueTreeWrapper<ValidatorResultListProperties> (ValidatorResultListTypeId) {}
    ValidatorResultListProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<ValidatorResultListProperties > (ValidatorResultListTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void addResult (juce::ValueTree validatorResultVT);
    void clear ();
    void forEachResult (std::function<bool (juce::ValueTree validatorResultVT)> validatorResultVTCallback);
    int getNumResults ();

    static inline const juce::Identifier ValidatorResultListTypeId { "VaildatorResultList" };

    void initValueTree () {}
    void processValueTree () {}

private:
};
