#include "WaveformDisplay.h"
#include "../../../Theme/A8ColourIds.h"
#include "../../../Theme/A8Fonts.h"
#include "../../../../SystemServices.h"
#include "oolib/Properties/RuntimeRootProperties.h"

namespace
{
    // The Assimil8or's own loop can never be shorter than this.
    constexpr juce::int64 kMinLoopLength { 4 };

    // Vertical divisions behind the trace, fixed to the view, as a scale for the eye.
    constexpr auto kGridDivisions { 8 };
}

WaveformDisplay::WaveformDisplay ()
{
    // Samples is what the module itself deals in, so it leads and is the
    // default; minutes:seconds is offered from the timeline's right-click menu.
    // Beats:bars is not on the list - there is no tempo here for it to mean
    // anything - which is why that menu has only two entries.
    timeline.setAvailableUnits ({ TimelineComponent::Unit::samples, TimelineComponent::Unit::timeMinutesSeconds });
    timeline.setUnit (TimelineComponent::Unit::samples);
    // Marker drag labels are formatted by the timeline, so they follow its unit.
    timeline.onUnitChanged = [this] (TimelineComponent::Unit) { markerOverlay.repaint (); };
    addAndMakeVisible (timeline);

    // Every waveform colour is the same trace colour, so there is no RMS body to
    // see - don't pay for one.
    waveform.setRmsVisible (false);
    // the audio outside sample start .. end is washed back, so the part that plays reads as the part that plays
    waveform.onPaintOverlay = [this] (juce::Graphics& g, WaveformView& view)
    {
        if (! hasSample () || markerOverlay.getNumMarkers () <= kSampleEnd)
            return;
        // with no audio between start and end there is no part that plays to pick out
        if (markerOverlay.getPosition (kSampleEnd) <= markerOverlay.getPosition (kSampleStart))
            return;
        const auto startX { view.sampleToX (markerOverlay.getPosition (kSampleStart)) };
        const auto endX { view.sampleToX (markerOverlay.getPosition (kSampleEnd)) };
        const auto height { static_cast<float> (view.getHeight ()) };
        g.setColour (findColour (A8Colours::waveformShade));
        if (startX > 0.0f)
            g.fillRect (juce::Rectangle<float> { 0.0f, 0.0f, startX, height });
        if (endX < static_cast<float> (view.getWidth ()))
            g.fillRect (juce::Rectangle<float> { endX, 0.0f, static_cast<float> (view.getWidth ()) - endX, height });
    };
    waveform.onViewChanged = [this] () { publishView (); };
    waveform.onDoubleClick = [this] ()
    {
        waveform.setVerticalZoom (1.0f);
        waveform.zoomToFit ();
        publishView ();
    };
    addAndMakeVisible (waveform);

    markerOverlay.setWaveformView (&waveform);
    markerOverlay.constrainPosition = [this] (int markerIndex, double proposedPosition) { return constrainMarker (markerIndex, proposedPosition); };
    markerOverlay.onMarkerMoved = [this] (int markerIndex) { markerMoved (markerIndex); };
    markerOverlay.formatPosition = [this] (double sample) { return timeline.formatSamplePosition (sample); };
    addAndMakeVisible (markerOverlay);

    setupMarkers ();
    setupColours ();
}

