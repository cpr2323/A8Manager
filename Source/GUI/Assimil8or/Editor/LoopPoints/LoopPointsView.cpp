#include "LoopPointsView.h"

void LoopPointsView::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::black);
    const auto halfWidth { getWidth () / 2 };
    const auto halfHeight { getHeight () / 2 };
    if (endSamples.size () > 1)
        for (auto sampleOffset { 0 }; sampleOffset < endSamples.size () - 1 && sampleOffset < halfWidth; ++sampleOffset)
            g.drawLine (sampleOffset, static_cast<int>(halfHeight - (endSamples [sampleOffset] * getHeight ())), sampleOffset + 1, static_cast<int>(halfHeight - (endSamples [sampleOffset + 1] * getHeight ())));

    if (startSamples.size () > 1)
        for (auto sampleOffset { 0 }; sampleOffset < startSamples.size () - 1 && sampleOffset < halfWidth; ++sampleOffset)
            g.drawLine (halfWidth + sampleOffset, static_cast<int>(halfHeight - (startSamples [sampleOffset] * getHeight ())), halfWidth + sampleOffset + 1, static_cast<int>(halfHeight - (startSamples [sampleOffset + 1] * getHeight ())));

    g.drawRect (getLocalBounds ());
    g.fillRect (getWidth () / 2, 0, 1, getHeight ());
}
