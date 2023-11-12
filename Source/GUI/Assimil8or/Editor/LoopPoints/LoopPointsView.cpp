#include "LoopPointsView.h"

LoopPointsView::LoopPointsView (juce::AudioBuffer<float>& theAudioBuffer)
    :audioBuffer (theAudioBuffer)
{
}

void LoopPointsView::setLoopInfo (int64_t theSampleOffset, int64_t theNumSamples)
{
    //juce::Logger::outputDebugString ("LoopPointsView::setLoopInfo: " + juce::String (theSampleOffset) + ", " + juce::String (theNumSamples));
    sampleOffset = theSampleOffset;
    numSamples = theNumSamples;
}

void LoopPointsView::paint (juce::Graphics& g)
{
    const auto halfWidth { getWidth () / 2 };
    const auto halfHeight { getHeight () / 2 };

    if (audioBuffer.getNumSamples () > numSamples && numSamples > 4)
    {
        juce::dsp::AudioBlock<float> audioBlock { audioBuffer };
        juce::dsp::AudioBlock<float> loopSamples { audioBlock.getSubBlock (sampleOffset, numSamples) };
        const auto samplesToDisplay {std::min<int>(numSamples, halfWidth)};

        g.setColour (juce::Colours::black);
        auto readPtr { loopSamples.getChannelPointer (0) };
        for (auto sampleCount { 0 }; sampleCount < samplesToDisplay - 1; ++sampleCount)
        {
            // draw one line of sample going reverse from middle to left
            const auto xOffset { halfWidth - sampleCount };
            const auto sampleIndex { numSamples - sampleCount };
            g.drawLine (xOffset, static_cast<int>(halfHeight - (readPtr [sampleIndex] * getHeight ())),
                        xOffset + 1, static_cast<int>(halfHeight - (readPtr [sampleIndex + 1] * getHeight ())));

            // draw one line of sample start going from middle to right
            g.drawLine (halfWidth + sampleCount, static_cast<int>(halfHeight - (readPtr [sampleCount] * getHeight ())),
                        halfWidth + sampleCount + 1, static_cast<int>(halfHeight - (readPtr [sampleCount + 1] * getHeight ())));
        }
    }

    g.drawRect (getLocalBounds ());
    g.fillRect (getWidth () / 2, 0, 1, getHeight ());
}
