#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class AudioSettingsProperties : public ValueTreeWrapper<AudioSettingsProperties>
{
public:
    AudioSettingsProperties () noexcept : ValueTreeWrapper<AudioSettingsProperties> (AudioConfigTypeId) {}
    AudioSettingsProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<AudioSettingsProperties> (AudioConfigTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void setDeviceName (juce::String deviceName, bool includeSelfCallback);
    juce::String getDeviceName ();

    std::function<void (juce::String deviceName)> onDeviceNameChange;

    static inline const juce::Identifier AudioConfigTypeId { "AudioSettings" };
    static inline const juce::Identifier DeviceNamePropertyId { "deviceName" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
