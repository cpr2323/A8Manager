#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"

class FileViewComponent : public juce::Component,
                          private juce::Timer
{
public:
    FileViewComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;

    juce::TextButton navigateUpButton;
    juce::TextButton openFolderButton;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TimeSliceThread folderContentsThread { "FolderContentsThread" };
    juce::DirectoryContentsList folderContentsDirectoryList { nullptr, folderContentsThread };

    void openFolder ();
    void startFolderScan (juce::File folderToScan);

    void resized () override;
    void timerCallback () override;
};
    