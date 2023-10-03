#include "AudioConfigProperties.h"

void AudioConfigProperties::initValueTree ()
{
    setDeviceName ({}, false);
}

void AudioConfigProperties::setDeviceName (juce::String fileName, bool includeSelfCallback)
{
    setValue (fileName, DeviceNamePropertyId, includeSelfCallback);
}

void AudioConfigProperties::setPlayState (PlayState playState, bool includeSelfCallback)
{
    setValue (static_cast<int>(playState), PlayStatePropertyId, includeSelfCallback);
}

juce::String AudioConfigProperties::getDeviceName ()
{
    return getValue<juce::String> (DeviceNamePropertyId);
}

AudioConfigProperties::PlayState AudioConfigProperties::getPlayState ()
{
    return static_cast<PlayState>(getValue<int> (PlayStatePropertyId));
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

    }
}

