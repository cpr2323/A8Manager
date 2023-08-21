#pragma once

#include <JuceHeader.h>

#include "ParameterPresetListProperties.h"

class ParameterPresetsSingleton : private juce::DeletedAtShutdown
{
public:
    ParameterPresetsSingleton ()
    {
        parameterPresetListProperties.wrap ({}, ParameterPresetListProperties::WrapperType::owner, ParameterPresetListProperties::EnableCallbacks::no);
    };
    ~ParameterPresetsSingleton ()
    {
        clearSingletonInstance ();
    }

    ParameterPresetListProperties& getParameterPresetListProperties () { return parameterPresetListProperties; }

    JUCE_DECLARE_SINGLETON (ParameterPresetsSingleton, false);

private:
    ParameterPresetListProperties parameterPresetListProperties;
};
