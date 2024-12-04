#pragma once

#include <JuceHeader.h>

class AudioManager
{
public:
    AudioManager ();

    void init (juce::ValueTree rootPropertiesVT);

    bool isAssimil8orSupportedAudioFile (const juce::File file);
    bool isA8ManagerSupportedAudioFile (const juce::File file);
    std::unique_ptr<juce::AudioFormatReader> getReaderFor (const juce::File file);
    juce::String getFileTypesList ();

private:
    juce::AudioFormatManager audioFormatManager;
    juce::StringArray audioFileExtensions;
};