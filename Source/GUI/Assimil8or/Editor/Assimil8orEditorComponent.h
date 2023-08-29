#pragma once

#include <JuceHeader.h>
#include "ChannelEditor.h"
#include "CvInputComboBox.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

class WindowDecorator : public juce::Component
{
public:
private:
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::white.withAlpha (0.2f));
        g.setColour (juce::Colours::white);
        g.drawLine ({ getLocalBounds ().getTopLeft ().toFloat (),getLocalBounds ().getTopRight ().toFloat () });
    }
};

class Assimil8orEditorComponent : public juce::Component
{
public:
    Assimil8orEditorComponent ();
    ~Assimil8orEditorComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    PresetProperties presetProperties;
    PresetProperties minPresetProperties;
    PresetProperties maxPresetProperties;

    juce::Label titleLabel;
    juce::TextButton saveButton;
    juce::TextButton importButton;
    juce::TextButton exportButton;
    juce::TabbedComponent channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    WindowDecorator windowDecorator;

    // Preset Parameters
    juce::TextEditor nameEditor;
    juce::Label data2AsCvLabel;
    CvInputGlobalComboBox data2AsCvComboBox;
    juce::Label xfadeGroupsLabel;
    struct XfadeGroupControls
    {
        juce::Label xfadeGroupLabel;
        juce::Label xfadeCvLabel;
        CvInputGlobalComboBox xfadeCvComboBox;
        juce::Label xfadeWidthLabel;
        juce::TextEditor xfadeWidthEditor;  // double
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
    std::array<ChannelEditor, 8> channelEditors;

    void exportPreset ();
    juce::String formatXfadeWidthString (double width);
    void importPreset ();
    void receiveSampleLoadRequest (juce::File sampleFile);
    void savePreset ();
    void setupPresetComponents ();
    void setupPresetPropertiesCallbacks ();

    // Preset callbacks 
    void indexDataChanged (int index); // tracks when a new preset has been loaded
    void nameDataChanged (juce::String name);
    void nameUiChanged (juce::String name);
    void data2AsCvDataChanged (juce::String data2AsCvString);
    void data2AsCvUiChanged (juce::String data2AsCvString);
    void xfadeCvDataChanged (int group, juce::String data2AsCvString);
    void xfadeCvUiChanged (int group, juce::String data2AsCvString);
    void xfadeWidthDataChanged (int group, double);
    void xfadeWidthUiChanged (int group, double);

    void resized () override;
    void paint (juce::Graphics& g) override;
};
