#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Assimil8or/ValidatorProperties.h"
#include "../Utility/PersistentRootProperties.h"

class ToolWindow : public juce::Component, public juce::Timer, public juce::ValueTree::Listener
{
public:
    ToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    PersistentRootProperties persistentRootProperties;
    juce::TimeSliceThread tst {"dcl"};
    juce::DirectoryContentsList directoryContentsList {nullptr, tst};

    juce::Label scanningStatusLabel;
    juce::Label progressUpdateLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton fileMenuButton;
    std::vector<juce::File> foldersToScan;
    ValidatorProperties validatorProperties;

    void updateProgress (juce::String progressUpdate);
    void updateScanStatus (juce::String scanStatus);
    void verifySdCardImage ();
    void verifyFileUi ();
    void verifyFile (juce::File presetFile);
    void verifyFoldersUi ();
    void verifyFolders (juce::File folder, bool verifySubFolders);

    void timerCallback () override;
    void paint (juce::Graphics& g) override;
    void resized () override;
};
