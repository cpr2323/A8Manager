#include "WaveformDisplay.h"
#include "../../../Theme/A8ColourIds.h"
#include "../../../Theme/A8Fonts.h"
#include "../../../Theme/UiComponents.h"
#include "../../../../SystemServices.h"
#include "oolib/Properties/RuntimeRootProperties.h"

namespace
{
    // The Assimil8or's own loop can never be shorter than this.
    constexpr juce::int64 kMinLoopLength { 4 };

    // Vertical divisions behind the trace, fixed to the view, as a scale for the eye.
    constexpr auto kGridDivisions { 8 };

    /*
        A greyed out menu item that says why it cannot be used, in a note that comes up, as a tooltip
        would, when the pointer rests on it. JUCE's menu items have no tooltips, and the app's tooltip
        window cannot stand in: it is a window of its own, and JUCE brings an open menu back in front of
        any other window that comes forward, so it would always be under the menu. The note is drawn
        inside the menu's own window instead, as wide as the menu, just below the item - or above it,
        where the item is at the bottom of the menu.
    */
    class DisabledMenuItemWithReason : public juce::PopupMenu::CustomComponent,
                                       private juce::Timer
    {
    public:
        DisabledMenuItemWithReason (juce::String itemText, juce::String reasonText)
            : juce::PopupMenu::CustomComponent (false), text (std::move (itemText)), reasonNote (std::move (reasonText))
        {
        }

        void getIdealSize (int& idealWidth, int& idealHeight) override
        {
            getLookAndFeel ().getIdealPopupMenuItemSize (text, false, -1, idealWidth, idealHeight);
        }

        // JUCE insets a custom item by the menu's border on either side, where it draws its own items across
        // the whole slot, so this draws across the slot it sits in - its parent - to line up with them
        void paint (juce::Graphics& g) override
        {
            const auto slotBounds { getLocalBounds ().withX (-getX ()).withWidth (getParentWidth ()) };
            getLookAndFeel ().drawPopupMenuItem (g, slotBounds, false, false, false, false, false, text, {}, nullptr, nullptr);
        }

        void mouseEnter (const juce::MouseEvent&) override { startTimer (kNoteDelayMs); }
        void mouseExit (const juce::MouseEvent&) override
        {
            stopTimer ();
            reasonNote.setVisible (false);
        }

    private:
        // about the delay of the app's tooltips
        static constexpr int kNoteDelayMs { 700 };
        static constexpr int kNoteMargin { 4 };

        class ReasonNote : public juce::Component
        {
        public:
            explicit ReasonNote (juce::String reasonText) : reason (std::move (reasonText))
            {
                setInterceptsMouseClicks (false, false);
            }

            // lays the text out to the width given, and sizes the note to fit it
            void setWidthAndFit (int width)
            {
                layout = makeLayout (width - (kPaddingX * 2), findColour (juce::TooltipWindow::textColourId));
                setSize (width, juce::roundToInt (std::ceil (layout.getHeight ())) + (kPaddingY * 2));
            }

        private:
            static constexpr int kPaddingX { 6 };
            static constexpr int kPaddingY { 4 };

            juce::String reason;
            juce::TextLayout layout;

            juce::TextLayout makeLayout (int textWidth, juce::Colour colour) const
            {
                juce::AttributedString attributedText;
                attributedText.setJustification (juce::Justification::centredLeft);
                attributedText.append (reason, A8Type::body (), colour);
                juce::TextLayout newLayout;
                newLayout.createLayout (attributedText, static_cast<float> (std::max (1, textWidth)));
                return newLayout;
            }

            // in the tooltip colours, so it reads as the tooltip it stands in for
            void paint (juce::Graphics& g) override
            {
                g.fillAll (findColour (juce::TooltipWindow::backgroundColourId));
                g.setColour (findColour (juce::TooltipWindow::outlineColourId));
                g.drawRect (getLocalBounds (), 1);
                layout.draw (g, getLocalBounds ().reduced (kPaddingX, kPaddingY).toFloat ());
            }
        };

        juce::String text;
        ReasonNote reasonNote;

        void timerCallback () override
        {
            stopTimer ();
            auto* menuWindow { getTopLevelComponent () };
            if (menuWindow == nullptr || menuWindow == this)
                return;

            if (reasonNote.getParentComponent () != menuWindow)
                menuWindow->addChildComponent (reasonNote);
            reasonNote.setWidthAndFit (menuWindow->getWidth () - (kNoteMargin * 2));

            const auto itemArea { menuWindow->getLocalArea (this, getLocalBounds ()) };
            const auto fitsBelow { itemArea.getBottom () + reasonNote.getHeight () <= menuWindow->getHeight () - kNoteMargin };
            reasonNote.setTopLeftPosition (kNoteMargin, fitsBelow ? itemArea.getBottom () : itemArea.getY () - reasonNote.getHeight ());
            reasonNote.setVisible (true);
            reasonNote.toFront (false);
        }
    };
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
    // The line through the samples while it can be drawn through every one of them, and the peak
    // envelope once there are more samples than that. The waveform thins the line out past four
    // samples a pixel, so that is where it changes over: while the view shows no more than four
    // samples for each pixel of its width. A wider view - the expanded one - keeps the line over a
    // longer stretch of audio.
    constexpr auto kMaxSampleLinePerPixel { 4.0 };
    waveform.setDrawStyle (WaveformView::DrawStyle::automatic);
    waveform.setPeakEnvelopeThreshold (kMaxSampleLinePerPixel);
    // the audio outside the active pair of markers - sample start .. end, or loop start .. end - is
    // washed back, so the part being worked with, and auditioned, reads as that part
    waveform.onPaintOverlay = [this] (juce::Graphics& g, WaveformView& view)
    {
        if (! hasSample () || markerOverlay.getNumMarkers () <= kLoopEnd)
            return;
        const auto loopPointsActive { activeSamplePoints == AudioPlayerProperties::SamplePointsSelector::LoopPoints };
        const auto firstMarker { loopPointsActive ? kLoopStart : kSampleStart };
        const auto lastMarker { loopPointsActive ? kLoopEnd : kSampleEnd };
        // with no audio between the two there is no part to pick out
        if (markerOverlay.getPosition (lastMarker) <= markerOverlay.getPosition (firstMarker))
            return;
        const auto startX { view.sampleToX (markerOverlay.getPosition (firstMarker)) };
        const auto endX { view.sampleToX (markerOverlay.getPosition (lastMarker)) };
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
        resetVerticalZoom ();
        fitToView ();
    };
    waveform.onPopupMenu = [this] (double clickSample)
    {
        createToolsMenu (clickSample).showMenuAsync (juce::PopupMenu::Options ());
    };
    addAndMakeVisible (waveform);

