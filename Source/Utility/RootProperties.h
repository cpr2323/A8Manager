#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

class RootProperties : public ValueTreeWrapper
{
public:
    RootProperties () noexcept : ValueTreeWrapper (RootPropertiesTypeId) {}

    static inline const juce::Identifier RootPropertiesTypeId { "Root" };

private:
    void initValueTree () override {}
};
