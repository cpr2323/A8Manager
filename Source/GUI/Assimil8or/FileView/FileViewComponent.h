#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioManager.h"
#include "../../../Utility/DirectoryDataProperties.h"
#include "../../../Utility/LambdaThread.h"

class FileViewComponent : public juce::Component,
                          private juce::ListBoxModel,
                          private juce::Timer,
                          public juce::FileDragAndDropTarget
{
public:
    FileViewComponent ();
    ~FileViewComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

    std::function<void (juce::File audioFile)> onAudioFileSelected;
    std::function<void (std::function<void ()>, std::function<void ()>)> overwritePresetOrCancel;

private:
    AppProperties appProperties;
    DirectoryDataProperties directoryDataProperties;
    AudioManager* audioManager { nullptr };

    juce::CriticalSection directoryListQuickLookupListLock;
    std::vector<juce::ValueTree> directoryListQuickLookupListA;
    std::vector<juce::ValueTree> directoryListQuickLookupListB;
    std::vector<juce::ValueTree>* curDirectoryListQuickLookupList { &directoryListQuickLookupListA };
    std::vector<juce::ValueTree>* updateDirectoryListQuickLookupList { &directoryListQuickLookupListB };

    juce::TextButton optionsButton;
    juce::ToggleButton showAllFiles { "Show All" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ListBox directoryContentsListBox { {}, this };
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    bool isRootFolder { false };
    int lastSelectedRow { -1 };
    std::unique_ptr<juce::AlertWindow> renameAlertWindow;
    std::unique_ptr<juce::AlertWindow> newAlertWindow;
    LambdaThread updateFromNewDataThread { "UpdateFromNewDataThread", 100 };

    juce::int64 curBlinkTime { 0 };
    int doubleClickedRow { -1 };

    int draggingFilesCount { 0 };
    bool supportedFile { false };
    juce::String dropMsg;

    void buildQuickLookupList ();
    void deleteUnusedSamples ();
    juce::ValueTree getDirectoryEntryVT (int row);
    void importSamples (const juce::StringArray& files);
    void newFolder ();
    void openFolder ();
    void resetDropInfo ();
    void updateDropInfo (const juce::StringArray& files);
    void updateFromNewData ();

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void fileDragEnter (const juce::StringArray& files, int, int) override;
    void fileDragMove (const juce::StringArray& files, int, int) override;
    void fileDragExit (const juce::StringArray& files) override;

    void timerCallback () override;
    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void resized () override;
    void paintOverChildren (juce::Graphics& g) override;
};
    