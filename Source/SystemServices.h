#pragma once

#include <JuceHeader.h>
#include "Assimil8or/Audio/AudioManager.h"
#include "GUI/Assimil8or/Editor/EditManager.h"
#include "oolib/ValueTree/ValueTreeWrapper.h"

class SystemServices : public ValueTreeWrapper<SystemServices>
{
public:
    SystemServices () noexcept : ValueTreeWrapper (SystemServicesTypeId)
    {
    }

    SystemServices (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper (SystemServicesTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    static inline const juce::Identifier SystemServicesTypeId { "SystemServices" };
    static inline const juce::Identifier AudioManagerPropertyId { "audioManager" };
    static inline const juce::Identifier EditManagerPropertyId { "editManager" };
    static inline const juce::Identifier AudioDeviceManagerPropertyId { "audioDeviceManager" };

    void setAudioManager (AudioManager* audioManager);
    AudioManager* getAudioManager ();
    void setEditManager (EditManager* editManager);
    EditManager* getEditManager ();
    // published by the audio layer so the GUI can build a device selector without
    // the audio layer having to own one
    void setAudioDeviceManager (juce::AudioDeviceManager* audioDeviceManager);
    juce::AudioDeviceManager* getAudioDeviceManager ();

    void initValueTree () {}
    void processValueTree () {}

private:
};
