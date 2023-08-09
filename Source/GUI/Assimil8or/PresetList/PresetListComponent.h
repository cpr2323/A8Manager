#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

const auto kMaxPresets { 199 };
class PresetListComponent : public juce::Component,
                            private juce::ListBoxModel,
                            private juce::Thread
{
public:
    PresetListComponent ();
    ~PresetListComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    PresetProperties presetProperties;
    juce::ListBox presetListBox { {}, this };
    std::array<bool, kMaxPresets> presetExists {false};
    juce::CriticalSection queuedFolderLock;
    juce::File queuedFolderToScan;
    juce::File rootFolder;
    std::atomic<bool> newItemQueued { false };

    void checkForPresets ();
    void forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback);
    juce::String getPresetName (int presetIndex);
    void loadFirstPreset ();
    void loadPreset (juce::File presetFile);
    bool shouldCancelOperation ();
    void startScan (juce::File folderToScan);

    void resized () override;
    int getNumRows () override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void run () override;
};
