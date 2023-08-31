#pragma once

#include <JuceHeader.h>
#include "Utility/ValueTreeWrapper.h"

class AppActionProperties : public ValueTreeWrapper<AppActionProperties>
{
public:
    AppActionProperties () noexcept : ValueTreeWrapper<AppActionProperties> (AppActionPropertiesId) {}

    void setPresetLoadState (bool loading);
    bool getPresetLoadState ();

    std::function<void (bool loading)> onPresetLoadStateChange;

    static inline const juce::Identifier AppActionPropertiesId { "AppActions" };
    static inline const juce::Identifier PresetLoadStatePropertyId { "presetLoadState" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
