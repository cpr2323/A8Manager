#include "SystemServices.h"

void SystemServices::setAudioManager (AudioManager* audioManager)
{
    setValue (audioManager, AudioManagerPropertyId, false);
}

AudioManager* SystemServices::getAudioManager ()
{
    return getValue<AudioManager*> (AudioManagerPropertyId, data);
}

void SystemServices::setEditManager (EditManager* editManager)
{
    setValue (editManager, EditManagerPropertyId, false);
}

EditManager* SystemServices::getEditManager ()
{
    return getValue<EditManager*> (EditManagerPropertyId, data);
}

void SystemServices::setAudioDeviceManager (juce::AudioDeviceManager* audioDeviceManager)
{
    setValue (audioDeviceManager, AudioDeviceManagerPropertyId, false);
}

juce::AudioDeviceManager* SystemServices::getAudioDeviceManager ()
{
    return getValue<juce::AudioDeviceManager*> (AudioDeviceManagerPropertyId, data);
}
