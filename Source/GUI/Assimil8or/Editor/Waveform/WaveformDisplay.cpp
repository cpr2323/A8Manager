#include "WaveformDisplay.h"
#include "../../../../SystemServices.h"
#include "../../../../Utility/RuntimeRootProperties.h"
#include "../../../../Utility/DebugLog.h"

void WaveformDisplay::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    editManager = systemServices.getEditManager ();
    audioManager = systemServices.getAudioManager ();
    auditionPlayhead = systemServices.getAuditionPlayhead ();

    setZone (0);
    startTimerHz (30);
}

void WaveformDisplay::setZone (int newZoneIndex)
{
    zoneIndex = newZoneIndex;
    zoneProperties.wrap (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    zoneProperties.onSampleChange = [this] (juce::String) { repaint (); };
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64>) { updateData (); repaint (); };
    zoneProperties.onSampleEndChange = [this] (std::optional<juce::int64>) { updateData (); repaint (); };
    zoneProperties.onLoopStartChange = [this] (std::optional<juce::int64>) { updateData (); repaint (); };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double>) { updateData (); repaint (); };
    zoneProperties.onSideChange = [this] (int) { updateData (); repaint (); };

    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelProperties.getId () - 1, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
    sampleProperties.onStatusChange = [this] (SampleStatus) { updateData ();  repaint (); };

    updateData ();
    repaint ();
}

void WaveformDisplay::updateData ()
{
    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        numSamples  = sampleProperties.getLengthInSamples ();
        sampleStart = zoneProperties.getSampleStart ().value_or (0);
        sampleEnd = zoneProperties.getSampleEnd ().value_or (numSamples);
        loopStart = zoneProperties.getLoopStart ().value_or (0);
        loopLength = static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (static_cast<double> (numSamples - loopStart)));
        samplesPerPixel = static_cast<float> (numSamples) / getWidth ();

        const auto markerHandleSize { 10 };

        // draw sample start marker
        sampleStartMarkerX = 1 + static_cast<int> ((static_cast<float> (sampleStart) / static_cast<float> (numSamples) * numPixels));
        sampleStartHandle = { sampleStartMarkerX, markerStartY, markerHandleSize, markerHandleSize };

        // draw sample end marker
        sampleEndMarkerX = 1 + static_cast<int> ((static_cast<float> (sampleEnd) / static_cast<float> (numSamples) * numPixels));
        sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };

        // draw loop start marker
        loopStartMarkerX = 1 + static_cast<int> ((static_cast<float> (loopStart) / static_cast<float> (numSamples) * numPixels));
        loopStartHandle = { loopStartMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };

        // draw loop end marker
        loopEndMarkerX = 1 + static_cast<int> (((static_cast<float> (loopStart + static_cast<juce::int64> (loopLength))) / static_cast<float> (numSamples) * numPixels));
        loopEndHandle = { loopEndMarkerX - markerHandleSize, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
    }
}

