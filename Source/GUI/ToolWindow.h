#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Utility/PersistentRootProperties.h"

class ToolWindow : public juce::Component, public juce::Timer
{
public:
    ToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    PersistentRootProperties persistentRootProperties;
    juce::TimeSliceThread tst {"dcl"};
    juce::DirectoryContentsList directoryContentsList {nullptr, tst};

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton fileMenuButton;
    std::vector<juce::File> foldersToScan;

    void verifyFileUi ();
    void verifyFile (juce::File presetFile);
    void verifyFoldersUi ();
    void verifyFolders (juce::File folder, bool verifySubFolders);

    void timerCallback () override;
    void paint (juce::Graphics& g) override;
    void resized () override;
};
