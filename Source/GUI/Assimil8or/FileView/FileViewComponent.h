#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Utility/DirectoryValueTree.h"

class FileViewComponent : public juce::Component,
                          private juce::Thread,
                          private juce::ListBoxModel
{
public:
    FileViewComponent ();
    ~FileViewComponent ();
    void init (juce::ValueTree rootPropertiesVT);

    std::function<void (juce::File audioFile)> onAudioFileSelected;
    std::function<void (std::function<void ()>, std::function<void ()>)> overwritePresetOrCancel;

private:
    AppProperties appProperties;
    juce::AudioFormatManager audioFormatManager;
    juce::TextButton openFolderButton;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ListBox directoryContentsListBox { {}, this };
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    int listOffset { 0 };
    int lastSelectedRow { -1 };

    DirectoryValueTree directoryValueTree;
    std::vector<juce::ValueTree> directoryListQuickLookupList;

    void buildQuickLookupList ();
    void openFolder ();
    bool isSupportedAudioFile (juce::File file);
    void startScan (juce::File folderToScan);

    void resized () override;
    void run () override;
    int getNumRows () override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent& me) override;
};
    