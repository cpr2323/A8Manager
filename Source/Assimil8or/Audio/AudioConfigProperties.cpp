#include "AudioConfigProperties.h"

void AudioConfigProperties::initValueTree ()
{
    setDeviceName ({}, false);
    setSourceFile ({}, false);
    setLoopStart (0, false);
    setLoopEnd (0, false);
}

void AudioConfigProperties::setDeviceName (juce::String deviceName, bool includeSelfCallback)
{
    setValue (deviceName, DeviceNamePropertyId, includeSelfCallback);
}

void AudioConfigProperties::setPlayState (PlayState playState, bool includeSelfCallback)
{
    setValue (static_cast<int>(playState), PlayStatePropertyId, includeSelfCallback);
}

void AudioConfigProperties::setSourceFile (juce::String sourceFile, bool includeSelfCallback)
{
    setValue (sourceFile, SourceFilePropertyId, includeSelfCallback);
}

void AudioConfigProperties::setLoopStart (int startSample, bool includeSelfCallback)
{
    setValue (startSample, LoopStartPropertyId, includeSelfCallback);
}

void AudioConfigProperties::setLoopEnd (int endSample, bool includeSelfCallback)
{
    setValue (endSample, LoopEndPropertyId, includeSelfCallback);
}

juce::String AudioConfigProperties::getDeviceName ()
{
    return getValue<juce::String> (DeviceNamePropertyId);
}

AudioConfigProperties::PlayState AudioConfigProperties::getPlayState ()
{
    return static_cast<PlayState>(getValue<int> (PlayStatePropertyId));
}

juce::String AudioConfigProperties::getSourceFile ()
{
    return getValue<juce::String> (SourceFilePropertyId);
}

int AudioConfigProperties::getLoopStart ()
{
    return getValue<int> (LoopStartPropertyId);
}

int AudioConfigProperties::getLoopEnd ()
{
    return getValue<int> (LoopEndPropertyId);
}

void AudioConfigProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == DeviceNamePropertyId)
        {
            if (onDeviceNameChange != nullptr)
                onDeviceNameChange (getDeviceName ());
        }
        else if (property == PlayStatePropertyId)
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

