#pragma once

#include <JuceHeader.h>

class LoopPointsView : public juce::Component
{
public:
    std::vector<float> startSamples;
    std::vector<float> endSamples;

private:
    void paint (juce::Graphics& g);
};