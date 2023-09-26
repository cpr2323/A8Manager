#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ParameterPresetListProperties : public ValueTreeWrapper<ParameterPresetListProperties>
{
public:
    ParameterPresetListProperties () noexcept : ValueTreeWrapper<ParameterPresetListProperties > (ParameterPresetListPropertiesTypeId) {}
    ParameterPresetListProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ParameterPresetListProperties> (ParameterPresetListPropertiesTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void forEachParameterPreset (std::function<bool (juce::ValueTree parameterPresetVT)> parameterPresetVTCallback);

    juce::ValueTree getParameterPreset (juce::String parameterPresetType);

    static inline const juce::Identifier ParameterPresetListPropertiesTypeId { "ParameterPresetList" };

    static inline const juce::String DefaultParameterPresetType { "default" };
    static inline const juce::String MinParameterPresetType     { "min" };
    static inline const juce::String MaxParameterPresetType     { "max" };

    void initValueTree ();
    void processValueTree () {}

private:
};
