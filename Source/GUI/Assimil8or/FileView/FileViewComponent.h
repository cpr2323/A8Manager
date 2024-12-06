#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioManager.h"
#include "../../../Utility/DirectoryDataProperties.h"
#include "../../../Utility/LambdaThread.h"

class FileViewComponent : public juce::Component,
                          private juce::ListBoxModel
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

    juce::TextButton openFolderButton;
    juce::TextButton newFolderButton;
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

    void buildQuickLookupList ();
    juce::ValueTree getDirectoryEntryVT (int row);
    void newFolder ();
    void openFolder ();
    void updateFromNewData ();

    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void resized () override;
};
    