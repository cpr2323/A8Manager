#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Utility/DirectoryDataProperties.h"
#include "../../../Utility/LambdaThread.h"

const auto kMaxPresets { 199 };
class PresetListComponent : public juce::Component,
                            private juce::ListBoxModel,
                            private juce::Timer
{
public:
    PresetListComponent ();
    ~PresetListComponent () = default;
    void init (juce::ValueTree rootPropertiesVT);

    std::function<void (std::function<void ()>, std::function<void ()>)> overwritePresetOrCancel;

private:
    AppProperties appProperties;
    DirectoryDataProperties directoryDataProperties;
    PresetProperties presetProperties;
    PresetProperties unEditedPresetProperties;
    PresetProperties copyBufferPresetProperties;

    juce::ToggleButton showAllPresets { "Show All" };
    juce::ListBox presetListBox { {}, this };
    std::array<std::tuple <int, bool, juce::String>, kMaxPresets> presetInfoList;
    int numPresets { kMaxPresets };
    juce::File currentFolder;
    juce::File previousFolder;
    int lastSelectedPresetIndex { -1 };
    LambdaThread checkPresetsThread { "CheckPresetsThread", 100 };

    void copyPreset (int presetNumber);
    void checkPresets ();
    void deletePreset (int presetNumber);
    juce::File getPresetFile (int presetNumber);
    void forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback);
    void loadPresetFile (juce::File presetFile, juce::ValueTree vt);
    void loadDefault (int row);
    void loadFirstPreset ();
    void loadPreset (juce::File presetFile);
    void pastePreset (int presetNumber);

    void resized () override;
    int getNumRows () override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void timerCallback () override;
};
