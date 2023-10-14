#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class AudioSettingsProperties : public ValueTreeWrapper<AudioSettingsProperties>
{
public:
    AudioSettingsProperties () noexcept : ValueTreeWrapper<AudioSettingsProperties> (AudioSettingsTypeId) {}
    AudioSettingsProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<AudioSettingsProperties> (AudioSettingsTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void setConfig (juce::String config, bool includeSelfCallback);
    juce::String getConfig ();

    std::function<void (juce::String config)> onConfigChange;

    static inline const juce::Identifier AudioSettingsTypeId { "AudioSettings" };
    static inline const juce::Identifier AudioDeviceConfigPropertyId { "audioDeviceConfig" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
