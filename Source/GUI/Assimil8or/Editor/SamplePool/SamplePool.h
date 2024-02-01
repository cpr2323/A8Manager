#pragma once

#include <JuceHeader.h>

struct SampleData
{
public:
    enum class SampleDataStatus
    {
        uninitialized,
        wrongFormat,
        doesNotExist,
        exists
    };
    SampleData () = default;
    SampleData (SampleDataStatus* s, int* bps, int* nc, juce::int64* lis, juce::AudioBuffer<float>* ab)
        : status (s),
          bitsPerSample (bps),
          numChannels (nc),
          lengthInSamples (lis),
          audioBuffer (ab)
    {
    }

    SampleData::SampleDataStatus getStatus () { return status != nullptr ? *status : SampleData::SampleDataStatus::uninitialized; }
    int getBitsPerSample () { return bitsPerSample != nullptr ? *bitsPerSample : 0; }
    int getNumChannels () { return numChannels != nullptr ? *numChannels : 0; }
    juce::int64 getLengthInSamples () { return lengthInSamples != nullptr ? *lengthInSamples: 0; }
    juce::AudioBuffer<float>* getAudioBuffer () { return audioBuffer != nullptr ? audioBuffer : &emptyAudioBuffer; }

private:
    SampleDataStatus* status { nullptr };
    int* bitsPerSample { nullptr };
    int* numChannels { nullptr };
    juce::int64* lengthInSamples { nullptr };
    juce::AudioBuffer<float>* audioBuffer { nullptr };
    juce::AudioBuffer<float> emptyAudioBuffer;
};

class SamplePool
{
public:
    SamplePool ();
    void setFolder (juce::File theParentFolder);
    SampleData open (juce::String fileName);
    void close (juce::String fileName);
    void clear ();
    void update ();
    SampleData getSampleData (juce::String fileName);

private:
    juce::AudioFormatManager audioFormatManager;
    juce::File parentFolder;
    struct SampleDataInternal
    {
        SampleData::SampleDataStatus status { SampleData::SampleDataStatus::uninitialized };
        int useCount { 0 };
        int bitsPerSample { 0 };
        int numChannels { 0 };
        juce::int64 lengthInSamples { 0 };
        juce::AudioBuffer<float> audioBuffer;
    };
    std::map <juce::String, SampleDataInternal> sampleList;

    SampleData loadSample (juce::String fileName);
    void updateSample (juce::String fileName, SampleDataInternal& sampleData);
};
