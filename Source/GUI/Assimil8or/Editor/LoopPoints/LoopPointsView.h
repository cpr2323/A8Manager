#pragma once

#include <JuceHeader.h>

class LoopPointsView : public juce::Component
{
public:
    LoopPointsView (juce::AudioBuffer<float>& theAudioBuffer);

    void setLoopInfo (int64_t theSampleOffset, int64_t theNumSamples);

private:
    juce::AudioBuffer<float>& audioBuffer;
    int64_t sampleOffset { 0 };
    int64_t numSamples { 0 };

    void paint (juce::Graphics& g);
};