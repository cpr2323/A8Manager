#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class PresetManagerProperties : public ValueTreeWrapper<PresetManagerProperties>
{
public:
    PresetManagerProperties () noexcept : ValueTreeWrapper<PresetManagerProperties > (PresetManagerPropertiesTypeId) {}
    PresetManagerProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<PresetManagerProperties> (PresetManagerPropertiesTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void addPreset (juce::String presetName, juce::ValueTree presetPropertiesVT);
    juce::ValueTree getPreset (juce::String presetName);

    static inline const juce::Identifier PresetManagerPropertiesTypeId { "PresetManager" };

    static inline const juce::Identifier PresetHolderPropertiesTypeId { "PresetHolder" };
    static inline const juce::Identifier PresetHolderNamePropertyId { "name" };

    void initValueTree () {};
    void processValueTree () {}

private:
};
