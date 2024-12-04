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
    juce::int64 findNextZeroCrossing (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side);
    juce::int64 findPreviousZeroCrossing (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side);

private:
    juce::AudioFormatManager audioFormatManager;
    juce::StringArray audioFileExtensions;
};