// Every colour comes from the palette, so this runs again whenever the palette changes.
void WaveformDisplay::setupColours ()
{
    // Every part of the waveform is drawn in the one trace colour.
    const auto foregroundColour { findColour (A8Colours::waveformForeground) };

    WaveformView::ColourScheme waveformColours;
    // the same lane as the zone's loop tuner, so the two read as views of the same audio
    waveformColours.background = findColour (A8Colours::tunerBackground);
    waveformColours.centreLine = findColour (A8Colours::waveformCentreLine);
    waveformColours.peak       = foregroundColour;
    waveformColours.rms        = foregroundColour;
    waveformColours.sampleLine = foregroundColour;
    waveformColours.sampleDot  = foregroundColour;
    waveform.setColourScheme (waveformColours);
    waveform.setGrid (kGridDivisions, findColour (A8Colours::waveformGrid));

    // the ruler is chrome, so it takes the chrome colours rather than the trace colour
    TimelineComponent::ColourScheme timelineColours;
    timelineColours.background = findColour (A8Colours::listBackground);
    timelineColours.majorTick  = findColour (A8Colours::menuHeaderText);
    timelineColours.minorTick  = findColour (A8Colours::textGhost);
    timelineColours.text       = findColour (A8Colours::menuHeaderText);
    timeline.setColourScheme (timelineColours);
    // a size smaller than SquidManager's, to fit the shorter ruler
    timeline.setLabelStyle ({ A8Type::ruler ().withPointHeight (8.0f), true, true });

    MarkerOverlay::Appearance markerAppearance;
    markerAppearance.handleOutline  = juce::Colours::transparentBlack;
    markerAppearance.labelFont      = A8Type::markerLabel ();
    markerAppearance.labelSeparator = ": ";
    markerAppearance.labelPlate     = juce::Colours::transparentBlack;
    markerAppearance.labelGap       = 6.0f;
    markerOverlay.setAppearance (markerAppearance);

    // Start is green and End red, the pairing that reads without being learned;
    // the loop points are gold, told apart by which side of their line the handle hangs
    const std::array<int, 4> markerColourIds { A8Colours::markerStart, A8Colours::markerEnd, A8Colours::markerLoop, A8Colours::markerLoop };
    for (auto markerIndex { 0 }; markerIndex < markerOverlay.getNumMarkers (); ++markerIndex)
    {
        auto style { markerOverlay.getStyle (markerIndex) };
        style.colour = findColour (markerColourIds [static_cast<size_t> (markerIndex)]);
        markerOverlay.setStyle (markerIndex, style);
    }
}

void WaveformDisplay::setupMarkers ()
{
    MarkerOverlay::Style style;
    style.lineThickness = 1.0f;
    style.shape         = MarkerOverlay::HandleShape::rectangle;
    style.handleWidth   = 8.0f;
    style.handleHeight  = 12.0f;
    // whether the labels are always shown depends on the room there is for them, which resized decides
    style.label         = MarkerOverlay::LabelVisibility::whileDragging;

    // In each pair the handles hang inwards, off the side of the line that faces
    // the region they bound, so which line a handle belongs to stays readable
    // when the two are close together. The colours are set in setupColours.

    // Sample start / end: solid lines, handles along the top.
    auto sampleStartStyle { style };
    sampleStartStyle.placement = MarkerOverlay::HandlePlacement::top;
    sampleStartStyle.alignment = MarkerOverlay::HandleAlignment::rightOfLine;
    markerOverlay.addMarker ({ "START", 0.0, sampleStartStyle });

    auto sampleEndStyle { sampleStartStyle };
    sampleEndStyle.alignment = MarkerOverlay::HandleAlignment::leftOfLine;
    markerOverlay.addMarker ({ "END", 0.0, sampleEndStyle });

    // Loop start / end: dashed lines, handles along the bottom.
    auto loopStartStyle { style };
    loopStartStyle.placement = MarkerOverlay::HandlePlacement::bottom;
    loopStartStyle.alignment = MarkerOverlay::HandleAlignment::rightOfLine;
    loopStartStyle.dashed    = true;
    markerOverlay.addMarker ({ "LOOP", 0.0, loopStartStyle });

    auto loopEndStyle { loopStartStyle };
    loopEndStyle.alignment = MarkerOverlay::HandleAlignment::leftOfLine;
    markerOverlay.addMarker ({ "LOOP END", 0.0, loopEndStyle });
}

void WaveformDisplay::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    editManager = systemServices.getEditManager ();

    setZone (0);
}

