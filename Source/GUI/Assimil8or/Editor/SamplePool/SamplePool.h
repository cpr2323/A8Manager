#pragma once

#include <JuceHeader.h>

class SamplePool
{
public:
    struct SampleData
    {
        const int bitsPerSample { 0 };
        const int numChannels { 0 };
        const int64_t lengthInSamples { 0 };
        const juce::AudioBuffer<float>& audioBuffer {};
    };

    SamplePool ();
    void setParentFolder (juce::File theParentFolder);
    SamplePool::SampleData useSample (juce::String fileName);
    void unUseSample (juce::String fileName);

private:
    juce::AudioFormatManager audioFormatManager;
    juce::File parentFolder;
    struct SampleDataInternal
    {
        int useCount { 0 };
        int bitsPerSample { 0 };
        int numChannels { 0 };
        int64_t lengthInSamples { 0 };
        juce::AudioBuffer<float> audioBuffer;
    };
    std::map <juce::String, SampleDataInternal> sampleList;

    SamplePool::SampleData loadSample (juce::String fileName);
};
