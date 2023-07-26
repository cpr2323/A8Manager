#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class Assimil8orPresetProperties : public ValueTreeWrapper
{
public:
    Assimil8orPresetProperties () noexcept : ValueTreeWrapper (Assimil8orPresetTypeId) {}

    static inline const juce::Identifier Assimil8orPresetTypeId { "Preset" };
    static inline const juce::Identifier PresetNamePropertyId { "name" };

    static inline const juce::Identifier ChannelTypeId { "Channel" };

    static inline const juce::Identifier ZoneTypeId { "Zone" };
private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};