void WaveformDisplay::setZone (int zoneIndex)
{
    zoneProperties.wrap (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    zoneProperties.onSampleChange = [this] (juce::String) { repaint (); };
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64>) { updateMarkerPositions (); };
    zoneProperties.onSampleEndChange = [this] (std::optional<juce::int64>) { updateMarkerPositions (); };
    zoneProperties.onLoopStartChange = [this] (std::optional<juce::int64>) { updateMarkerPositions (); };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double>) { updateMarkerPositions (); };
    zoneProperties.onSideChange = [this] (int) { updateDisplayChannel (); };

    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelProperties.getId () - 1, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
    sampleProperties.onStatusChange = [this] (SampleStatus) { updateAudioSource (); };
    sampleProperties.onAudioBufferPtrChange = [this] (AudioBufferType*) { updateAudioSource (); };
    sampleProperties.onSampleRateChange = [this] (double newSampleRate) { timeline.setSampleRate (newSampleRate); };

    updateAudioSource ();
}

//==============================================================================
bool WaveformDisplay::hasSample ()
{
    return zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists;
}

juce::int64 WaveformDisplay::getSampleLength ()
{
    return hasSample () ? sampleProperties.getLengthInSamples () : 0;
}

int WaveformDisplay::getDisplayChannel ()
{
    if (! hasSample ())
        return 0;

    const auto side { zoneProperties.getSide () };
    return side < sampleProperties.getNumChannels () ? side : 0;
}

//==============================================================================
void WaveformDisplay::updateAudioSource ()
{
    // The waveform holds the buffer without owning it, and the SampleManager
    // announces an unload by clearing the status before it clears the pointer.
    // Letting go of it first means nothing here can read a buffer that has gone
    // away, and it also makes the channel change below free (there is nothing
    // left to summarise) rather than a scan of the outgoing buffer.
    waveform.setAudioBuffer (nullptr);
    waveform.setDisplayChannel (getDisplayChannel ());
    waveform.setAudioBuffer (hasSample () ? sampleProperties.getAudioBufferPtr () : nullptr);

    if (hasSample ())
        timeline.setSampleRate (sampleProperties.getSampleRate ());

    markerOverlay.setVisible (hasSample ());
    updateMarkerPositions ();
    publishView ();
}

void WaveformDisplay::updateDisplayChannel ()
{
    waveform.setDisplayChannel (getDisplayChannel ());
}

void WaveformDisplay::updateMarkerPositions ()
{
    if (! hasSample ())
        return;

    const auto sampleLength { getSampleLength () };
    const auto sampleStart { zoneProperties.getSampleStart ().value_or (0) };
    const auto sampleEnd { zoneProperties.getSampleEnd ().value_or (sampleLength) };
    const auto loopStart { zoneProperties.getLoopStart ().value_or (0) };
    const auto loopLength { static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - loopStart))) };

    markerOverlay.setPosition (kSampleStart, static_cast<double> (sampleStart));
    markerOverlay.setPosition (kSampleEnd, static_cast<double> (sampleEnd));
    markerOverlay.setPosition (kLoopStart, static_cast<double> (loopStart));
    markerOverlay.setPosition (kLoopEnd, static_cast<double> (loopStart + loopLength));
    waveform.repaint ();
}

// The timeline and the overlay both position by sample, so they have to be
// handed the waveform's view every time a gesture changes it.
void WaveformDisplay::publishView ()
{
    timeline.setView (waveform.getVisibleStartSample (), waveform.getSamplesPerPixel ());
    markerOverlay.repaint ();
}

//==============================================================================
// Where a dragged marker may go, relative to the others. The overlay applies the
// audio bounds itself afterwards.
double WaveformDisplay::constrainMarker (int markerIndex, double proposedPosition)
{
    if (! hasSample ())
        return proposedPosition;

    const auto sampleLength { getSampleLength () };
    const auto position { static_cast<juce::int64> (proposedPosition) };

    switch (markerIndex)
    {
        case kSampleStart:
        {
            const auto sampleEnd { zoneProperties.getSampleEnd ().value_or (sampleLength) };
            return static_cast<double> (std::clamp (position, juce::int64 { 0 }, std::max (juce::int64 { 0 }, sampleEnd - 1)));
        }
        case kSampleEnd:
        {
            const auto sampleStart { zoneProperties.getSampleStart ().value_or (0) };
            return static_cast<double> (std::clamp (position, std::min (sampleStart + 1, sampleLength), sampleLength));
        }
        case kLoopStart:
        {
            const auto maxLoopStart { editManager == nullptr ? sampleLength
                                                             : editManager->getMaxLoopStart (channelProperties.getId () - 1, zoneProperties.getId () - 1) };
            return static_cast<double> (std::clamp (position, juce::int64 { 0 }, std::max (juce::int64 { 0 }, maxLoopStart)));
        }
        case kLoopEnd:
        {
            const auto loopStart { zoneProperties.getLoopStart ().value_or (0) };
            return static_cast<double> (std::clamp (position, std::min (loopStart + kMinLoopLength, sampleLength), sampleLength));
        }
        default:
        {
            return proposedPosition;
        }
    }
}

