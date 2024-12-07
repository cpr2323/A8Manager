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
    void mixStereoToMono (juce::File inputFile);
    void splitStereoIntoTwoMono (juce::File inputFile);
    juce::int64 findNextZeroCrossing (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side);
    juce::int64 findPreviousZeroCrossing (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side);
//    juce::int64 findNextZeroWaveMatching (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side);
//    juce::int64 findPreviousWaveMatching (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side);

private:
    juce::AudioFormatManager audioFormatManager;
    juce::StringArray audioFileExtensions;

//    enum class SearchDirection { left, right };
//    float calculateSimilarity (const float* buffer, size_t start1, size_t start2, size_t window_size);
//    size_t findWaveMatchingOffset (const float* buffer, juce::int64 size, size_t currentOffset, size_t targetOffset, size_t windowSize, SearchDirection searchDirection, size_t maxDistance);
};