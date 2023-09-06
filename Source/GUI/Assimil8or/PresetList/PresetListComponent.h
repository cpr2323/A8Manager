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

    juce::ToggleButton showAllPresets { "Show All" };
    juce::ListBox presetListBox { {}, this };
    std::array<std::tuple <int, bool>, kMaxPresets> presetExists;
    int numPresets { kMaxPresets };
    juce::File rootFolder;
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    std::atomic<bool> newItemQueued { false };
    int lastSelectedRow { -1 };

    void checkForPresets (bool resetPosition);
    void forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback);
    juce::String getPresetName (int presetIndex);
    void loadDefault (int row);
    void loadFirstPreset ();
    void loadPreset (juce::File presetFile);
    bool shouldCancelOperation ();
    void startScan (juce::File folderToScan);

    void resized () override;
    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void run () override;
    void timerCallback () override;
};