// A marker was dragged; write it back to the zone. A property setter here comes
// back through the onXChange callbacks above and repositions every marker, which
// is how the ones that have to follow this one get moved.
void WaveformDisplay::markerMoved (int markerIndex)
{
    if (! hasSample ())
        return;

    const auto sampleLength { getSampleLength () };
    const auto position { static_cast<juce::int64> (markerOverlay.getPosition (markerIndex)) };

    switch (markerIndex)
    {
        case kSampleStart:
        {
            zoneProperties.setSampleStart (position == 0 ? -1 : position, true);
        }
        break;

        case kSampleEnd:
        {
            zoneProperties.setSampleEnd (position == sampleLength ? -1 : position, true);
        }
        break;

        case kLoopStart:
        {
            const auto originalLoopStart { zoneProperties.getLoopStart ().value_or (0) };
            zoneProperties.setLoopStart (position == 0 ? -1 : position, true);
            if (channelProperties.getLoopLengthIsEnd ())
            {
                // Loop Length is always stored as a length, even when it is being
                // shown as an end, so holding the end still means moving the
                // length by however far the start travelled.
                const auto lengthChangeAmount { static_cast<double> (originalLoopStart - position) };
                const auto newLoopLength { zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength)) + lengthChangeAmount };
                zoneProperties.setLoopLength (newLoopLength == static_cast<double> (sampleLength) ? -1.0 : newLoopLength, true);
            }
        }
        break;

        case kLoopEnd:
        {
            const auto newLoopLength { static_cast<double> (position - zoneProperties.getLoopStart ().value_or (0)) };
            zoneProperties.setLoopLength (newLoopLength == static_cast<double> (sampleLength) ? -1.0 : newLoopLength, true);
        }
        break;

        default:
        break;
    }
}

//==============================================================================
// Disabling the display stops it being edited, but panning, zooming and the
// timeline's unit menu only change the view, so they stay live.
void WaveformDisplay::enablementChanged ()
{
    markerOverlay.setInterceptsMouseClicks (isEnabled (), false);
}

void WaveformDisplay::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    setupColours ();
}

void WaveformDisplay::resized ()
{
    auto bounds { getLocalBounds ().reduced (1) };
    timeline.setBounds (bounds.removeFromTop (juce::jmin (kTimelineHeight, bounds.getHeight () / 3)));
    waveform.setBounds (bounds);
    markerOverlay.setBounds (bounds);

    // The labels name each marker and give its position, as SquidManager's do, but they sit under the
    // handles, top and bottom, so on a short waveform the two rows would run into each other. There
    // they are shown only while their marker is being dragged.
    constexpr auto kLabelRowHeight { 12 + 3 + 16 };
    const auto labelVisibility { bounds.getHeight () >= (kLabelRowHeight * 2) + 4 ? MarkerOverlay::LabelVisibility::always
                                                                                   : MarkerOverlay::LabelVisibility::whileDragging };
    for (auto markerIndex { 0 }; markerIndex < markerOverlay.getNumMarkers (); ++markerIndex)
    {
        auto style { markerOverlay.getStyle (markerIndex) };
        if (style.label == labelVisibility)
            continue;
        style.label = labelVisibility;
        markerOverlay.setStyle (markerIndex, style);
    }

    publishView ();
}

void WaveformDisplay::paintOverChildren (juce::Graphics& g)
{
    g.setColour (findColour (A8Colours::outline));
    g.drawRect (getLocalBounds ());
}
