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
    enum class PlayState { stop, play, loop };
    void setPlayState (PlayState playState, bool includeSelfCallback);
    void setSampleSource (int channelIndex, int zoneIndex, bool includeSelfCallback);
    void setSamplePointsSelector (SamplePointsSelector samplePointsSelector, bool includeSelfCallback);
    void showConfigDialog (bool includeSelfCallback);

    PlayState getPlayState ();
    std::tuple<int, int> getSampleSource ();
    SamplePointsSelector getSSamplePointsSelector ();

    std::function<void (PlayState playState)> onPlayStateChange;
    std::function<void (std::tuple<int, int> channelAndZoneIndecies)> onSampleSourceChanged;
    std::function<void (SamplePointsSelector samplePointsSelector)> onSamplePointsSelectorChanged;
    std::function<void ()> onShowConfigDialog;

    static inline const juce::Identifier AudioConfigTypeId { "AudioPlayer" };
    static inline const juce::Identifier PlayStatePropertyId            { "playState" };
    static inline const juce::Identifier SampleSourcePropertyId         { "sampleSource" };
    static inline const juce::Identifier SamplePointsSelectorPropertyId { "samplePointsSelector" };
    static inline const juce::Identifier ShowConfigDialogPropertyId     { "showConfigDialog" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
