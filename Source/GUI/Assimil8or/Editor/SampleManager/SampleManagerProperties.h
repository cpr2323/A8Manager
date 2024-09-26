#pragma once

#include <JuceHeader.h>
#include "../../../../Utility/ValueTreeWrapper.h"

class SampleManagerProperties : public ValueTreeWrapper<SampleManagerProperties>
{
public:
    SampleManagerProperties () noexcept : ValueTreeWrapper<SampleManagerProperties> (SampleManagerTypeId)
    {
    }
    SampleManagerProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<SampleManagerProperties> (SampleManagerTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    juce::ValueTree getSamplePropertiesVT (int channelIndex, int zoneIndex);

    static inline const juce::Identifier SampleManagerTypeId { "SampleManager" };

    void initValueTree ();
    void processValueTree () {}

private:
};
