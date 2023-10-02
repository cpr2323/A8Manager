#include "AudioConfigProperties.h"

void AudioConfigProperties::initValueTree ()
{
    setDeviceName ({}, false);
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
    }
}

void AudioConfigProperties::setDeviceName (juce::String fileName, bool includeSelfCallback)
{
    setValue (fileName, DeviceNamePropertyId, includeSelfCallback);
}

juce::String AudioConfigProperties::getDeviceName ()
{
    return getValue<juce::String> (DeviceNamePropertyId);
}
