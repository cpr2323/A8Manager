#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class AudioConfigProperties : public ValueTreeWrapper<AudioConfigProperties>
{
public:
    AudioConfigProperties () noexcept : ValueTreeWrapper<AudioConfigProperties> (AudioConfigTypeId) {}
    AudioConfigProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<AudioConfigProperties> (AudioConfigTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    enum PlayState
    {
        stop,
        play
    };
    void setDeviceName (juce::String deviceName, bool includeSelfCallback);
    void setPlayState (PlayState playState, bool includeSelfCallback);

    juce::String getDeviceName ();
    PlayState getPlayState ();

    std::function<void (juce::String deviceName)> onDeviceNameChange;
    std::function<void (PlayState playState)> onPlayStateChange;

    static inline const juce::Identifier AudioConfigTypeId { "AudioConfig" };
    static inline const juce::Identifier DeviceNamePropertyId { "deviceName" };
    static inline const juce::Identifier PlayStatePropertyId { "playState" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
