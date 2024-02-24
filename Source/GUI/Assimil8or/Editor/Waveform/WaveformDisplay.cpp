#include "WaveformDisplay.h"
#include "../../../../Utility/RuntimeRootProperties.h"
#include "../../../../Utility/DebugLog.h"

void WaveformDisplay::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
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
        samplesPerPixel = numSamples / getWidth ();

        const auto markerHandleSize { 5 };

        // draw sample start marker
        sampleStartMarkerX = 1 + (static_cast<float> (sampleStart) / static_cast<float> (numSamples) * numPixels);
        sampleStartHandle = { sampleStartMarkerX, markerStartY, markerHandleSize, markerHandleSize };

        // draw sample end marker
        sampleEndMarkerX = 1 + (static_cast<float>(sampleEnd) / static_cast<float>(numSamples) * numPixels);
        sampleEndHandle = { sampleEndMarkerX - markerHandleSize, markerStartY, markerHandleSize, markerHandleSize };

        // draw loop start marker
        loopStartMarkerX = 1 + (static_cast<float>(loopStart) / static_cast<float>(numSamples) * numPixels);
        loopStartHandle = { loopStartMarkerX, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };

        // draw loop end marker
        loopEndMarkerX = 1 + ((static_cast<float>(loopStart + static_cast<juce::int64> (loopLength))) / static_cast<float>(numSamples) * numPixels);
        loopEndHandle = { loopEndMarkerX - markerHandleSize, markerEndY - markerHandleSize, markerHandleSize, markerHandleSize };
    }
}

void WaveformDisplay::resized ()
{
    halfHeight = getHeight () / 2;
    numPixels = getWidth () - 2;
    samplesPerPixel = numSamples / getWidth ();
    markerEndY = static_cast<float>(getHeight () - 2.f);
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
        g.drawLine (sampleStartMarkerX, markerStartY, sampleStartMarkerX, markerEndY);

        // draw sample end marker
        g.fillRect (sampleEndHandle);
        g.drawLine (sampleEndMarkerX, markerStartY, sampleEndMarkerX, markerEndY);

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
        DebugLog ("WaveformDisplay", "mouseMove - handleIndex: " + juce::String (handleIndex));
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
            zoneProperties.setSampleStart (e.getPosition ().getX () * samplesPerPixel, true);
        }
        break;
        case 1:
        {
            zoneProperties.setSampleEnd (e.getPosition ().getX () * samplesPerPixel, true);
        }
        break;
        case 2:
        {
            zoneProperties.setLoopStart (e.getPosition ().getX () * samplesPerPixel, true);
        }
        break;
        case 3:
        {
            zoneProperties.setLoopLength (static_cast<double> (zoneProperties.getLoopStart().value_or (0) + (e.getPosition ().getX () * samplesPerPixel)), true);
        }
        break;
    }
}
