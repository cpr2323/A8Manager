#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

class RootProperties : public ValueTreeWrapper<RootProperties>
{
public:
    RootProperties () noexcept : ValueTreeWrapper<RootProperties> (RootPropertiesTypeId) {}

    static inline const juce::Identifier RootPropertiesTypeId { "Root" };

    void initValueTree () {}
    void processValueTree () {}

private:
};
