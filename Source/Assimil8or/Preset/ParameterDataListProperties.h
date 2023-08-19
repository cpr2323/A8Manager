#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

#define LOG_VALIDATE_PARAMETERS 0
#if LOG_VALIDATE_PARAMETERS
#define LogValidateParameters(text) juce::Logger::outputDebugString (text);
#else
#define LogValidateParameters(text) ;
#endif

class ParameterDataListProperties : public ValueTreeWrapper<ParameterDataListProperties>
{
public:
    ParameterDataListProperties () noexcept : ValueTreeWrapper<ParameterDataListProperties> (Assimil8orParameterValuesTypeId) {}
    ParameterDataListProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ParameterDataListProperties> (Assimil8orParameterValuesTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void forEachParameter (juce::Identifier sectionTypeId, std::function<bool (juce::ValueTree parameterVT)> parameterVTCallback);

    juce::ValueTree getParameter (juce::Identifier sectionTypeId, juce::String parameterName);

    static inline const juce::Identifier Assimil8orParameterValuesTypeId { "Assimil8orParameterValues" };
    static inline const juce::Identifier PresetSectionTypeId { "Preset" };
    static inline const juce::Identifier ChannelSectionTypeId { "Channel" };
    static inline const juce::Identifier ZoneSectionTypeId { "Zone" };
    static inline const juce::Identifier ParameterTypeId { "Parameter" };

    void initValueTree ();
    void processValueTree ()
    {
#if JUCE_DEBUG
        validateParameterData ();
#endif
    }

private:
    void validateParameterData ();
};
