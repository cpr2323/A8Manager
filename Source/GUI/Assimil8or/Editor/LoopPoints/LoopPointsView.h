#pragma once

#include <JuceHeader.h>

// TODO - refactor to take a ZoneProperties VT and get the data from there
//        Will just need an function to set whether to use Sample or Loop points
class LoopPointsView : public juce::Component
{
public:
    void setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer);
    void setLoopPoints (juce::int64 theSampleOffset, juce::int64 theNumSamples, int theSide);

private:
    juce::AudioBuffer<float>* audioBuffer { nullptr };
    juce::int64 sampleOffset { 0 };
    juce::int64 numSamples { 0 };
    int side { 0 };

    void paint (juce::Graphics& g);
};