    markerOverlay.setWaveformView (&waveform);
    markerOverlay.constrainPosition = [this] (int markerIndex, double proposedPosition) { return constrainMarker (markerIndex, proposedPosition); };
    markerOverlay.onMarkerMoved = [this] (int markerIndex) { markerMoved (markerIndex); };
    markerOverlay.formatMarkerPosition = [this] (int markerIndex, double sample)
    {
        // a loop end shown as a Loop Length is labelled with the length, as the zone's field shows it
        if (markerIndex == kLoopEnd && ! channelProperties.getLoopLengthIsEnd ())
            return timeline.formatSamplePosition (sample - markerOverlay.getPosition (kLoopStart));
        return timeline.formatSamplePosition (sample);
    };
    addAndMakeVisible (markerOverlay);

    expandButton.onClick = [this] ()
    {
        if (onExpandToggle != nullptr)
            onExpandToggle ();
    };
    addAndMakeVisible (expandButton);
    toolsButton.setTooltip ("Waveform Tools");
    toolsButton.onClick = [this] ()
    {
        createToolsMenu ({}).showMenuAsync (juce::PopupMenu::Options ().withTargetComponent (&toolsButton));
    };
    addAndMakeVisible (toolsButton);
    setExpanded (false);

    // for the marker jump keys; a click on any part of it lands here, as none of its parts take the focus
    setWantsKeyboardFocus (true);

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
    // the loop starts at gold and ends at violet. The zone editor's swatches beside
    // the four fields use the same colours.
    const std::array<int, 4> markerColourIds { A8Colours::markerStart, A8Colours::markerEnd, A8Colours::markerLoop, A8Colours::markerLoopEnd };
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

    // End's label drops a row below Start's, so the two do not run into each other when the markers are close
    auto sampleEndStyle { sampleStartStyle };
    sampleEndStyle.alignment = MarkerOverlay::HandleAlignment::leftOfLine;
    sampleEndStyle.labelOffset = kLabelStagger;
    markerOverlay.addMarker ({ "END", 0.0, sampleEndStyle });

