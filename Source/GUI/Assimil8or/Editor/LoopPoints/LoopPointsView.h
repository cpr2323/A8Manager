#pragma once

#include <JuceHeader.h>

class LoopPointsView : public juce::Component
{
public:
    void setLoopInfo (juce::AudioBuffer<float>* theAudioBuffer, int64_t theSampleOffset, int64_t theNumSamples);

private:
    juce::AudioBuffer<float>* audioBuffer { nullptr };
    int64_t sampleOffset { 0 };
    int64_t numSamples { 0 };

    void paint (juce::Graphics& g);
};