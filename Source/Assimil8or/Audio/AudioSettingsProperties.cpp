#include "AudioSettingsProperties.h"

void AudioSettingsProperties::initValueTree ()
{
    setDeviceName ({}, false);
}

void AudioSettingsProperties::setDeviceName (juce::String deviceName, bool includeSelfCallback)
{
    setValue (deviceName, DeviceNamePropertyId, includeSelfCallback);
}

juce::String AudioSettingsProperties::getDeviceName ()
{
    return getValue<juce::String> (DeviceNamePropertyId);
}

void AudioSettingsProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
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

