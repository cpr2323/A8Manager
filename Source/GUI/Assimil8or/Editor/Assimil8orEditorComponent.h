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
    juce::TextEditor nameEditor;

    juce::TextButton saveButton;
    juce::TextButton importButton;
    juce::TextButton exportButton;

    void exportPreset ();
    juce::String getPresetFileName (int presetIndex);
    void importPreset ();
    void refreshName (juce::String name);
    void savePreset ();
    void updateName (juce::String name);

    void resized () override;
    void paint (juce::Graphics& g) override;
};
