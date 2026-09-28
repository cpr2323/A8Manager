#pragma once

#include <JuceHeader.h>
#include "../EditManager.h"
#include "../SamplePointAdjust.h"
#include "../SampleManager/SampleManagerProperties.h"
#include "../SampleManager/SampleProperties.h"
#include "../../../../Assimil8or/Audio/AudioManager.h"
#include "../../../../Assimil8or/Audio/AudioPlayerProperties.h"
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

    Layout is a column of tools down the left, then a timeline strip over the
    waveform, with the marker overlay on top of the waveform sharing its bounds -
    the overlay maps sample <-> pixel through the waveform, and the timeline is
    handed the same view, so all three stay locked together while panning and
    zooming.

    The first tool expands the display over the whole channel view, or puts it
    away again; the owner does the expanding, through onExpandToggle, and says
    which of the two this display is with setExpanded. The second is the
    waveform menu: the view resets, zooms to the sample or loop, and jumps to
    each marker. Right-clicking the waveform brings up the same menu, with the
    markers that can be set to the point clicked added to it. With the keyboard
    focus, which a click anywhere on it gives it, the keys 1 to 4 jump to the
    four markers.
*/
class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay ();

    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT);
    void setZone (int zoneIndex);

    // whether this is the display popped up over the channel view, which turns the expand tool into collapse
    void setExpanded (bool isExpanded);
    std::function<void ()> onExpandToggle;

    // Whether the markers can be dragged. The view itself - panning, zooming, the
    // tools and the timeline's unit menu - is live either way, as none of it edits.
    void setEditable (bool isEditable);

    // the same stretch of audio, at the same zoom, as another display is showing
    void setViewFrom (const WaveformDisplay& otherDisplay);

    // The pair of markers the zone is working with: the audio between them is shown at full strength,
    // and everything outside them is washed back. Moving a marker of the other pair asks, through
    // onActiveSamplePointsRequested, for that pair to become the active one; the owner answers by
    // calling setActiveSamplePoints.
    void setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector newSamplePointsSelector);
    std::function<void (AudioPlayerProperties::SamplePointsSelector samplePointsSelector)> onActiveSamplePointsRequested;

private:
    /*
        InteractiveWaveform's gestures, with two changes. The right button is the
        menu, so zooming by drag takes the command key with it (ctrl on Windows
        and Linux, cmd on the Mac). And double-click anywhere returns to the whole
        sample: with no scrollbars it is the only way back out of a deep zoom.
    */
    class ZoomableWaveform : public InteractiveWaveform
    {
    public:
        std::function<void ()> onDoubleClick;
        std::function<void (double clickSample)> onPopupMenu;

        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;
        void mouseDoubleClick (const juce::MouseEvent&) override;

    private:
        // the gesture that started with the menu is not also a drag
        bool menuGesture { false };
    };

    /*
        A tool in the column beside the waveform: a drawn glyph on the header
        colour, lifting under the pointer as SquidManager's cue tools do.
    */
    class ToolButton : public juce::Button
    {
    public:
        enum class Glyph { expand, collapse, gear };

        explicit ToolButton (Glyph theGlyph) : juce::Button ({}), glyph (theGlyph) {}
        void setGlyph (Glyph newGlyph);

    private:
        Glyph glyph;

        void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
    };

    // Marker list indices, in the order they are added to the overlay.
    enum MarkerIndex
    {
        kSampleStart = 0,
        kSampleEnd,
        kLoopStart,
        kLoopEnd,
        kNumMarkers
    };

    static constexpr int kTimelineHeight { 16 };
    // how far the end markers' labels sit from the start markers' - one row of label text
    static constexpr float kLabelStagger { 12.0f };
    // the tool column, including its border, and the gap between it and the waveform
    static constexpr int kToolColumnWidth { 18 };
    static constexpr int kToolColumnGap { 3 };

    ChannelProperties channelProperties;
    SampleManagerProperties sampleManagerProperties;
    ZoneProperties zoneProperties;
    SampleProperties sampleProperties;
    EditManager* editManager { nullptr };
    AudioManager* audioManager { nullptr };
    bool editable { true };
    AudioPlayerProperties::SamplePointsSelector activeSamplePoints { AudioPlayerProperties::SamplePointsSelector::SamplePoints };

    TimelineComponent timeline;
    ZoomableWaveform waveform;
    MarkerOverlay markerOverlay;
    ToolButton expandButton { ToolButton::Glyph::expand };
    ToolButton toolsButton { ToolButton::Glyph::gear };

    // set in resized, outlined in paintOverChildren
    juce::Rectangle<int> toolColumnBounds;
    juce::Rectangle<int> displayBounds;

    bool hasSample ();
    juce::int64 getSampleLength ();
    int getDisplayChannel ();

    void setupColours ();
    void setupMarkers ();
    void updateLoopEndName ();
    void updateAudioSource ();
    void updateDisplayChannel ();
    void updateMarkerPositions ();
    void publishView ();

    void fitToView ();
    void resetVerticalZoom ();
    void zoomToMarkers (int leftMarkerIndex, int rightMarkerIndex);
    void jumpToMarker (int markerIndex);
    SamplePointAdjust getMarkerAdjust (int markerIndex);
    std::optional<double> getMarkerPositionAt (int markerIndex, double sample);
    // why a marker cannot be set to this sample, for the tooltip on its greyed out menu item
    juce::String getSetMarkerBlockedReason (int markerIndex, double sample);
    void setMarkerAt (int markerIndex, double sample);
    // clickSample is where the waveform was right-clicked, and adds the markers that can be set there
    juce::PopupMenu createToolsMenu (std::optional<double> clickSample);

    bool isLinkedDrag (int markerIndex) const;
    juce::int64 getLoopLengthInSamples ();
    double constrainMarker (int markerIndex, double proposedPosition);
    void markerMoved (int markerIndex);

    void lookAndFeelChanged () override;
    bool keyPressed (const juce::KeyPress& key) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
};
