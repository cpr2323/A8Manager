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
    void setSourceFile (juce::String sourceFile, bool includeSelfCallback);
    void setLoopStart (int startSample, bool includeSelfCallback);
    void setLoopEnd (int endSample, bool includeSelfCallback);

    juce::String getDeviceName ();
    PlayState getPlayState ();
    juce::String getSourceFile ();
    int getLoopStart ();
    int getLoopEnd ();

    std::function<void (juce::String deviceName)> onDeviceNameChange;
    std::function<void (PlayState playState)> onPlayStateChange;
    std::function<void (juce::String sourceFile)> onSourceFileChanged;
    std::function<void (int startSample)> onLoopStartChanged;
    std::function<void (int endSample)> onLoopEndChanged;

    static inline const juce::Identifier AudioConfigTypeId { "AudioConfig" };
    static inline const juce::Identifier DeviceNamePropertyId { "deviceName" };
    static inline const juce::Identifier PlayStatePropertyId  { "playState" };
    static inline const juce::Identifier SourceFilePropertyId { "sourceFile" };
    static inline const juce::Identifier LoopStartPropertyId  { "loopStart" };
    static inline const juce::Identifier LoopEndPropertyId    { "loopEnd" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