void WaveformDisplay::resized ()
{
    halfHeight = getHeight () / 2;
    numPixels = getWidth () - 2;
    samplesPerPixel = static_cast<float> (numSamples) / getWidth ();
    markerEndY = getHeight () - 2;
    const auto dashSize { getHeight () / 11.f };
    dashedSpec = { dashSize, dashSize };
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey.darker (0.3f));
    g.fillRect (getLocalBounds ());

    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        const auto audioBufferPtr { sampleProperties.getAudioBufferPtr () };
        const auto side { zoneProperties.getSide () };
        auto readPtr { audioBufferPtr->getReadPointer (side < sampleProperties.getNumChannels () ? side : 0) };

        g.setColour (juce::Colours::black);
        // TODO - get proper end pixel if sample ends before end of display
        for (auto pixelIndex { 0 }; pixelIndex < numPixels - 1; ++pixelIndex)
        {
            if ((pixelIndex + 1) * samplesPerPixel < numSamples)
            {
                const auto pixelOffset { pixelIndex + 1 };
                g.drawLine (static_cast<float> (pixelOffset),
                            static_cast<float> (static_cast<int> (halfHeight + (readPtr [static_cast<int> (pixelIndex * samplesPerPixel)] * halfHeight))),
                            static_cast<float> (pixelOffset + 1),
                            static_cast<float> (static_cast<int> (halfHeight + (readPtr [static_cast<int> ((pixelIndex + 1) * samplesPerPixel)] * halfHeight))));
            }
            else
            {
                break;
            }
        }

        // setup marker drawing
        g.setColour (juce::Colours::white);

        // draw sample start marker
        g.fillRect (sampleStartHandle);
        g.drawLine (juce::Line<int> { sampleStartMarkerX, markerStartY, sampleStartMarkerX, markerEndY }.toFloat ());

        // draw sample end marker
        g.fillRect (sampleEndHandle);
        g.drawLine (juce::Line<int> { sampleEndMarkerX, markerStartY, sampleEndMarkerX, markerEndY }.toFloat ());

        // draw loop start marker
        g.fillRect (loopStartHandle);
        g.drawDashedLine (juce::Line<int>{ loopStartMarkerX, markerStartY, loopStartMarkerX, markerEndY }.toFloat (), dashedSpec.data (), 2);

        // draw loop end marker
        g.fillRect (loopEndHandle);
        g.drawDashedLine (juce::Line<int>{ loopEndMarkerX, markerStartY, loopEndMarkerX, markerEndY }.toFloat (), dashedSpec.data (), 2);

        // where the audition is playing
        if (playheadX.has_value ())
        {
            g.setColour (juce::Colours::cyan);
            g.fillRect (*playheadX, 1, 1, getHeight () - 2);
        }
    }

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}

void WaveformDisplay::mouseMove (const juce::MouseEvent& e)
{
    if (! isEnabled ())
        return;

    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        if (sampleStartHandle.contains (e.getPosition ()))
            handleIndex = EditHandleIndex::kSampleStart;
        else if (sampleEndHandle.contains (e.getPosition ()))
            handleIndex = EditHandleIndex::kSampleEnd;
        else if (loopStartHandle.contains (e.getPosition ()))
            handleIndex = EditHandleIndex::kLoopStart;
        else if (loopEndHandle.contains (e.getPosition ()))
            handleIndex = EditHandleIndex::kLoopEnd;
        else
            handleIndex = EditHandleIndex::kNone;
        repaint ();
        //DebugLog ("WaveformDisplay", "mouseMove - handleIndex: " + juce::String (handleIndex));
    }
}

void WaveformDisplay::mouseDown (const juce::MouseEvent& e)
{
    if (! isEnabled () || ! hasSample ())
        return;

    if (e.mods.isPopupMenu ())
    {
        const auto clickSample { std::clamp (static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel), juce::int64 { 0 }, numSamples) };
        createMarkerMenu (clickSample).showMenuAsync ({});
    }
}

void WaveformDisplay::mouseDrag (const juce::MouseEvent& e)
{
    if (! isEnabled () || ! hasSample () || e.mods.isPopupMenu () || handleIndex == EditHandleIndex::kNone)
        return;

    // With the command key held (ctrl on Windows and Linux, cmd on the Mac), dragging a marker takes the other
    // marker of its pair along with it, so the span between them keeps its length
    const auto linked { e.mods.isCommandDown () };
    const auto markerIndex { static_cast<int> (handleIndex) };
    const auto proposedPosition { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
    setMarker (markerIndex, constrainMarker (markerIndex, proposedPosition, linked), linked);
}

bool WaveformDisplay::hasSample ()
{
    return zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists;
}

int WaveformDisplay::sampleToX (double sample)
{
    return 1 + static_cast<int> (sample / static_cast<double> (numSamples) * numPixels);
}

// an unset Loop Length is a loop that runs to the end of the sample
juce::int64 WaveformDisplay::getLoopLengthInSamples ()
{
    const auto curLoopStart { zoneProperties.getLoopStart ().value_or (0) };
    return static_cast<juce::int64> (std::ceil (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleProperties.getLengthInSamples () - curLoopStart))));
}

