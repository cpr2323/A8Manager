#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Utility/DirectoryValueTree.h"

class FileViewComponent : public juce::Component,
                          private juce::Thread,
                          private juce::Timer,
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
    juce::TextButton newFolderButton;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ListBox directoryContentsListBox { {}, this };
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    bool isRootFolder { false };
    int lastSelectedRow { -1 };
    std::unique_ptr<juce::AlertWindow> renameAlertWindow;
    std::unique_ptr<juce::AlertWindow> newAlertWindow;

    DirectoryValueTree directoryValueTree;
    std::vector<juce::ValueTree> directoryListQuickLookupList;

    void buildQuickLookupList ();
    void newFolder ();
    void openFolder ();
    bool isSupportedAudioFile (juce::File file);
    void startScan (juce::File folderToScan);

    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void resized () override;
    void run () override;
    void timerCallback () override;
};
    