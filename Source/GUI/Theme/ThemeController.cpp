#include "ThemeController.h"
#include "oolib/Properties/PersistentRootProperties.h"

ThemeController::ThemeController ()
{
    // Installed before any component is built, so everything - including popup
    // menus and dialogs, which have no parent to inherit from - resolves through
    // the palette.
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
}

ThemeController::~ThemeController ()
{
    stopTimer ();
    // must outlive every component that resolves through it
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

void ThemeController::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::yes);
    guiProperties.onBackgroundLevelChange = [this] (float backgroundLevel) { requestBackground (backgroundLevel); };

    // the stored level is applied straight away, before there is anything to repaint
    requestedBackground = guiProperties.getBackgroundLevel ();
    appliedBackground = requestedBackground;
    applyBackground (requestedBackground);
}

void ThemeController::requestBackground (float backgroundLevel)
{
    requestedBackground = backgroundLevel;

    // First change of a gesture goes through immediately, so dragging the slider
    // responds at once rather than after a tick.
    if (! isTimerRunning ())
    {
        appliedBackground = requestedBackground;
        applyBackground (requestedBackground);
        startTimer (kMinIntervalMs);
        return;
    }

    // Otherwise it is left for the timer to pick up. Everything that arrives
    // before the next tick collapses into one application of the newest value.
}

void ThemeController::timerCallback ()
{
    if (! juce::approximatelyEqual (requestedBackground, appliedBackground))
    {
        appliedBackground = requestedBackground;
        applyBackground (requestedBackground);
        // keep running: the gesture is still going, and the value that arrives
        // after this one still has to land
        return;
    }

    // nothing new since the last tick, so the gesture has settled
    stopTimer ();
}

void ThemeController::applyBackground (float backgroundLevel)
{
    lookAndFeel.setBackground (backgroundLevel);

    // Every top level window has to be told, not just the main one: the settings
    // dialog and any open popup are siblings on the desktop, not children of it.
    auto& desktop { juce::Desktop::getInstance () };
    for (auto componentIndex { 0 }; componentIndex < desktop.getNumComponents (); ++componentIndex)
        if (auto* component { desktop.getComponent (componentIndex) })
            component->sendLookAndFeelChange ();
}
