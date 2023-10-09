#include "AudioPlayerProperties.h"

void AudioPlayerProperties::initValueTree ()
{
    setSourceFile ({}, false);
    setLoopStart (0, false);
    setLoopLength (0, false);
}

void AudioPlayerProperties::setPlayState (PlayState playState, bool includeSelfCallback)
{
    setValue (static_cast<int>(playState), PlayStatePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setSourceFile (juce::String sourceFile, bool includeSelfCallback)
{
    setValue (sourceFile, SourceFilePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setLoopStart (int loopStart, bool includeSelfCallback)
{
    setValue (loopStart , LoopStartPropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setLoopLength (int loopLength, bool includeSelfCallback)
{
    setValue (loopLength, LoopLengthPropertyId, includeSelfCallback);
}

void AudioPlayerProperties::showConfigDialog (bool includeSelfCallback)
{
    toggleValue (ShowConfigDialogPropertyId, includeSelfCallback);
}

AudioPlayerProperties::PlayState AudioPlayerProperties::getPlayState ()
{
    return static_cast<PlayState>(getValue<int> (PlayStatePropertyId));
}

juce::String AudioPlayerProperties::getSourceFile ()
{
    return getValue<juce::String> (SourceFilePropertyId);
}

int AudioPlayerProperties::getLoopStart ()
{
    return getValue<int> (LoopStartPropertyId);
}

int AudioPlayerProperties::getLoopLength ()
{
    return getValue<int> (LoopLengthPropertyId);
}

void AudioPlayerProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == PlayStatePropertyId)
        {
            if (onPlayStateChange != nullptr)
                onPlayStateChange (getPlayState ());
        }
        else if (property == SourceFilePropertyId)
        {
            if (onSourceFileChanged != nullptr)
                onSourceFileChanged (getSourceFile ());
        }
        else if (property == LoopStartPropertyId)
        {
            if (onLoopStartChanged != nullptr)
                onLoopStartChanged (getLoopStart ());
        }
        else if (property == LoopLengthPropertyId)
        {
            if (onLoopLengthChanged != nullptr)
                onLoopLengthChanged (getLoopLength ());
        }
        else if (property == ShowConfigDialogPropertyId)
        {
            if (onShowConfigDialog != nullptr)
                onShowConfigDialog ();
        }
    }
}

