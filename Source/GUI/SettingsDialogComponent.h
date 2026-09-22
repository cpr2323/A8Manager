#pragma once

#include <JuceHeader.h>
#include "GuiProperties.h"

/*
    The app's settings window: a tab strip with one page per area.

    The Audio page is a juce::AudioDeviceSelectorComponent built here, in the GUI
    layer, from the juce::AudioDeviceManager the audio layer publishes through
    SystemServices. It skins itself from the app's LookAndFeel, so its combo
    boxes, labels, channel list and Test button all follow the palette.
*/
class SettingsDialogComponent : public juce::Component
{
public:
    SettingsDialogComponent ();

    void init (juce::ValueTree rootPropertiesVT);

    // The size the window should be given: wide and tall enough for the largest
    // page, measured rather than guessed, so adding rows to a page cannot leave
    // it clipped. Only valid once init has run.
    juce::Rectangle<int> getPreferredBounds () const;

    void resized () override;
    void paint (juce::Graphics& g) override;
    void lookAndFeelChanged () override;

private:
    /*
        The background control. It is a plain client of GuiProperties: it writes the
        new level and reads the current one, and knows nothing about the
        LookAndFeel or about who repaints. ThemeController hears the same
        property change and does that work.
    */
    class AppearancePage : public juce::Component
    {
    public:
        AppearancePage ();
        void init (juce::ValueTree rootPropertiesVT);
        void resized () override;
        void paint (juce::Graphics& g) override;
        int getPreferredHeight () const;

    private:
        GuiProperties guiProperties;
        juce::Label   backgroundLabel;
        juce::Label   darkEndLabel;
        juce::Label   lightEndLabel;
        juce::Label   backgroundDescription;
        juce::Slider  backgroundSlider { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    };

    // TabbedComponent only reports a tab change through a virtual, so a small
    // subclass is the way to hear about it without polling.
    class TabStrip : public juce::TabbedComponent
    {
    public:
        TabStrip () : juce::TabbedComponent (juce::TabbedButtonBar::TabsAtTop) {}
        std::function<void (const juce::String& tabName)> onTabChanged;
        void currentTabChanged (int, const juce::String& newCurrentTabName) override
        {
            if (onTabChanged != nullptr)
                onTabChanged (newCurrentTabName);
        }
    };

    static int contentHeightOf (const juce::Component& component);

    GuiProperties guiProperties;
    TabStrip tabs;
    AppearancePage appearancePage;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioPage;
    int preferredPageHeight { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsDialogComponent)
};
