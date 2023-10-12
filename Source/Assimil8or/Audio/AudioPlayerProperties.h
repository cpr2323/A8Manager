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
    void setLoopLength (int endSample, bool includeSelfCallback);
    void setLooping (bool isLooping, bool includeSelfCallback);
    void showConfigDialog (bool includeSelfCallback);

    PlayState getPlayState ();
    juce::String getSourceFile ();
    int getLoopStart ();
    int getLoopLength ();
    bool getLooping ();

    std::function<void (PlayState playState)> onPlayStateChange;
    std::function<void (juce::String sourceFile)> onSourceFileChanged;
    std::function<void (int loopStart)> onLoopStartChanged;
    std::function<void (int loopLength)> onLoopLengthChanged;
    std::function<void (bool isLooping)> onLoopingChanged;
    std::function<void ()> onShowConfigDialog;

    static inline const juce::Identifier AudioConfigTypeId { "AudioPlayer" };
    static inline const juce::Identifier PlayStatePropertyId        { "playState" };
    static inline const juce::Identifier LoopingPropertyId          { "isLooping" };
    static inline const juce::Identifier SourceFilePropertyId       { "sourceFile" };
    static inline const juce::Identifier LoopStartPropertyId        { "loopStart" };
    static inline const juce::Identifier LoopLengthPropertyId       { "loopLength" };
    static inline const juce::Identifier ShowConfigDialogPropertyId { "showConfigDialog" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