    // Loop start / end: dashed lines, handles along the bottom.
    auto loopStartStyle { style };
    loopStartStyle.placement = MarkerOverlay::HandlePlacement::bottom;
    loopStartStyle.alignment = MarkerOverlay::HandleAlignment::rightOfLine;
    loopStartStyle.dashed    = true;
    markerOverlay.addMarker ({ "LOOP", 0.0, loopStartStyle });

    // and Loop End's rises a row above Loop Start's
    auto loopEndStyle { loopStartStyle };
    loopEndStyle.alignment = MarkerOverlay::HandleAlignment::leftOfLine;
    loopEndStyle.labelOffset = kLabelStagger;
    // named for the channel's Loop Length / Loop End setting in updateLoopEndName
    markerOverlay.addMarker ({ "LOOP END", 0.0, loopEndStyle });
}

// The fourth marker is always where the loop ends, but it is named, and labelled, the way the
// zone editor shows its value: as a Loop End, or as a Loop Length.
void WaveformDisplay::updateLoopEndName ()
{
    markerOverlay.setName (kLoopEnd, channelProperties.getLoopLengthIsEnd () ? "LOOP END" : "LOOP LENGTH");
}

void WaveformDisplay::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    channelProperties.onLoopLengthIsEndChange = [this] (bool) { updateLoopEndName (); };
    updateLoopEndName ();
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    editManager = systemServices.getEditManager ();
    audioManager = systemServices.getAudioManager ();

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
void WaveformDisplay::ZoomableWaveform::mouseDown (const juce::MouseEvent& e)
{
    // a right-click (or its Mac equivalent) without the command key is the menu; with it, the drag zooms
    menuGesture = e.mods.isPopupMenu () && ! e.mods.isCommandDown ();
    if (menuGesture)
    {
        if (onPopupMenu != nullptr && getNumSamples () > 0)
            onPopupMenu (xToSample (e.position.x));
        return;
    }
    InteractiveWaveform::mouseDown (e);
}

void WaveformDisplay::ZoomableWaveform::mouseDrag (const juce::MouseEvent& e)
{
    if (menuGesture)
        return;
    InteractiveWaveform::mouseDrag (e);
}

void WaveformDisplay::ZoomableWaveform::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu ())
        return;
    if (onDoubleClick != nullptr)
        onDoubleClick ();
}