juce::int64 WaveformDisplay::getMarkerPosition (int markerIndex)
{
    const auto sampleLength { sampleProperties.getLengthInSamples () };
    switch (markerIndex)
    {
        case static_cast<int> (EditHandleIndex::kSampleStart): return zoneProperties.getSampleStart ().value_or (0);
        case static_cast<int> (EditHandleIndex::kSampleEnd): return zoneProperties.getSampleEnd ().value_or (sampleLength);
        case static_cast<int> (EditHandleIndex::kLoopStart): return zoneProperties.getLoopStart ().value_or (0);
        case static_cast<int> (EditHandleIndex::kLoopEnd):
        default: return zoneProperties.getLoopStart ().value_or (0) + getLoopLengthInSamples ();
    }
}

// Where a marker may go, relative to the others.
juce::int64 WaveformDisplay::constrainMarker (int markerIndex, juce::int64 position, bool linked)
{
    const auto sampleLength { sampleProperties.getLengthInSamples () };

    // Moving a pair together, the span keeps its length, so it is the marker being carried along that
    // stops at the end of the sample.
    if (linked)
    {
        const auto isSamplePair { markerIndex == static_cast<int> (EditHandleIndex::kSampleStart) || markerIndex == static_cast<int> (EditHandleIndex::kSampleEnd) };
        const auto isStart { markerIndex == static_cast<int> (EditHandleIndex::kSampleStart) || markerIndex == static_cast<int> (EditHandleIndex::kLoopStart) };
        const auto span { isSamplePair ? zoneProperties.getSampleEnd ().value_or (sampleLength) - zoneProperties.getSampleStart ().value_or (0)
                                       : getLoopLengthInSamples () };
        const auto startPosition { isStart ? position : position - span };
        const auto clampedStart { std::clamp (startPosition, juce::int64 { 0 }, std::max (juce::int64 { 0 }, sampleLength - span)) };
        return isStart ? clampedStart : clampedStart + span;
    }

    switch (markerIndex)
    {
        case static_cast<int> (EditHandleIndex::kSampleStart):
        {
            const auto curSampleEnd { zoneProperties.getSampleEnd ().value_or (sampleLength) };
            return std::clamp (position, juce::int64 { 0 }, std::max (juce::int64 { 0 }, curSampleEnd - 1));
        }
        case static_cast<int> (EditHandleIndex::kSampleEnd):
        {
            const auto curSampleStart { zoneProperties.getSampleStart ().value_or (0) };
            return std::clamp (position, std::min (curSampleStart + 1, sampleLength), sampleLength);
        }
        case static_cast<int> (EditHandleIndex::kLoopStart):
        {
            const auto maxLoopStart { editManager->getMaxLoopStart (channelProperties.getId () - 1, zoneProperties.getId () - 1) };
            return std::clamp (position, juce::int64 { 0 }, std::max (juce::int64 { 0 }, maxLoopStart));
        }
        case static_cast<int> (EditHandleIndex::kLoopEnd):
        default:
        {
            const auto curLoopStart { zoneProperties.getLoopStart ().value_or (0) };
            return std::clamp (position, std::min (curLoopStart + kMinLoopLength, sampleLength), sampleLength);
        }
    }
}

