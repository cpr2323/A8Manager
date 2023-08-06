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
    juce::TextButton navigateUpButton;

    juce::TimeSliceThread folderContentsThread { "FolderContentsThread" };
    juce::DirectoryContentsList folderContentsDirectoryList { nullptr, folderContentsThread };
    juce::FileTreeComponent folderContentsTree { folderContentsDirectoryList };

    AppProperties appProperties;

    void startFolderScan (juce::File folderToScan);

    void resized () override;
    void timerCallback () override;
};
    