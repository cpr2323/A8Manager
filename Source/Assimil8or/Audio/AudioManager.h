#pragma once

#include <JuceHeader.h>

class AudioManager
{
public:
    // file is the sample to assign to a zone, or an invalid file if staging failed. errorMessage is set on failure, and can also be set on success
    // when there is something the user should know about (ie. the original could not be removed after converting it)
    struct StagedSample
    {
        juce::File file;
        juce::String errorMessage;
    };

    AudioManager ();

    void init (juce::ValueTree rootPropertiesVT);

    bool isAssimil8orSupportedAudioFile (const juce::File file);
    bool isA8ManagerSupportedAudioFile (const juce::File file);
    std::unique_ptr<juce::AudioFormatReader> getReaderFor (const juce::File file);
    juce::String getFileTypesList ();
    void mixStereoToMono (juce::File inputFile);
    void splitStereoIntoTwoMono (juce::File inputFile);
    // copies, or converts, sourceFile into presetFolder so it can be assigned to a zone. removeOriginal is only called for a file in presetFolder
    // that was converted to a new file name (ie. .aif to .wav)
    StagedSample stageSampleForPreset (juce::File sourceFile, juce::File presetFolder,
                                       std::function<bool (juce::File)> removeOriginal = [] (juce::File file) { return file.moveToTrash (); });
    juce::int64 findNextZeroCrossing (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side);
    juce::int64 findPreviousZeroCrossing (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side);
    juce::int64 findNextMatchingLevel (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::int64 matchSampleOffset, bool matchMovesWithStart,
                                       juce::AudioBuffer<float>& buffer, int side);
    juce::int64 findPreviousMatchingLevel (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::int64 matchSampleOffset, bool matchMovesWithStart,
                                           juce::AudioBuffer<float>& buffer, int side);
//    juce::int64 findNextZeroWaveMatching (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side);
//    juce::int64 findPreviousWaveMatching (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side);

private:
    juce::AudioFormatManager audioFormatManager;
    juce::StringArray audioFileExtensions;

    juce::int64 findMatchingLevel (juce::int64 startSampleOffset, juce::int64 limitSampleOffset, juce::int64 matchSampleOffset, bool matchMovesWithStart,
                                   juce::AudioBuffer<float>& buffer, int side, bool searchRight);

//    enum class SearchDirection { left, right };
//    float calculateSimilarity (const float* buffer, size_t start1, size_t start2, size_t window_size);
//    size_t findWaveMatchingOffset (const float* buffer, juce::int64 size, size_t currentOffset, size_t targetOffset, size_t windowSize, SearchDirection searchDirection, size_t maxDistance);
};