// Moves a marker to a position constrainMarker has allowed, and with linked, the other marker of its pair with it
void WaveformDisplay::setMarker (int markerIndex, juce::int64 position, bool linked)
{
    const auto sampleLength { sampleProperties.getLengthInSamples () };
    auto setSampleStart = [this] (juce::int64 newSampleStart) { zoneProperties.setSampleStart (newSampleStart == 0 ? -1 : newSampleStart, true); };
    auto setSampleEnd = [this, sampleLength] (juce::int64 newSampleEnd) { zoneProperties.setSampleEnd (newSampleEnd == sampleLength ? -1 : newSampleEnd, true); };
    // carried together, the loop keeps its length. an unset Loop Length runs to the end of the sample, so it has to
    // be set before the start moves, or the loop would change length with it
    auto holdLoopLength = [this, sampleLength] ()
    {
        if (! zoneProperties.getLoopLength ().has_value ())
            zoneProperties.setLoopLength (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)), true);
    };

    switch (markerIndex)
    {
        case static_cast<int> (EditHandleIndex::kSampleStart):
        {
            // carrying the end along, by as far as the start moved
            const auto sampleSpan { zoneProperties.getSampleEnd ().value_or (sampleLength) - zoneProperties.getSampleStart ().value_or (0) };
            setSampleStart (position);
            if (linked)
                setSampleEnd (position + sampleSpan);
        }
        break;
        case static_cast<int> (EditHandleIndex::kSampleEnd):
        {
            // carrying the start along, by as far as the end moved
            const auto sampleSpan { zoneProperties.getSampleEnd ().value_or (sampleLength) - zoneProperties.getSampleStart ().value_or (0) };
            if (linked)
                setSampleStart (position - sampleSpan);
            setSampleEnd (position);
        }
        break;
        case static_cast<int> (EditHandleIndex::kLoopStart):
        {
            if (linked)
                holdLoopLength ();
            const auto originalLoopStart { zoneProperties.getLoopStart ().value_or (0) };
            zoneProperties.setLoopStart (position == 0 ? -1 : position, true);
            if (channelProperties.getLoopLengthIsEnd () && ! linked)
            {
                // When treating Loop Length as Loop End, we need to adjust the internal storage of Loop Length by the amount Loop Start changed
                const auto lengthChangeAmount { static_cast<double> (originalLoopStart - position) };
                const auto newLoopLength { zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength)) + lengthChangeAmount };
                zoneProperties.setLoopLength (newLoopLength == static_cast<double> (sampleLength) ? -1.0 : newLoopLength, true);
            }
        }
        break;
        case static_cast<int> (EditHandleIndex::kLoopEnd):
        default:
        {
            if (linked)
            {
                // the length is left as it is, and the start is moved to wherever keeps it ending here
                holdLoopLength ();
                const auto newLoopStart { position - getLoopLengthInSamples () };
                zoneProperties.setLoopStart (newLoopStart == 0 ? -1 : newLoopStart, true);
                break;
            }
            const auto newLoopLength { static_cast<double> (position - zoneProperties.getLoopStart ().value_or (0)) };
            zoneProperties.setLoopLength (newLoopLength == static_cast<double> (sampleLength) ? -1.0 : newLoopLength, true);
        }
        break;
    }
}

// The same adjustments the zone editor's fields and loop tuner offer, searching only as far as the marker
// is allowed to go
SamplePointAdjust WaveformDisplay::getMarkerAdjust (int markerIndex)
{
    // the markers are in pairs, start then end
    const auto oppositeMarkerIndex { markerIndex ^ 1 };

    SamplePointAdjust adjust;
    adjust.audioManager = audioManager;
    adjust.getAudioBuffer = [this] () { return hasSample () ? sampleProperties.getAudioBufferPtr () : nullptr; };
    adjust.getChannel = [this] () { return zoneProperties.getSide (); };
    adjust.getPosition = [this, markerIndex] () { return getMarkerPosition (markerIndex); };
    adjust.getMinPosition = [this, markerIndex] () { return constrainMarker (markerIndex, 0, false); };
    adjust.getMaxPosition = [this, markerIndex] () { return constrainMarker (markerIndex, sampleProperties.getLengthInSamples (), false); };
    adjust.getOppositePosition = [this, oppositeMarkerIndex] () { return getMarkerPosition (oppositeMarkerIndex); };
    adjust.oppositeMovesWithPoint = markerIndex == static_cast<int> (EditHandleIndex::kLoopStart) && ! channelProperties.getLoopLengthIsEnd ();
    adjust.setPosition = [this, markerIndex] (juce::int64 sample) { setMarker (markerIndex, sample, false); };
    return adjust;
}

