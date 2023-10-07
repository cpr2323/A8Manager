#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class AudioPlayerProperties : public ValueTreeWrapper<AudioPlayerProperties>
{
public:
    AudioPlayerProperties () noexcept : ValueTreeWrapper<AudioPlayerProperties> (AudioConfigTypeId) {}
    AudioPlayerProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<AudioPlayerProperties> (AudioConfigTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    enum class PlayState
    {
        stop,
        play
    };
    void setPlayState (PlayState playState, bool includeSelfCallback);
    void setSourceFile (juce::String sourceFile, bool includeSelfCallback);
    void setLoopStart (int startSample, bool includeSelfCallback);
    void setLoopEnd (int endSample, bool includeSelfCallback);

    PlayState getPlayState ();
    juce::String getSourceFile ();
    int getLoopStart ();
    int getLoopEnd ();

    std::function<void (PlayState playState)> onPlayStateChange;
    std::function<void (juce::String sourceFile)> onSourceFileChanged;
    std::function<void (int startSample)> onLoopStartChanged;
    std::function<void (int endSample)> onLoopEndChanged;

    static inline const juce::Identifier AudioConfigTypeId { "AudioPlayer" };
    static inline const juce::Identifier PlayStatePropertyId { "playState" };
    static inline const juce::Identifier SourceFilePropertyId { "sourceFile" };
    static inline const juce::Identifier LoopStartPropertyId { "loopStart" };
    static inline const juce::Identifier LoopEndPropertyId { "loopEnd" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
