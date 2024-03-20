#pragma once

#include <JuceHeader.h>
#include "../EditManager.h"
#include "../SampleManager/SampleManagerProperties.h"
#include "../SampleManager/SampleProperties.h"
#include "../../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../../Assimil8or/Preset/ZoneProperties.h"

class WaveformDisplay : public juce::Component
{
public:
    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT, EditManager* theEditManager);
    void setZone (int zoneIndex);

private:
    enum class EditHandleIndex
    {
        kNone = -1,
        kSampleStart = 0,
        kSampleEnd = 1,
        kLoopStart = 2,
        kLoopEnd = 3
    };
    ChannelProperties channelProperties;
    SampleManagerProperties sampleManagerProperties;
    ZoneProperties zoneProperties;
    SampleProperties sampleProperties;
    EditManager* editManager { nullptr };
    juce::int64 numSamples { 0 };
    juce::int64 sampleStart { 0 };
    juce::int64 sampleEnd { 0 };
    juce::int64 loopStart { 0 };
    juce::int64 loopLength { 0 };
    int halfHeight { 0 };
    int numPixels { 0 };
    int samplesPerPixel { 0 };
    int markerStartY { 1 };
    int markerEndY { 0 };
    juce::Rectangle<int> sampleStartHandle;
    juce::Rectangle<int> sampleEndHandle;
    juce::Rectangle<int> loopStartHandle;
    juce::Rectangle<int> loopEndHandle;
    std::array<float, 2> dashedSpec;
    int sampleStartMarkerX { 0 };
    int sampleEndMarkerX { 0 };
    int loopStartMarkerX { 0 };
    int loopEndMarkerX { 0 };
    EditHandleIndex handleIndex { EditHandleIndex::kNone };

    void updateData ();

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};