juce::PopupMenu WaveformDisplay::createMarkerMenu (juce::int64 clickSample)
{
    // the fourth marker is the loop end either way, but it is named as the channel shows its value
    const auto loopLengthIsEnd { channelProperties.getLoopLengthIsEnd () };
    const std::array<juce::String, kNumMarkers> markerNames { "Sample Start", "Sample End", "Loop Start", loopLengthIsEnd ? "Loop End" : "Loop Length" };
    const std::array<juce::String, kNumMarkers> matchBoundaryNames { "Sample Start to End", "Sample End to Start", "Loop Start to End", "Loop End to Start" };

    juce::PopupMenu menu;
    menu.addSectionHeader ("MOVE MARKER");
    menu.addSeparator ();
    // as the zone editor's fields and loop tuner offer them
    juce::PopupMenu zeroCrossingNudgeMenu;
    juce::PopupMenu matchBoundaryMenu;
    for (auto markerIndex { 0 }; markerIndex < kNumMarkers; ++markerIndex)
    {
        const auto adjust { getMarkerAdjust (markerIndex) };
        zeroCrossingNudgeMenu.addSubMenu ("Nudge " + markerNames [static_cast<size_t> (markerIndex)], adjust.createZeroCrossingMenu (true));
        matchBoundaryMenu.addSubMenu (matchBoundaryNames [static_cast<size_t> (markerIndex)], adjust.createMatchOppositeMenu (true));
    }
    menu.addSubMenu ("Zero Crossing Nudge", zeroCrossingNudgeMenu);
    menu.addSubMenu ("Match Opposite Boundary", matchBoundaryMenu);

    // a marker that cannot go where the click was, because of where the others are, is greyed out
    menu.addSectionHeader ("SET MARKER HERE");
    menu.addSeparator ();
    for (auto markerIndex { 0 }; markerIndex < kNumMarkers; ++markerIndex)
    {
        const auto canSet { constrainMarker (markerIndex, clickSample, false) == clickSample };
        menu.addItem ("Set " + markerNames [static_cast<size_t> (markerIndex)], canSet, false, [this, markerIndex, clickSample] ()
        {
            if (hasSample ())
                setMarker (markerIndex, constrainMarker (markerIndex, clickSample, false), false);
        });
    }
    return menu;
}

bool WaveformDisplay::isAuditionedZone ()
{
    const auto playheadChannelIndex { auditionPlayhead->channelIndex.load () };
    const auto displayChannelIndex { channelProperties.getId () - 1 };
    // a Stereo/Right channel plays along with the channel before it, on the same playhead
    const auto isPlayheadChannel { displayChannelIndex == playheadChannelIndex ||
                                   (displayChannelIndex == playheadChannelIndex + 1 && channelProperties.getChannelMode () == ChannelProperties::ChannelMode::stereoRight) };
    return isPlayheadChannel && zoneIndex == auditionPlayhead->zoneIndex.load ();
}

void WaveformDisplay::repaintPlayhead (std::optional<int> x)
{
    if (x.has_value ())
        repaint (*x - 1, 0, 3, getHeight ());
}

void WaveformDisplay::timerCallback ()
{
    std::optional<int> newPlayheadX;
    if (auditionPlayhead != nullptr && isShowing () && hasSample () && numSamples > 0)
    {
        const auto position { auditionPlayhead->position.load () };
        if (position >= 0.0 && isAuditionedZone ())
            newPlayheadX = sampleToX (position);
    }
    if (newPlayheadX == playheadX)
        return;
    repaintPlayhead (playheadX);
    playheadX = newPlayheadX;
    repaintPlayhead (playheadX);
}
