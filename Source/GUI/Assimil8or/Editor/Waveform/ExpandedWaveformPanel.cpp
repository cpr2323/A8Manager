#include "ExpandedWaveformPanel.h"
#include "../../../Theme/A8ColourIds.h"

ExpandedWaveformPanel::ExpandedWaveformPanel ()
{
    setWantsKeyboardFocus (true);
    waveformDisplay.setExpanded (true);
    addChildComponent (waveformDisplay);
}

void ExpandedWaveformPanel::setCollapsedDisplay (WaveformDisplay* newCollapsedDisplay)
{
    collapsedDisplay = newCollapsedDisplay;
}

void ExpandedWaveformPanel::setExpandedWaveformBounds (juce::Rectangle<int> newExpandedWaveformBounds)
{
    expandedWaveformBounds = newExpandedWaveformBounds;
    waveformDisplay.setBounds (expandedWaveformBounds);
    repaint ();
}

void ExpandedWaveformPanel::setExpanded (bool shouldBeExpanded, bool animate)
{
    if (shouldBeExpanded == expanded)
        return;
    expanded = shouldBeExpanded;

    if (expanded && ! isVisible ())
    {
        // starting from closed: the waveform opens on the small one's view
        washAmount = 0.0;
        if (collapsedDisplay != nullptr)
            waveformDisplay.setViewFrom (*collapsedDisplay);
        setVisible (true);
    }

    if (! animate)
    {
        stopTimer ();
        washAmount = expanded ? 1.0 : 0.0;
        updateWaveformVisibility ();
        setVisible (expanded);
        return;
    }

    // until the waveform shows, the panel holds the focus itself, so nothing under it keeps it
    if (expanded)
        grabKeyboardFocus ();
    updateWaveformVisibility ();
    lastTickMs = juce::Time::getMillisecondCounterHiRes ();
    startTimerHz (60);
}

void ExpandedWaveformPanel::timerCallback ()
{
    const auto nowMs { juce::Time::getMillisecondCounterHiRes () };
    const auto step { (nowMs - lastTickMs) / kWashMs };
    lastTickMs = nowMs;

    washAmount = expanded ? std::min (1.0, washAmount + step) : std::max (0.0, washAmount - step);
    updateWaveformVisibility ();
    repaint ();

    if (washAmount == (expanded ? 1.0 : 0.0))
    {
        stopTimer ();
        if (! expanded)
            setVisible (false);
    }
}

// the waveform is only there once the wash is all the way up, and goes as soon as closing starts
void ExpandedWaveformPanel::updateWaveformVisibility ()
{
    const auto showWaveform { expanded && washAmount == 1.0 };
    if (showWaveform == waveformDisplay.isVisible ())
        return;
    waveformDisplay.setVisible (showWaveform);
    if (showWaveform && isShowing ())
        waveformDisplay.grabKeyboardFocus ();
    repaint ();
}

juce::Rectangle<int> ExpandedWaveformPanel::getCollapsedBounds () const
{
    if (collapsedDisplay == nullptr || collapsedDisplay->getParentComponent () == nullptr)
        return {};
    return getLocalArea (collapsedDisplay->getParentComponent (), collapsedDisplay->getBounds ());
}

// the panel only holds the focus so that nothing under it can; the keys are for the waveform
void ExpandedWaveformPanel::focusGained (FocusChangeType)
{
    if (waveformDisplay.isVisible ())
        waveformDisplay.grabKeyboardFocus ();
}

void ExpandedWaveformPanel::paint (juce::Graphics& g)
{
    // Open, everything from the waveform's top down is solid. While the wash fades, it goes round the
    // small waveform rather than over it, so that is never dimmed.
    const auto opaqueBounds { waveformDisplay.isVisible () ? getLocalBounds ().withTop (expandedWaveformBounds.getY ()) : juce::Rectangle<int> {} };
    const auto clearBounds { waveformDisplay.isVisible () ? opaqueBounds : getCollapsedBounds () };

    g.saveState ();
    g.excludeClipRegion (clearBounds);
    g.setColour (findColour (A8Colours::disabledOverlay).withMultipliedAlpha (static_cast<float> (washAmount)));
    g.fillRect (getLocalBounds ());
    g.restoreState ();

    if (! opaqueBounds.isEmpty ())
    {
        g.setColour (findColour (A8Colours::windowBackground));
        g.fillRect (opaqueBounds);
    }
}

void ExpandedWaveformPanel::resized ()
{
    repaint ();
}
