#pragma once

#include <JuceHeader.h>
#include "../EditManager.h"
#include "../SampleManager/SampleManagerProperties.h"
#include "../SampleManager/SampleProperties.h"
#include "../../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../../Assimil8or/Preset/ZoneProperties.h"
#include "oolib/GUI/InteractiveWaveform.h"
#include "oolib/GUI/MarkerOverlay.h"
#include "oolib/GUI/TimelineComponent.h"

//==============================================================================
/**
    WaveformDisplay - the zone's sample, its start/end and its loop points.

    The drawing, the gestures and the marker editing all come from oolib
    (WaveformView / InteractiveWaveform / MarkerOverlay / TimelineComponent).
    What lives here is only the part that is A8Manager's: which ValueTree
    properties the four markers are bound to, and the rules about where they may
    go relative to each other (including the Loop Length as Loop End mode).

    Layout is a timeline strip over the waveform, with the marker overlay on top
    of the waveform sharing its bounds - the overlay maps sample <-> pixel
    through the waveform, and the timeline is handed the same view, so all three
    stay locked together while panning and zooming.
*/
class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay ();

    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT);
    void setZone (int zoneIndex);

private:
    // Double-click anywhere on the waveform returns to the whole sample: with no
    // scrollbars it is the only way back out of a deep zoom.
    class ZoomableWaveform : public InteractiveWaveform
    {
    public:
        std::function<void ()> onDoubleClick;

        void mouseDoubleClick (const juce::MouseEvent&) override
        {
            if (onDoubleClick != nullptr)
                onDoubleClick ();
        }
    };

    // Marker list indices, in the order they are added to the overlay.
    enum MarkerIndex
    {
        kSampleStart = 0,
        kSampleEnd,
        kLoopStart,
        kLoopEnd
    };

    static constexpr int kTimelineHeight { 20 };

    ChannelProperties channelProperties;
    SampleManagerProperties sampleManagerProperties;
    ZoneProperties zoneProperties;
    SampleProperties sampleProperties;
    EditManager* editManager { nullptr };

    TimelineComponent timeline;
    ZoomableWaveform waveform;
    MarkerOverlay markerOverlay;

    bool hasSample ();
    juce::int64 getSampleLength ();
    int getDisplayChannel ();

    void setupColours ();
    void setupMarkers ();
    void updateAudioSource ();
    void updateDisplayChannel ();
    void updateMarkerPositions ();
    void publishView ();

    double constrainMarker (int markerIndex, double proposedPosition);
    void markerMoved (int markerIndex);

    void enablementChanged () override;
    void resized () override;
    void paintOverChildren (juce::Graphics& g) override;
};
