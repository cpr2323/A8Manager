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
        const auto sampleStart { zoneProperties.getSampleStart () };
        const auto sampleEnd { zoneProperties.getSampleEnd () };
        const auto loopStart { zoneProperties.getLoopStart () };
        const auto loopLength { zoneProperties.getLoopLength () };

        juce::dsp::AudioBlock<float> audioBlock { *audioBufferPtr };
        const auto samplesPerPixel { numSamples / getWidth () };

        g.setColour (juce::Colours::black);
        auto readPtr { audioBlock.getChannelPointer (zoneProperties.getSide()) };
        for (auto pixelIndex { 0 }; pixelIndex < getWidth () - 1; ++pixelIndex)
        {
            if ((pixelIndex + 1) * samplesPerPixel < numSamples)
            {
                g.drawLine (static_cast<float> (pixelIndex),
                    static_cast<float> (static_cast<int> (halfHeight + (readPtr [pixelIndex * samplesPerPixel] * halfHeight))),
                    static_cast<float> (pixelIndex + 1),
                    static_cast<float> (static_cast<int> (halfHeight + (readPtr [(pixelIndex + 1) * samplesPerPixel] * halfHeight))));
            }
        }
    }

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds ());
}
