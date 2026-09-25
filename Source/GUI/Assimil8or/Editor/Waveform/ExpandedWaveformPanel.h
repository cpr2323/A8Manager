#pragma once

#include <JuceHeader.h>
#include "WaveformDisplay.h"

//==============================================================================
/**
    ExpandedWaveformPanel - the waveform popped up over the channel parameters,
    leaving the zones beside it.

    The panel covers everything left of the zones. From the top of the waveform
    down it is opaque, so the controls it covers do not show round it; the
    parameters still showing above it are washed back as a stereo right
    channel's are. It takes every click, and keeps the keyboard focus on its
    waveform, so nothing under it can be edited.

    Opening, the wash fades up first, leaving the small waveform clear, and then
    the expanded waveform appears. Closing, the waveform goes at once and the
    wash fades away. Reversing part way through turns back from wherever the
    fade has got to.
*/
class ExpandedWaveformPanel : public juce::Component,
                              private juce::Timer
{
public:
    ExpandedWaveformPanel ();

    WaveformDisplay waveformDisplay;

    // the small waveform, which the wash leaves clear while it fades, and which the expanded one opens on the view of
    void setCollapsedDisplay (WaveformDisplay* newCollapsedDisplay);
    // where the waveform sits once open, in this panel's coordinates
    void setExpandedWaveformBounds (juce::Rectangle<int> newExpandedWaveformBounds);

    // animate is false for a panel that is not on screen, which just goes straight to the end state
    void setExpanded (bool shouldBeExpanded, bool animate);
    bool isExpanded () const noexcept { return expanded; }

private:
    static constexpr double kWashMs { 110.0 };

    WaveformDisplay* collapsedDisplay { nullptr };
    juce::Rectangle<int> expandedWaveformBounds;
    bool expanded { false };
    // how far the wash has faded up, 0 to 1
    double washAmount { 0.0 };
    double lastTickMs { 0.0 };

    juce::Rectangle<int> getCollapsedBounds () const;
    void updateWaveformVisibility ();

    void timerCallback () override;
    void focusGained (FocusChangeType cause) override;
    void paint (juce::Graphics& g) override;
    void resized () override;
};
