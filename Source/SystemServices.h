#pragma once

#include <JuceHeader.h>
#include "Assimil8or/Audio/AudioManager.h"
#include "Utility/ValueTreeWrapper.h"

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

    void setAudioManager (AudioManager* audioManager);
    AudioManager* getAudioManager ();

    void initValueTree () {}
    void processValueTree () {}

private:
};
