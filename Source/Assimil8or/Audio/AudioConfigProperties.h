#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class AudioConfigProperties : public ValueTreeWrapper<AudioConfigProperties>
{
public:
    AudioConfigProperties () noexcept : ValueTreeWrapper<AudioConfigProperties> (AudioConfigTypeId) {}
    AudioConfigProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<AudioConfigProperties> (AudioConfigTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void setDeviceName (juce::String deviceName, bool includeSelfCallback);
    juce::String getDeviceName ();

    std::function<void (juce::String deviceName)> onDeviceNameChange;

    static inline const juce::Identifier AudioConfigTypeId { "AudioConfig" };
    static inline const juce::Identifier DeviceNamePropertyId { "deviceName" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
