#pragma once

#include <JuceHeader.h>
#include "A8LookAndFeel.h"
#include "../GuiProperties.h"

/*
    Owns the app's LookAndFeel and keeps it in step with the stored background level.

    This is the "code responsible for doing the change" end of the wiring: it
    wraps GuiProperties as a client with callbacks on, so it hears about a new
    background level however it was set. Whatever set it - a slider, a restored
    settings file, anything added later - is none of its business, and it holds
    no reference to any of them.

    Applying a background level is not cheap: every colour is re-resolved and then
    every top level window is walked, so each component can re-read its colours
    and repaint. That is fine a few times a second and far too much once per
    mouse move, so changes are coalesced - the first one applies immediately, and
    after that at most one per kMinIntervalMs, always with the newest value.
*/
class ThemeController : private juce::Timer
{
public:
    ThemeController ();
    ~ThemeController () override;

    void init (juce::ValueTree rootPropertiesVT);

private:
    // ~30 applications a second is smooth to the eye and leaves the message
    // thread time to actually paint between them
    static constexpr int kMinIntervalMs { 33 };

    void requestBackground (float backgroundLevel);
    void applyBackground (float backgroundLevel);
    void timerCallback () override;

    A8LookAndFeel lookAndFeel;
    GuiProperties guiProperties;

    float requestedBackground { 0.0f };
    float appliedBackground { 0.0f };

    JUCE_DECLARE_NON_COPYABLE (ThemeController)
};
