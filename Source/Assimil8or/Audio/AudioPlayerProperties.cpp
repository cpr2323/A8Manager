#include "AudioPlayerProperties.h"

void AudioPlayerProperties::initValueTree ()
{
    setSourceFile ({}, false);
    setLoopStart (0, false);
    setLoopEnd (0, false);
}

void AudioPlayerProperties::setPlayState (PlayState playState, bool includeSelfCallback)
{
    setValue (static_cast<int>(playState), PlayStatePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setSourceFile (juce::String sourceFile, bool includeSelfCallback)
{
    setValue (sourceFile, SourceFilePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setLoopStart (int startSample, bool includeSelfCallback)
{
    setValue (startSample, LoopStartPropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setLoopEnd (int endSample, bool includeSelfCallback)
{
    setValue (endSample, LoopEndPropertyId, includeSelfCallback);
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

int AudioPlayerProperties::getLoopEnd ()
{
    return getValue<int> (LoopEndPropertyId);
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
        else if (property == LoopEndPropertyId)
        {
            if (onLoopEndChanged != nullptr)
                onLoopEndChanged (getLoopEnd ());
        }
    }
}

