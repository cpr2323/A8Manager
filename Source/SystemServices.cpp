#include "SystemServices.h"

void SystemServices::setAudioManager (AudioManager* audioManager)
{
    setValue (audioManager, AudioManagerPropertyId, false);
}

AudioManager* SystemServices::getAudioManager ()
{
    return getValue<AudioManager*> (AudioManagerPropertyId, data);
}