//==============================================================================
void WaveformDisplay::setExpanded (bool isExpanded)
{
    expandButton.setGlyph (isExpanded ? ToolButton::Glyph::collapse : ToolButton::Glyph::expand);
    expandButton.setTooltip (isExpanded ? "Collapse Waveform" : "Expand Waveform");
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

void WaveformDisplay::fitToView ()
{
    waveform.zoomToFit ();
    // zoomToFit does not report a view change, so the ruler and markers are told here
    publishView ();
}

void WaveformDisplay::resetVerticalZoom ()
{
    waveform.setVerticalZoom (1.0f);
}

// the one marker at the left edge of the view, the other at the right
void WaveformDisplay::zoomToMarkers (int leftMarkerIndex, int rightMarkerIndex)
{
    if (! hasSample ())
        return;

    const auto left { markerOverlay.getPosition (leftMarkerIndex) };
    const auto right { markerOverlay.getPosition (rightMarkerIndex) };
    // setVisibleRange holds the zoom to the waveform's own limits, so a span of a few samples is as close as it goes
    waveform.setVisibleRange (left, std::max (1.0, right - left));
    publishView ();
}

// Scrolls, at the current zoom, to put the marker in the middle of the view. Near
// either end of the audio the view cannot go that far, and stops at the end instead.
void WaveformDisplay::jumpToMarker (int markerIndex)
{
    if (! hasSample () || waveform.getWidth () <= 0)
        return;

    const auto visibleSamples { waveform.getSamplesPerPixel () * static_cast<double> (waveform.getWidth ()) };
    const auto viewCentre { waveform.getVisibleStartSample () + (visibleSamples / 2.0) };
    // the waveform clamps the scroll to the audio, which is what keeps the view from running off either end
    waveform.scrollBySamples (markerOverlay.getPosition (markerIndex) - viewCentre);
    publishView ();
}

// The zero crossing search the zone editor's fields offer, from the marker to the next crossing either
// side, searching only as far as the marker is allowed to go.
void WaveformDisplay::moveMarkerToZeroCrossing (int markerIndex, bool searchRight)
{
    if (! hasSample () || ! editable || audioManager == nullptr)
        return;

    const auto position { static_cast<juce::int64> (markerOverlay.getPosition (markerIndex)) };
    auto& audioBuffer { *sampleProperties.getAudioBufferPtr () };
    const auto zeroCrossing { searchRight ? audioManager->findNextZeroCrossing (position, static_cast<juce::int64> (constrainMarker (markerIndex, static_cast<double> (getSampleLength ()))),
                                                                                audioBuffer, getDisplayChannel ())
                                          : audioManager->findPreviousZeroCrossing (position, static_cast<juce::int64> (constrainMarker (markerIndex, 0.0)),
                                                                                    audioBuffer, getDisplayChannel ()) };
    if (zeroCrossing != -1)
        setMarkerAt (markerIndex, static_cast<double> (zeroCrossing));
}

// Where a marker would land if it were set to this sample, or nothing if the rules between the markers
// would not let it go there - an item that would put a marker somewhere other than where it was
// clicked is not offered.
std::optional<double> WaveformDisplay::getMarkerPositionAt (int markerIndex, double sample)
{
    if (! hasSample () || ! editable)
        return {};

    const auto clickPosition { std::round (sample) };
    const auto position { std::clamp (constrainMarker (markerIndex, clickPosition), 0.0, static_cast<double> (getSampleLength ())) };
    if (position != clickPosition)
        return {};
    return position;
}

juce::String WaveformDisplay::getSetMarkerBlockedReason (int markerIndex, double sample)
{
    if (! hasSample ())
        return "There is no sample to set it in.";
    if (! editable)
        return "This channel is the right half of a stereo pair, so its markers are set on the left channel.";

    const auto clickPosition { std::round (sample) };
    const auto nearestPosition { std::clamp (constrainMarker (markerIndex, clickPosition), 0.0, static_cast<double> (getSampleLength ())) };
    if (nearestPosition == clickPosition)
        return {};

    const auto loopLengthIsEnd { channelProperties.getLoopLengthIsEnd () };
    const auto rule = [markerIndex, loopLengthIsEnd] () -> juce::String
    {
        switch (markerIndex)
        {
            case kSampleStart: return "Sample Start has to be before Sample End.";
            case kSampleEnd: return "Sample End has to be after Sample Start.";
            case kLoopStart:
                return loopLengthIsEnd ? "Loop Start has to be at least " + juce::String (kMinLoopLength) + " samples before Loop End."
                                       : "Loop Length moves with Loop Start, and the loop cannot run past the end of the sample.";
            case kLoopEnd:
            default:
                return "The loop has to be at least " + juce::String (kMinLoopLength) + " samples long, so it has to end at least "
                       + juce::String (kMinLoopLength) + " samples after Loop Start.";
        }
    } ();
    const auto limit { nearestPosition < clickPosition ? "The latest it can go is " : "The earliest it can go is " };
    return rule + " " + limit + timeline.formatSamplePosition (nearestPosition) + ".";
}

// As a drag that ended on the sample would: through the same rules, and written back the same way.
void WaveformDisplay::setMarkerAt (int markerIndex, double sample)
{
    const auto position { getMarkerPositionAt (markerIndex, sample) };
    if (! position.has_value ())
        return;
    markerOverlay.setPosition (markerIndex, position.value ());
    markerMoved (markerIndex);
}

juce::PopupMenu WaveformDisplay::createToolsMenu (std::optional<double> clickSample)
{
    const auto hasAudio { hasSample () };
    const auto canEdit { hasAudio && editable };
    // the fourth marker is the loop end either way, but it is named as the channel shows its value
    const auto loopLengthIsEnd { channelProperties.isValid () && channelProperties.getLoopLengthIsEnd () };
    const std::array<juce::String, 4> markerNames { "Sample Start", "Sample End", "Loop Start", loopLengthIsEnd ? "Loop End" : "Loop Length" };

    juce::PopupMenu menu;
    menu.addSectionHeader ("ZOOM");
    menu.addSeparator ();
    menu.addItem ("Reset Zoom", [this] () { resetVerticalZoom (); fitToView ();});
    menu.addItem ("Zoom To Sample Markers", hasAudio, false, [this] () { zoomToMarkers (kSampleStart, kSampleEnd); });
    menu.addItem ("Zoom To Loop Markers", hasAudio, false, [this] () { zoomToMarkers (kLoopStart, kLoopEnd); });

    menu.addSectionHeader ("JUMP TO MARKER");
    menu.addSeparator ();
    for (auto markerIndex { 0 }; markerIndex < kNumMarkers; ++markerIndex)
    {
        juce::PopupMenu::Item jumpItem { "Jump To " + markerNames [static_cast<size_t> (markerIndex)] };
        jumpItem.setEnabled (hasAudio).setAction ([this, markerIndex] () { jumpToMarker (markerIndex); });
        // the key that does the same, when the waveform has the focus
        jumpItem.shortcutKeyDescription = juce::String (markerIndex + 1);
        menu.addItem (jumpItem);
    }

    // as the zone editor's fields offer it
    menu.addSectionHeader ("ZERO CROSSING NUDGE");
    menu.addSeparator ();
    for (auto markerIndex { 0 }; markerIndex < kNumMarkers; ++markerIndex)
    {
        juce::PopupMenu zeroCrossingMenu;
        zeroCrossingMenu.addItem ("Left  <<", canEdit, false, [this, markerIndex] () { moveMarkerToZeroCrossing (markerIndex, false); });
        zeroCrossingMenu.addItem ("Right >>", canEdit, false, [this, markerIndex] () { moveMarkerToZeroCrossing (markerIndex, true); });
        menu.addSubMenu ("Nudge " + markerNames [static_cast<size_t> (markerIndex)], zeroCrossingMenu, canEdit);
    }

    // from a right-click on the waveform, the markers that can be moved to the point clicked
    if (clickSample.has_value ())
    {
        const auto sample { clickSample.value () };
        menu.addSectionHeader ("SET MARKER HERE");
        menu.addSeparator ();
        for (auto markerIndex { 0 }; markerIndex < kNumMarkers; ++markerIndex)
        {
            const auto& markerName { markerNames [static_cast<size_t> (markerIndex)] };
            if (getMarkerPositionAt (markerIndex, sample).has_value ())
            {
                menu.addItem ("Set " + markerName, [this, markerIndex, sample] () { setMarkerAt (markerIndex, sample); });
                continue;
            }
            // greyed out, with the reason in a tooltip
            juce::PopupMenu::Item blockedItem { markerName };
            blockedItem.setEnabled (false);
            blockedItem.setCustomComponent (new DisabledMenuItemWithReason ("Set " + markerName, getSetMarkerBlockedReason (markerIndex, sample)));
            menu.addItem (blockedItem);
        }
    }
    return menu;
}

//==============================================================================
// With the command key held (ctrl on Windows and Linux, cmd on the Mac) - the key that makes a right
// drag zoom - dragging a marker takes the other marker of its pair along with it, so the span between
// them keeps its length. Only a drag does this; a marker set from the menu moves on its own.
bool WaveformDisplay::isLinkedDrag (int markerIndex) const
{
    return markerOverlay.getDraggedMarker () == markerIndex && juce::ModifierKeys::getCurrentModifiers ().isCommandDown ();
}

// an unset Loop Length is a loop that runs to the end of the sample
juce::int64 WaveformDisplay::getLoopLengthInSamples ()
{
    const auto loopStart { zoneProperties.getLoopStart ().value_or (0) };
    return static_cast<juce::int64> (std::ceil (zoneProperties.getLoopLength ().value_or (static_cast<double> (getSampleLength () - loopStart))));
}

// Where a dragged marker may go, relative to the others. The overlay applies the
// audio bounds itself afterwards.
double WaveformDisplay::constrainMarker (int markerIndex, double proposedPosition)
{
    if (! hasSample ())
        return proposedPosition;

    const auto sampleLength { getSampleLength () };
    const auto position { static_cast<juce::int64> (proposedPosition) };

    // Moving a pair together, the span keeps its length, so it is the marker being carried along that
    // stops at the end of the sample.
    if (isLinkedDrag (markerIndex))
    {
        switch (markerIndex)
        {
            case kSampleStart:
            case kSampleEnd:
            {
                const auto span { zoneProperties.getSampleEnd ().value_or (sampleLength) - zoneProperties.getSampleStart ().value_or (0) };
                const auto startPosition { markerIndex == kSampleStart ? position : position - span };
                const auto clampedStart { std::clamp (startPosition, juce::int64 { 0 }, std::max (juce::int64 { 0 }, sampleLength - span)) };
                return static_cast<double> (markerIndex == kSampleStart ? clampedStart : clampedStart + span);
            }
            case kLoopStart:
            case kLoopEnd:
            {
                const auto span { getLoopLengthInSamples () };
                const auto startPosition { markerIndex == kLoopStart ? position : position - span };
                const auto clampedStart { std::clamp (startPosition, juce::int64 { 0 }, std::max (juce::int64 { 0 }, sampleLength - span)) };
                return static_cast<double> (markerIndex == kLoopStart ? clampedStart : clampedStart + span);
            }
            default:
            break;
        }
    }

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

    // moving a marker of the pair not being worked with makes it the one being worked with
    const auto movedSamplePoints { markerIndex == kSampleStart || markerIndex == kSampleEnd ? AudioPlayerProperties::SamplePointsSelector::SamplePoints
                                                                                             : AudioPlayerProperties::SamplePointsSelector::LoopPoints };
    if (movedSamplePoints != activeSamplePoints && onActiveSamplePointsRequested != nullptr)
        onActiveSamplePointsRequested (movedSamplePoints);

    const auto sampleLength { getSampleLength () };
    const auto position { static_cast<juce::int64> (markerOverlay.getPosition (markerIndex)) };
    const auto linked { isLinkedDrag (markerIndex) };
    auto setSampleStart = [this] (juce::int64 sampleStart) { zoneProperties.setSampleStart (sampleStart == 0 ? -1 : sampleStart, true); };
    auto setSampleEnd = [this, sampleLength] (juce::int64 sampleEnd) { zoneProperties.setSampleEnd (sampleEnd == sampleLength ? -1 : sampleEnd, true); };

    switch (markerIndex)
    {
        case kSampleStart:
        {
            // carrying the end along, by as far as the start moved
            const auto sampleSpan { zoneProperties.getSampleEnd ().value_or (sampleLength) - zoneProperties.getSampleStart ().value_or (0) };
            setSampleStart (position);
            if (linked)
                setSampleEnd (position + sampleSpan);
        }
        break;

        case kSampleEnd:
        {
            // carrying the start along, by as far as the end moved
            const auto sampleSpan { zoneProperties.getSampleEnd ().value_or (sampleLength) - zoneProperties.getSampleStart ().value_or (0) };
            if (linked)
                setSampleStart (position - sampleSpan);
            setSampleEnd (position);
        }
        break;

        case kLoopStart:
        {
            const auto originalLoopStart { zoneProperties.getLoopStart ().value_or (0) };
            zoneProperties.setLoopStart (position == 0 ? -1 : position, true);
            // carried together, the length is left as it is, which takes the end along with the start - as a
            // Loop Length always does
            if (channelProperties.getLoopLengthIsEnd () && ! linked)
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
            if (linked)
            {
                // the length is left as it is, and the start is moved to wherever keeps it ending here
                const auto newLoopStart { position - getLoopLengthInSamples () };
                zoneProperties.setLoopStart (newLoopStart == 0 ? -1 : newLoopStart, true);
                break;
            }
            const auto newLoopLength { static_cast<double> (position - zoneProperties.getLoopStart ().value_or (0)) };
            zoneProperties.setLoopLength (newLoopLength == static_cast<double> (sampleLength) ? -1.0 : newLoopLength, true);
        }
        break;

        default:
        break;
    }
}

//==============================================================================
void WaveformDisplay::setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector newSamplePointsSelector)
{
    if (activeSamplePoints == newSamplePointsSelector)
        return;
    activeSamplePoints = newSamplePointsSelector;
    waveform.repaint ();
}

