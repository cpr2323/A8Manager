#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class AudioPlayerProperties : public ValueTreeWrapper<AudioPlayerProperties>
{
public:
    AudioPlayerProperties () noexcept : ValueTreeWrapper<AudioPlayerProperties> (AudioConfigTypeId) {}
    AudioPlayerProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<AudioPlayerProperties> (AudioConfigTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    enum class SamplePointsSelector { SamplePoints, LoopPoints };
    // play: a voice is sounding, set by the player when it is triggered. stop: set by the player when the voice
    // ends, or by a client to stop it at once
    enum class PlayState { stop, play };
    void setPlayState (PlayState playState, bool includeSelfCallback);
    // A gate rise, or a retrigger while playing, which restarts the voice at Sample Start whatever the gate is doing.
    // The gate is the level the module sees on its gate input: Play Mode and Loop Mode decide what it does.
    void trigger (bool includeSelfCallback);
    void setGate (bool gateHigh, bool includeSelfCallback);
    void setSampleSource (int channelIndex, int zoneIndex, bool includeSelfCallback);
    void setSamplePointsSelector (SamplePointsSelector samplePointsSelector, bool includeSelfCallback);
    void showConfigDialog (bool includeSelfCallback);

    PlayState getPlayState ();
    std::tuple<int, int> getSampleSource ();
    SamplePointsSelector getSamplePointsSelector ();
    bool getGate ();

    std::function<void (PlayState playState)> onPlayStateChange;
    std::function<void (std::tuple<int, int> channelAndZoneIndecies)> onSampleSourceChanged;
    std::function<void (SamplePointsSelector samplePointsSelector)> onSamplePointsSelectorChanged;
    std::function<void ()> onTrigger;
    std::function<void (bool gateHigh)> onGateChange;
    std::function<void ()> onShowConfigDialog;

    static inline const juce::Identifier AudioConfigTypeId { "AudioPlayer" };
    static inline const juce::Identifier PlayStatePropertyId            { "playState" };
    static inline const juce::Identifier SampleSourcePropertyId         { "sampleSource" };
    static inline const juce::Identifier SamplePointsSelectorPropertyId { "samplePointsSelector" };
    static inline const juce::Identifier ShowConfigDialogPropertyId     { "showConfigDialog" };
    static inline const juce::Identifier TriggerPropertyId              { "trigger" };
    static inline const juce::Identifier GatePropertyId                 { "gate" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
