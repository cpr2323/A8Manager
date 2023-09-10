#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

const auto kMaxPresets { 199 };
class PresetListComponent : public juce::Component,
                            private juce::ListBoxModel,
                            private juce::Timer,
                            private juce::Thread
{
public:
    PresetListComponent ();
    ~PresetListComponent ();
    void init (juce::ValueTree rootPropertiesVT);

    std::function<void (std::function<void ()>, std::function<void ()>)> overwritePresetOrCancel;

private:
    AppProperties appProperties;
    PresetProperties presetProperties;
    PresetProperties unEditedPresetProperties;
    PresetProperties copyBufferPresetProperties;

    juce::ToggleButton showAllPresets { "Show All" };
    juce::ListBox presetListBox { {}, this };
    std::array<std::tuple <int, bool, juce::String>, kMaxPresets> presetInfo;
    int numPresets { kMaxPresets };
    juce::File rootFolder;
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    std::atomic<bool> newItemQueued { false };
    std::atomic<bool> newItemIsNewFolder { true };
    int lastSelectedRow { -1 };

    void copyPreset (int presetNumber);
    void checkForPresets (bool resetPosition);
    void deletePreset (int presetNumber);
    juce::File getPresetFile (int presetNumber);
    void forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback);
    juce::String getPresetName (int presetIndex);
    void loadPresetFile (juce::File presetFile, juce::ValueTree vt);
    void loadDefault (int row);
    void loadFirstPreset ();
    void loadPreset (juce::File presetFile);
    void pastePreset (int presetNumber);
    bool shouldCancelOperation ();
    void startScan (juce::File folderToScan, bool selectingNewFolder);

    void resized () override;
    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void run () override;
    void timerCallback () override;
};
