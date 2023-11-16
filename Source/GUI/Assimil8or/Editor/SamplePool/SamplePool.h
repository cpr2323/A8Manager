#pragma once

#include <JuceHeader.h>

struct SampleData
{
public:
    enum class SampleDataStatus
    {
        wrongFormat,
        doesNotExist,
        exists
    };
    SampleData (SampleDataStatus* s, int* bps, int* nc, int64_t* lis, juce::AudioBuffer<float>* ab)
        : status (s),
          bitsPerSample (bps),
          numChannels (nc),
          lengthInSamples (lis),
          audioBuffer (ab)
    {
    }

    SampleData::SampleDataStatus getStatus () { jassert (status != nullptr); return *status; }
    int getBitsPerSample () { jassert (bitsPerSample != nullptr); return *bitsPerSample; }
    int getNumChannels () { jassert (numChannels!= nullptr); return *numChannels; }
    int64_t getLengthInSamples() { jassert (lengthInSamples != nullptr); return *lengthInSamples; }
    juce::AudioBuffer<float>& getAudioBuffer () { jassert (audioBuffer != nullptr); return *audioBuffer; }

private:
    SampleDataStatus* status { nullptr };
    int* bitsPerSample { nullptr };
    int* numChannels { nullptr };
    int64_t* lengthInSamples { nullptr };
    juce::AudioBuffer<float>* audioBuffer { nullptr };
};

class SamplePool
{
public:
    SamplePool ();
    void setParentFolder (juce::File theParentFolder);
    SampleData useSample (juce::String fileName);
    void unUseSample (juce::String fileName);
    void clear ();

private:
    juce::AudioFormatManager audioFormatManager;
    juce::File parentFolder;
    struct SampleDataInternal
    {
        SampleData::SampleDataStatus status { SampleData::SampleDataStatus::doesNotExist };
        int useCount { 0 };
        int bitsPerSample { 0 };
        int numChannels { 0 };
        int64_t lengthInSamples { 0 };
        juce::AudioBuffer<float> audioBuffer;
    };
    std::map <juce::String, SampleDataInternal> sampleList;

    SampleData loadSample (juce::String fileName);
    void updateSample (juce::String fileName);
};
