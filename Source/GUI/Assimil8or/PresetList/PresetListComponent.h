#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"

const auto kMaxPresets { 199 };
class PresetListComponent : public juce::Component,
                            private juce::ListBoxModel
{
public:
    PresetListComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    juce::ListBox presetListBox { {}, this };
    std::array<bool, kMaxPresets> presetExitsts {false};
    void checkForPresets (juce::File folderToScan);

    void resized () override;
    int getNumRows () override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
};
