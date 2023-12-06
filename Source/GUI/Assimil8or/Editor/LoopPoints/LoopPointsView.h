#pragma once

#include <JuceHeader.h>

class LoopPointsView : public juce::Component
{
public:
    void setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer);
    void setLoopPoints (juce::int64 theSampleOffset, juce::int64 theNumSamples);

private:
    juce::AudioBuffer<float>* audioBuffer { nullptr };
    juce::int64 sampleOffset { 0 };
    juce::int64 numSamples { 0 };

    void paint (juce::Graphics& g);
};