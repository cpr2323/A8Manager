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
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64>) { repaint (); };
    zoneProperties.onSampleEndChange = [this] (std::optional<juce::int64>) { repaint (); };
    zoneProperties.onLoopStartChange = [this] (std::optional<juce::int64>) { repaint (); };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double>) { repaint (); };

    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelProperties.getId () - 1, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
    sampleProperties.onStatusChange = [this] (SampleStatus) { repaint (); };
    repaint ();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey.darker (0.3f));
    g.fillRect (getLocalBounds ());

    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus() == SampleStatus::exists)
    {
        const auto halfHeight { getHeight () / 2 };
        const auto audioBufferPtr { sampleProperties.getAudioBufferPtr () };
        const auto numSamples { sampleProperties.getLengthInSamples () };
        const auto sampleStart { zoneProperties.getSampleStart ().value_or(0) };
        const auto sampleEnd { zoneProperties.getSampleEnd ().value_or (numSamples) };
        const auto loopStart { zoneProperties.getLoopStart ().value_or(0) };
        const auto loopLength { zoneProperties.getLoopLength ().value_or(static_cast<double>(numSamples))};
        const auto numPixels { getWidth () - 2 };

        juce::dsp::AudioBlock<float> audioBlock { *audioBufferPtr };
        const auto samplesPerPixel { numSamples / getWidth () };

        g.setColour (juce::Colours::black);
        auto readPtr { audioBlock.getChannelPointer (zoneProperties.getSide()) };
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

        g.setColour (juce::Colours::white);
        // draw sample markers
        const auto sampleStartX { 1 + (static_cast<float>(sampleStart) / static_cast<float>(numSamples) * numPixels) };
        g.drawLine (sampleStartX, 0, sampleStartX, getHeight ());
        const auto sampleEndX { 1 + (static_cast<float>(sampleEnd) / static_cast<float>(numSamples) * numPixels) };
        g.drawLine (sampleEndX, 0, sampleEndX, getHeight ());

        // draw loop markers
        const auto dashSize { getHeight() / 11.f };
        const std::array<float,2> dashedSpec { dashSize, dashSize };
        const auto loopStartX { 1 + (static_cast<float>(loopStart) / static_cast<float>(numSamples) * numPixels) };
        g.drawDashedLine (juce::Line<float>{ loopStartX, 0, loopStartX, static_cast<float>(getHeight ()) }, dashedSpec.data (), 2);
        const auto loopEndX { 1 + ((static_cast<float>(loopStart + static_cast<juce::int64> (loopLength))) / static_cast<float>(numSamples) * numPixels) };
        g.drawDashedLine ({ loopEndX, 0, loopEndX, static_cast<float>(getHeight ()) }, dashedSpec.data (), 2);
    }

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}
