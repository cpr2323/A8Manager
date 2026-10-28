#pragma once

#include <JuceHeader.h>
#include "../EditManager.h"
#include "../SamplePointAdjust.h"
#include "../SampleManager/SampleManagerProperties.h"
#include "../SampleManager/SampleProperties.h"
#include "../../../../Assimil8or/Audio/AudioManager.h"
#include "../../../../Assimil8or/Audio/AuditionPlayhead.h"
#include "../../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../../Assimil8or/Preset/ZoneProperties.h"

class WaveformDisplay : public juce::Component,
                        private juce::Timer
{
public:
    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT);
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
    static constexpr int kNumMarkers { 4 };
    // the shortest loop the module allows
    static constexpr juce::int64 kMinLoopLength { 4 };

    ChannelProperties channelProperties;
    SampleManagerProperties sampleManagerProperties;
    ZoneProperties zoneProperties;
    SampleProperties sampleProperties;
    EditManager* editManager { nullptr };
    AudioManager* audioManager { nullptr };
    AuditionPlayhead* auditionPlayhead { nullptr };
    int zoneIndex { 0 };
    juce::int64 numSamples { 0 };
    juce::int64 sampleStart { 0 };
    juce::int64 sampleEnd { 0 };
    juce::int64 loopStart { 0 };
    juce::int64 loopLength { 0 };
    int halfHeight { 0 };
    int numPixels { 0 };
    float samplesPerPixel { 0.f };
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
    // where the audition playhead was last drawn, if it was
    std::optional<int> playheadX;

    void updateData ();
    bool hasSample ();
    int sampleToX (double sample);
    juce::int64 getLoopLengthInSamples ();
    juce::int64 getMarkerPosition (int markerIndex);
    juce::int64 constrainMarker (int markerIndex, juce::int64 proposedPosition, bool linked);
    void setMarker (int markerIndex, juce::int64 position, bool linked);
    SamplePointAdjust getMarkerAdjust (int markerIndex);
    juce::PopupMenu createMarkerMenu (juce::int64 clickSample);
    bool isAuditionedZone ();
    void repaintPlayhead (std::optional<int> x);

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
    void timerCallback () override;
};
