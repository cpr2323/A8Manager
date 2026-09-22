#pragma once

#include <JuceHeader.h>
#include "../../Theme/UiComponents.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioManager.h"
#include "oolib/Directory/DirectoryDataProperties.h"
#include "oolib/Directory/DirectoryValueTree.h"
#include "oolib/Core/LambdaThread.h"

class FileViewComponent : public juce::Component,
                          private juce::ListBoxModel,
                          private juce::Timer,
                          public juce::FileDragAndDropTarget
{
public:
    FileViewComponent ();
    ~FileViewComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

    // narrow enough that the header still fits its title and tools
    int getMinimumWidth () const;

    std::function<void (juce::File audioFile)> onAudioFileSelected;
    std::function<void (std::function<void ()>, std::function<void ()>)> overwritePresetOrCancel;

private:
    AppProperties appProperties;
    DirectoryDataProperties directoryDataProperties;
    AudioManager* audioManager { nullptr };
    // resolved in init (), once DirectoryValueTree has published the types Main registered
    int audioFileTypeId { DirectoryValueTree::unknownTypeId };
    int presetFileTypeId { DirectoryValueTree::unknownTypeId };

    juce::CriticalSection directoryListQuickLookupListLock;
    std::vector<juce::ValueTree> directoryListQuickLookupListA;
    std::vector<juce::ValueTree> directoryListQuickLookupListB;
    std::vector<juce::ValueTree>* curDirectoryListQuickLookupList { &directoryListQuickLookupListA };
    std::vector<juce::ValueTree>* updateDirectoryListQuickLookupList { &directoryListQuickLookupListB };

    PaneHeader paneHeader { "FOLDERS" };
    ChromeButton optionsButton { "OPTIONS" };
    ChromeButton showAllFiles { "ALL" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ListBox directoryContentsListBox { {}, this };
    ListRowHover rowHover { directoryContentsListBox };
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    bool isRootFolder { false };
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
    void selectedRowsChanged (int lastRowSelected) override;
    void resized () override;
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
};
    