void WaveformDisplay::setViewFrom (const WaveformDisplay& otherDisplay)
{
    if (! hasSample () || otherDisplay.waveform.getWidth () <= 0)
        return;
    waveform.setVisibleRange (otherDisplay.waveform.getVisibleStartSample (),
                              otherDisplay.waveform.getSamplesPerPixel () * static_cast<double> (waveform.getWidth ()));
    waveform.setVerticalZoom (otherDisplay.waveform.getVerticalZoom ());
    publishView ();
}

// 1 to 4 jump to the markers, in the order they are listed in the menu
bool WaveformDisplay::keyPressed (const juce::KeyPress& key)
{
    if (key.getModifiers ().isAnyModifierKeyDown ())
        return false;
    const auto character { key.getTextCharacter () };
    if (character < '1' || character >= '1' + kNumMarkers)
        return false;
    jumpToMarker (static_cast<int> (character - '1'));
    return true;
}

void WaveformDisplay::setEditable (bool isEditable)
{
    editable = isEditable;
    markerOverlay.setInterceptsMouseClicks (isEditable, false);
}

void WaveformDisplay::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    setupColours ();
}

void WaveformDisplay::resized ()
{
    auto localBounds { getLocalBounds () };

    // the tools, top down, each a square split from the next by a hairline; the
    // first sits level with the ruler
    toolColumnBounds = localBounds.removeFromLeft (kToolColumnWidth);
    localBounds.removeFromLeft (kToolColumnGap);
    auto toolColumn { toolColumnBounds.reduced (1) };
    const auto toolSize { toolColumn.getWidth () };
    expandButton.setBounds (toolColumn.removeFromTop (toolSize));
    toolColumn.removeFromTop (1);
    toolsButton.setBounds (toolColumn.removeFromTop (toolSize));

    displayBounds = localBounds;
    auto bounds { displayBounds.reduced (1) };
    timeline.setBounds (bounds.removeFromTop (juce::jmin (kTimelineHeight, bounds.getHeight () / 3)));
    waveform.setBounds (bounds);
    markerOverlay.setBounds (bounds);

    // The labels name each marker and give its position, as SquidManager's do, but they sit under the
    // handles, top and bottom, so on a short waveform the two rows would run into each other. There
    // they are shown only while their marker is being dragged.
    // the handle, the gap, the label, and the second label row the end markers' labels are staggered onto
    constexpr auto kLabelRowHeight { 12 + 3 + 16 + static_cast<int> (kLabelStagger) };
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

void WaveformDisplay::paint (juce::Graphics& g)
{
    // the column below the tools is the tools' own colour, so it reads as one strip
    g.setColour (findColour (A8Colours::panelHeader));
    g.fillRect (toolColumnBounds);
}

void WaveformDisplay::paintOverChildren (juce::Graphics& g)
{
    g.setColour (findColour (A8Colours::outline));
    g.drawRect (displayBounds);
    g.drawRect (toolColumnBounds);
    // the hairline under each tool
    for (const auto* tool : { &expandButton, &toolsButton })
        g.fillRect (tool->getX (), tool->getBottom (), tool->getWidth (), 1);
}

//==============================================================================
void WaveformDisplay::ToolButton::setGlyph (Glyph newGlyph)
{
    glyph = newGlyph;
    repaint ();
}

void WaveformDisplay::ToolButton::paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const auto enabled { isEnabled () };
    const auto hovered { enabled && (isMouseOver || isMouseDown) };
    g.fillAll (findColour (hovered ? A8Colours::hoverBackground : A8Colours::panelHeader));
    if (hovered)
    {
        g.setColour (findColour (A8Colours::accentDeep));
        g.drawRect (getLocalBounds (), 1);
    }
    g.setColour (findColour (! enabled ? A8Colours::textGhost
                                       : (hovered ? A8Colours::text : A8Colours::textDim)));

    const auto glyphBounds { getLocalBounds ().toFloat ().withSizeKeepingCentre (10.0f, 10.0f) };
    if (glyph == Glyph::gear)
    {
        A8Paint::gear (g, glyphBounds);
        return;
    }

    // an arrow off a bar: up and out of the channel view, or back down into it
    const auto pointsUp { glyph == Glyph::expand };
    const auto centreX { glyphBounds.getCentreX () };
    const auto tip { pointsUp ? glyphBounds.getY () : glyphBounds.getBottom () };
    const auto tail { pointsUp ? glyphBounds.getBottom () - 2.5f : glyphBounds.getY () + 2.5f };
    juce::Path arrow;
    arrow.addArrow ({ centreX, tail, centreX, tip }, 1.5f, 7.0f, 4.5f);
    g.fillPath (arrow);
    const auto barY { pointsUp ? glyphBounds.getBottom () - 1.0f : glyphBounds.getY () };
    g.fillRect (juce::Rectangle<float> { glyphBounds.getX (), barY, glyphBounds.getWidth (), 1.0f });
}
