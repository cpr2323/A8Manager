#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

// A ValueTreeWrapper for properties that are saved to the properties file
class PersistentRootProperties : public ValueTreeWrapper<PersistentRootProperties>
{
public:
    PersistentRootProperties () noexcept : ValueTreeWrapper<PersistentRootProperties> (PersistentRootPropertiesId) {}

    juce::ValueTree addSection (juce::Identifier sectionType);
    bool removeSection (juce::Identifier sectionType);
    juce::ValueTree getSection (juce::Identifier sectionType);

    static inline const juce::Identifier PersistentRootPropertiesId { "PersistentRoot" };

    void initValueTree ();
    void processValueTree () {}

private:
};
