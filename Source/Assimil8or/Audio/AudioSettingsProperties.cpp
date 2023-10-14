#include "AudioSettingsProperties.h"

void AudioSettingsProperties::initValueTree ()
{
    setConfig ({}, false);
}

void AudioSettingsProperties::setConfig (juce::String config, bool includeSelfCallback)
{
    setValue (config, AudioDeviceConfigPropertyId, includeSelfCallback);
}

juce::String AudioSettingsProperties::getConfig ()
{
    return getValue<juce::String> (AudioDeviceConfigPropertyId);
}

void AudioSettingsProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == AudioDeviceConfigPropertyId)
        {
            if (onConfigChange!= nullptr)
                onConfigChange (getConfig ());
        }
    }
}

