#pragma once

#include <JuceHeader.h>
#include "ChannelEditor.h"
#include "CvInputComboBox.h"
#include "EditManager.h"
#include "SamplePool/SamplePool.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/DirectoryDataProperties.h"
#include "../../../Utility/InputControlComponent.h"
#include "../../../Utility/RuntimeRootProperties.h"

class CustomTextEditor : public juce::TextEditor
{
public:
    void mouseDown (juce::MouseEvent& mouseEvent)
    {
        if (! mouseEvent.mods.isPopupMenu ())
        {
            if (mouseEvent.mods.isCtrlDown ())
            {
                DebugLog ("CustomTextEditor", "capturing mouse");
                mouseDownCaptured = true;
                return;
            }
        }
        juce::TextEditor::mouseDown (mouseEvent);
    }

    void mouseUp (juce::MouseEvent& mouseEvent)
    {
        if (mouseDownCaptured == true)
        {
            mouseDownCaptured = false;
        }
        else
        {
            juce::TextEditor::mouseUp (mouseEvent);
        }
    }

private:
    bool mouseDownCaptured { false };
};

class WindowDecorator : public juce::Component
{
public:
private:
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::grey);
        g.setColour (juce::Colours::white);
        g.drawLine ({ getLocalBounds ().getTopLeft ().toFloat (),getLocalBounds ().getTopRight ().toFloat () });
    }
};

class Assimil8orEditorComponent : public juce::Component,
                                  public juce::Timer
{
public:
    Assimil8orEditorComponent ();
    ~Assimil8orEditorComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);
    void receiveSampleLoadRequest (juce::File sampleFile);
    void overwritePresetOrCancel (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction);

private:
    RuntimeRootProperties runtimeRootProperties;
    AppProperties appProperties;
    AudioPlayerProperties audioPlayerProperties;
    DirectoryDataProperties directoryDataProperties;
    PresetProperties presetProperties;
    PresetProperties unEditedPresetProperties;
    PresetProperties defaultPresetProperties;
    PresetProperties minPresetProperties;
    PresetProperties maxPresetProperties;
    ChannelProperties defaultChannelProperties;
    ChannelProperties copyBufferChannelProperties;
    bool copyBufferHasData { false };
    SamplePool samplePool;
    EditManager editManager;

    juce::Label titleLabel;
    juce::TextButton saveButton;
    juce::TextButton importButton;
    juce::TextButton exportButton;
    juce::TabbedComponent channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    WindowDecorator windowDecorator;

    // Preset Parameters
    juce::TextEditor nameEditor;
    juce::Label midiSetupLabel;
    juce::ComboBox midiSetupComboBox;
    juce::Label data2AsCvLabel;
    CvInputGlobalComboBox data2AsCvComboBox;
    juce::Label xfadeGroupsLabel;
    struct XfadeGroupControls
    {
        juce::Label xfadeGroupLabel;
        juce::Label xfadeCvLabel;
        CvInputGlobalComboBox xfadeCvComboBox;
        juce::Label xfadeWidthLabel;
        InputControlComponent inputControlComponent;
        //CustomTextEditor xfadeWidthEditor;
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
    std::array<ChannelProperties, 8> channelProperties;

    void checkSampleFilesExistance ();
    void exportPreset ();
    juce::String formatXfadeWidthString (double width);
    void importPreset ();
    bool isChannelActive (int channelIndex);
    void savePreset ();
    void setupPresetComponents ();
    void setupPresetPropertiesCallbacks ();

    // Preset callbacks
    void idDataChanged (int id); // tracks when a new preset has been loaded
    void midiSetupDataChanged (int midiSetupId);
    void midiSetupUiChanged (int midiSetupId);
    void nameDataChanged (juce::String name);
    void nameUiChanged (juce::String name);
    void data2AsCvDataChanged (juce::String data2AsCvString);
    void data2AsCvUiChanged (juce::String data2AsCvString);
    void xfadeCvDataChanged (int group, juce::String data2AsCvString);
    void xfadeCvUiChanged (int group, juce::String data2AsCvString);
    void xfadeWidthDataChanged (int group, double);
    void xfadeWidthUiChanged (int group, double);

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
    void updateAllChannelTabNames ();
    void updateChannelTabName (int channelIndex);
};
