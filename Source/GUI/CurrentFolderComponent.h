#pragma once

#include <JuceHeader.h>
#include "GuiProperties.h"
#include "Theme/UiComponents.h"
#include "../AppProperties.h"
#include "oolib/Directory/DirectoryDataProperties.h"

/*
    The strip along the top of the window: where you are, which output you are
    listening on, and the way in to Settings.

    The path is drawn as breadcrumbs rather than a raw path string - the folders
    are context, so they are dimmed, and the open preset file is the part that
    matters, so it is not. The preset file is added as the last crumb only while
    it is in the folder being viewed. While the folder is being scanned, the scan
    progress follows the crumbs.
*/
class CurrentFolderComponent : public juce::Component
{
public:
    CurrentFolderComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    GuiProperties guiProperties;
    DirectoryDataProperties directoryDataProperties;
    juce::AudioDeviceManager* audioDeviceManager { nullptr };

    static constexpr int kPadding { 9 };
    juce::StringArray pathSegments;
    // the last crumb is picked out only when it names the open preset file
    bool pathEndsInFile { false };
    juce::String progressText;

    ChromeButton outputButton { "OUT", ChromeButton::Size::chip };
    ChromeButton settingsButton { "SETTINGS", ChromeButton::Size::chip };

    void refreshPath ();
    void showOutputMenu ();
    void refreshOutputName ();

    void paint (juce::Graphics& g) override;
    void resized () override;
};
