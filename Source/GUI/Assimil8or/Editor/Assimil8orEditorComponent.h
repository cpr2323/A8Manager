#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

class Assimil8orEditorComponent : public juce::Component
{
public:
    Assimil8orEditorComponent ();
    ~Assimil8orEditorComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    PresetProperties presetProperties;

    juce::TextButton saveButton;
    juce::TextButton importButton;
    juce::TextButton exportButton;

    // preset fields
    // Data2asCV
    // Name
    // XfadeACV
    // XfadeAWidth
    // XfadeBCV
    // XfadeBWidth
    // XfadeCCV
    // XfadeCWidth
    // XfadeDCV
    // XfadeDWidth
    juce::TextEditor nameEditor;
    juce::Label data2AsCvLabel;
    juce::ComboBox data2AsCvComboBox;
    struct XfadeGroupControls
    {
        juce::Label xfadeCvLabel;
        juce::TextEditor xfadeCvEditor;
        juce::Label xfadeWidthLabel;
        juce::TextEditor xfadeWidthEditor;
    };
    enum XfadeGroupIndex
    {
        groupA,
        groupB,
        groupC,
        groupD,
        numberOfGroups
    };
    std::array<XfadeGroupControls, 4> xfadeGroups;

    void exportPreset ();
    juce::String getPresetFileName (int presetIndex);
    void importPreset ();
    void savePreset ();
    void setupChannelControls ();
    void setupPresetControls ();
    void setupZoneControls ();

    void nameDataChanged (juce::String name);
    void nameUiChanged (juce::String name);

    void data2AsCvDataChanged (juce::String data2AsCvString);
    void data2AsCvUiChanged (juce::String data2AsCvString);

    void resized () override;
    void paint (juce::Graphics& g) override;
};
