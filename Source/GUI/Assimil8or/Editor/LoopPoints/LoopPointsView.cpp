#include "LoopPointsView.h"
#include "../../../Theme/A8ColourIds.h"
#include "../../../Theme/A8Fonts.h"
#include "oolib/Debug/DebugLog.h"

void LoopPointsView::setAudioBuffer (juce::AudioBuffer<float>* theAudioBuffer)
{
    audioBuffer = theAudioBuffer;
}

void LoopPointsView::setLoopPoints (juce::int64 theSampleOffset, juce::int64 theNumSamples, int theSide)
{
    sampleOffset = theSampleOffset;
    numSamples = theNumSamples;
    side = theSide;
}

void LoopPointsView::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu () && onPopupMenu != nullptr)
        onPopupMenu (e.x >= getWidth () / 2);
}

void LoopPointsView::paint (juce::Graphics& g)
{
    const auto halfWidth { getWidth () / 2 };
    const auto halfHeight { getHeight () / 2 };

    const auto area { getLocalBounds ().toFloat () };
    g.setColour (findColour (A8Colours::tunerBackground));
    g.fillRoundedRectangle (area, 2.0f);

    // the zero line is there whether or not there is audio to hang on it
    g.setColour (findColour (A8Colours::tunerDash));
    const auto dashSize { getHeight () / 11.f };
    std::array<float, 2> dashedSpec { dashSize, dashSize };
    g.drawDashedLine (juce::Line<int>{ 0, halfHeight, getWidth (), halfHeight }.toFloat (), dashedSpec.data (), 2);

    if (audioBuffer != nullptr && audioBuffer->getNumSamples () >= numSamples && numSamples > 4 && audioBuffer->getNumSamples () >= sampleOffset + numSamples)
    {
        juce::dsp::AudioBlock<float> audioBlock { *audioBuffer };
        juce::dsp::AudioBlock<float> loopSamples { audioBlock.getSubBlock (sampleOffset, numSamples) };
        const auto samplesToDisplay { static_cast<int> (std::min<juce::int64> (numSamples, halfWidth)) };

        g.setColour (findColour (A8Colours::waveformForeground));
        auto readPtr { loopSamples.getChannelPointer (side < audioBuffer->getNumChannels () ? side : 0) };
        for (auto sampleCount { 0 }; sampleCount < samplesToDisplay - 1; ++sampleCount)
        {
            // draw one line of sample going reverse from middle to left
            const auto xOffset { halfWidth - sampleCount - 1 };
            const auto sampleIndex { numSamples - sampleCount };
            g.drawLine (static_cast<float> (xOffset),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleIndex] * halfHeight))),
                        static_cast<float> (xOffset + 1),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleIndex + 1] * halfHeight))));

            // draw one line of sample start going from middle to right
            g.drawLine (static_cast<float> (halfWidth + sampleCount),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleCount] * halfHeight))),
                        static_cast<float> (halfWidth + sampleCount + 1),
                        static_cast<float> (static_cast<int> (halfHeight + (readPtr [sampleCount + 1] * halfHeight))));
        }
    }

    // where the end of the loop meets its start
    g.setColour (findColour (A8Colours::tunerDivider));
    g.fillRect (halfWidth, 0, 1, getHeight ());

    // which side is which, along the bottom either side of the divider
    constexpr auto kCaptionGap { 5 };
    constexpr auto kCaptionHeight { 11 };
    const auto captionRow { getLocalBounds ().removeFromBottom (kCaptionHeight + 1).withTrimmedBottom (1) };
    g.setFont (A8Type::caption ());
    g.setColour (findColour (A8Colours::tunerCaption));
    g.drawText ("END", captionRow.withRight (halfWidth - kCaptionGap), juce::Justification::centredRight, false);
    g.drawText ("START", captionRow.withLeft (halfWidth + kCaptionGap + 1), juce::Justification::centredLeft, false);

    g.setColour (findColour (A8Colours::outline));
    g.drawRoundedRectangle (area.reduced (0.5f), 2.0f, 1.0f);
}
