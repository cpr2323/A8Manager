#pragma once

#include <JuceHeader.h>

#include "ParameterPresetListProperties.h"

class ParameterPresetsSingleton
{
public:
    ParameterPresetsSingleton () = default;
    ~ParameterPresetsSingleton ()
    {
        clearSingletonInstance ();
    }

    ParameterPresetListProperties& getParameterPresetListProperties () { return parameterPresetListProperties; }

    JUCE_DECLARE_SINGLETON (ParameterPresetsSingleton, false);

private:
    ParameterPresetListProperties parameterPresetListProperties;
};
