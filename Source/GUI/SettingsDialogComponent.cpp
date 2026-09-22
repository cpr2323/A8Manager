#include "SettingsDialogComponent.h"
#include "Theme/A8ColourIds.h"
#include "../SystemServices.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

namespace
{
    // wide enough for the audio page's "Audio device type:" label column plus its
    // combo boxes without the text having to shrink
    const auto kPageWidth { 400 };
    const auto kPageMargin { 12 };
    // only used to force a layout pass before the real height is measured
    const auto kMeasuringHeight { 2000 };
}

SettingsDialogComponent::SettingsDialogComponent ()
{
    setOpaque (true);
    tabs.setOutline (0);
    addAndMakeVisible (tabs);
}

int SettingsDialogComponent::contentHeightOf (const juce::Component& component)
{
    // AudioDeviceSelectorComponent lays its rows out top down and has no notion
    // of a preferred size, so ask the laid out children where they actually end
    auto bottom { 0 };
    for (auto* child : component.getChildren ())
        if (child->isVisible ())
            bottom = juce::jmax (bottom, child->getBottom ());
    return bottom;
}

void SettingsDialogComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);

    const auto tabColour { findColour (A8Colours::windowBackground) };

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    if (auto* audioDeviceManager { systemServices.getAudioDeviceManager () })
    {
        audioPage = std::make_unique<juce::AudioDeviceSelectorComponent> (*audioDeviceManager,
                                                                          0, 0, 0, 256,
                                                                          false, false, true, false);
        // lay it out once at the real width so its height can be measured
        audioPage->setSize (kPageWidth, kMeasuringHeight);
        preferredPageHeight = juce::jmax (preferredPageHeight, contentHeightOf (*audioPage) + kPageMargin);
        tabs.addTab ("AUDIO", tabColour, audioPage.get (), false);
    }

    appearancePage.init (rootPropertiesVT);
    preferredPageHeight = juce::jmax (preferredPageHeight, appearancePage.getPreferredHeight ());
    tabs.addTab ("APPEARANCE", tabColour, &appearancePage, false);

    // Open on whichever tab was last used. Done before the change callback is
    // wired up, so restoring the tab is not itself recorded as a choice.
    const auto storedTab { guiProperties.getSettingsTabName () };
    const auto tabIndex { tabs.getTabNames ().indexOf (storedTab) };
    if (tabIndex >= 0)
        tabs.setCurrentTabIndex (tabIndex, false);

    tabs.onTabChanged = [this] (const juce::String& tabName)
    {
        guiProperties.setSettingsTabName (tabName, false);
    };
}

juce::Rectangle<int> SettingsDialogComponent::getPreferredBounds () const
{
    return { 0, 0, kPageWidth, tabs.getTabBarDepth () + preferredPageHeight };
}

void SettingsDialogComponent::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    for (auto tabIndex { 0 }; tabIndex < tabs.getNumTabs (); ++tabIndex)
        tabs.setTabBackgroundColour (tabIndex, findColour (A8Colours::windowBackground));
}

void SettingsDialogComponent::resized ()
{
    tabs.setBounds (getLocalBounds ());
}

void SettingsDialogComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (A8Colours::windowBackground));
}

//==============================================================================
SettingsDialogComponent::AppearancePage::AppearancePage ()
{
    auto setupLabel = [this] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setFont (label.getFont ().withPointHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };

    setupLabel (backgroundLabel, "BACKGROUND", 14.0f, juce::Justification::centredLeft);
    setupLabel (darkEndLabel, "DARK", 11.0f, juce::Justification::centredLeft);
    setupLabel (lightEndLabel, "LIGHT", 11.0f, juce::Justification::centredRight);
    setupLabel (backgroundDescription,
                "Sets how light the app is. Text and markers switch ends where they have to, so they stay readable at any setting.",
                12.0f, juce::Justification::topLeft);
    backgroundDescription.setJustificationType (juce::Justification::topLeft);

    backgroundSlider.setRange (0.0, 1.0, 0.001);
    backgroundSlider.setTooltip ("Background lightness, from the near-black scheme to a white one.");
    addAndMakeVisible (backgroundSlider);
}

void SettingsDialogComponent::AppearancePage::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::yes);

    // follow the stored value, wherever it was changed from
    guiProperties.onBackgroundLevelChange = [this] (float backgroundLevel)
    {
        backgroundSlider.setValue (backgroundLevel, juce::NotificationType::dontSendNotification);
    };
    backgroundSlider.setValue (guiProperties.getBackgroundLevel (), juce::NotificationType::dontSendNotification);

    // write only; whoever cares about the new value is listening for it
    backgroundSlider.onValueChange = [this] ()
    {
        guiProperties.setBackgroundLevel (static_cast<float> (backgroundSlider.getValue ()), false);
    };
}

int SettingsDialogComponent::AppearancePage::getPreferredHeight () const
{
    // the fixed stack laid out in resized, plus its top and bottom margins
    return 14 + 20 + 4 + 26 + 16 + 10 + 56 + 14;
}

void SettingsDialogComponent::AppearancePage::resized ()
{
    auto localBounds { getLocalBounds ().reduced (14, 14) };

    backgroundLabel.setBounds (localBounds.removeFromTop (20));
    localBounds.removeFromTop (4);
    backgroundSlider.setBounds (localBounds.removeFromTop (26));

    auto endsBounds { localBounds.removeFromTop (16) };
    darkEndLabel.setBounds (endsBounds.removeFromLeft (endsBounds.getWidth () / 2));
    lightEndLabel.setBounds (endsBounds);

    localBounds.removeFromTop (10);
    backgroundDescription.setBounds (localBounds.removeFromTop (56));
}

void SettingsDialogComponent::AppearancePage::paint (juce::Graphics& g)
{
    g.fillAll (findColour (A8Colours::windowBackground));
}
