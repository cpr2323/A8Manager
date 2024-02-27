#include "WaveformDisplay.h"
#include "../../../../Utility/RuntimeRootProperties.h"
#include "../../../../Utility/DebugLog.h"

void WaveformDisplay::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT, EditManager* theEditManager)
{
    jassert (theEditManager != nullptr);
    editManager = theEditManager;
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);
    setZone (0);
}

void WaveformDisplay::setZone (int zoneIndex)
{
    zoneProperties.wrap (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    zoneProperties.onSampleChange = [this] (juce::String) { repaint (); };
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64>) { updateData (); repaint (); };
    zoneProperties.onSampleEndChange = [this] (std::optional<juce::int64>) { updateData (); repaint (); };
    zoneProperties.onLoopStartChange = [this] (std::optional<juce::int64>) { updateData (); repaint (); };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double>) { updateData (); repaint (); };

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
        loopLength = static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (static_cast<double>(numSamples)));
        samplesPerPixel = static_cast<int> (numSamples / getWidth ());

        const auto markerHandleSize { 5 };

        // draw sample start marker
        sampleStartMarkerX = 1 + static_cast<int> ((static_cast<float> (sampleStart) / static_cast<float> (numSamples) * numPixels));
        sampleStartHandle = { sampleStartMarkerX, markerStartY, markerHandleSize, markerHandleSize };

        // draw sample end marker
        sampleEndMarkerX = 1 + static_cast<int> ((static_cast<float>(sampleEnd) / static_cast<float>(numSamples) * numPixels));
        sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };

        // draw loop start marker
        loopStartMarkerX = 1 + static_cast<int> ((static_cast<float>(loopStart) / static_cast<float>(numSamples) * numPixels));
        loopStartHandle = { loopStartMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };

        // draw loop end marker
        loopEndMarkerX = 1 + static_cast<int> (((static_cast<float>(loopStart + static_cast<juce::int64> (loopLength))) / static_cast<float>(numSamples) * numPixels));
        loopEndHandle = { loopEndMarkerX - markerHandleSize, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
    }
}

void WaveformDisplay::resized ()
{
    halfHeight = getHeight () / 2;
    numPixels = getWidth () - 2;
    samplesPerPixel = static_cast<int> (numSamples / getWidth ());
    markerEndY = getHeight () - 2;
    const auto dashSize { getHeight () / 11.f };
    dashedSpec = { dashSize, dashSize };
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey.darker (0.3f));
    g.fillRect (getLocalBounds ());

    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus() == SampleStatus::exists)
    {
        const auto audioBufferPtr { sampleProperties.getAudioBufferPtr () };
        auto readPtr { audioBufferPtr->getReadPointer (zoneProperties.getSide ()) };

        g.setColour (juce::Colours::black);
        // TODO - get proper end pixel if sample ends before end of display
        for (auto pixelIndex { 0 }; pixelIndex < numPixels - 1; ++pixelIndex)
        {
            if ((pixelIndex + 1) * samplesPerPixel < numSamples)
            {
                const auto pixelOffset { pixelIndex + 1 };
                g.drawLine (static_cast<float> (pixelOffset),
                            static_cast<float> (static_cast<int> (halfHeight + (readPtr [pixelIndex * samplesPerPixel] * halfHeight))),
                            static_cast<float> (pixelOffset + 1),
                            static_cast<float> (static_cast<int> (halfHeight + (readPtr [(pixelIndex + 1) * samplesPerPixel] * halfHeight))));
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
        g.drawLine (juce::Line<int> {sampleStartMarkerX, markerStartY, sampleStartMarkerX, markerEndY}.toFloat ());

        // draw sample end marker
        g.fillRect (sampleEndHandle);
        g.drawLine (juce::Line<int> {sampleEndMarkerX, markerStartY, sampleEndMarkerX, markerEndY}.toFloat ());

        // draw loop start marker
        g.fillRect (loopStartHandle);
        g.drawDashedLine (juce::Line<int>{ loopStartMarkerX, markerStartY, loopStartMarkerX, markerEndY }.toFloat (), dashedSpec.data (), 2);

        // draw loop end marker
        g.fillRect (loopEndHandle);
        g.drawDashedLine (juce::Line<int>{ loopEndMarkerX, markerStartY, loopEndMarkerX, markerEndY }.toFloat (), dashedSpec.data (), 2);
    }

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}

void WaveformDisplay::mouseMove (const juce::MouseEvent& e)
{
    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        if (sampleStartHandle.contains (e.getPosition ()))
            handleIndex = 0;
        else if (sampleEndHandle.contains (e.getPosition ()))
            handleIndex = 1;
        else if (loopStartHandle.contains (e.getPosition ()))
            handleIndex = 2;
        else if (loopEndHandle.contains (e.getPosition ()))
            handleIndex = 3;
        else
            handleIndex = -1;
        repaint ();
        //DebugLog ("WaveformDisplay", "mouseMove - handleIndex: " + juce::String (handleIndex));
    }
}

void WaveformDisplay::mouseDown ([[maybe_unused]] const juce::MouseEvent& e)
{
    if (handleIndex == -1)
        return;
}

void WaveformDisplay::mouseDrag (const juce::MouseEvent& e)
{
    switch (handleIndex)
    {
        case -1:
        {
            return;
        }
        break;
        case 0:
        {
            const auto newSampleStart { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleStart { std::clamp (newSampleStart, static_cast<juce::int64> (0), zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) - 1) };
            zoneProperties.setSampleStart (clampedSampleStart, true);
        }
        break;
        case 1:
        {
            const auto newSampleEnd { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedSampleEnd { std::clamp (newSampleEnd, zoneProperties.getSampleStart ().value_or (0) + 1, sampleProperties.getLengthInSamples ()) };
            zoneProperties.setSampleEnd (clampedSampleEnd, true);
        }
        break;
        case 2:
        {
            const auto originalLoopStart { zoneProperties.getLoopStart ().value_or (0) };
            const auto newLoopStart { static_cast<juce::int64> (e.getPosition ().getX () * samplesPerPixel) };
            const auto clampedLoopStart { std::clamp (newLoopStart, static_cast<juce::int64> (0), editManager->getMaxLoopStart (channelProperties.getId () - 1, zoneProperties.getId () - 1)) };
            zoneProperties.setLoopStart (clampedLoopStart, true);
            if (channelProperties.getLoopLengthIsEnd ())
            {
                // When treating Loop Length as Loop End, we need to adjust the internal storage of Loop Length by the amount Loop Start changed
                const auto lengthChangeAmount { static_cast<double> (originalLoopStart - clampedLoopStart) };
                const auto newLoopLength { zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()) + lengthChangeAmount };
                zoneProperties.setLoopLength (newLoopLength, true);
            }

        }
        break;
        case 3:
        {
            const auto newLoopLength { static_cast<double> ((e.getPosition ().getX () * samplesPerPixel) - zoneProperties.getLoopStart ().value_or (0)) };
            const auto clampedLoopLength { std::clamp (newLoopLength, 4.0, static_cast<double> (sampleProperties.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0))) };
            zoneProperties.setLoopLength (clampedLoopLength, true);
        }
        break;
    }
}
