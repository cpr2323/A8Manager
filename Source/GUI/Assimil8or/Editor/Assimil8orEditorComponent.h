#pragma once

#include <JuceHeader.h>
#include "ChannelEditor.h"
#include "CvInputComboBox.h"
#include "EditManager.h"
#include "../../GuiControlProperties.h"
#include "../../Theme/UiComponents.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "oolib/GUI/CustomTextEditor.h"
#include "oolib/Debug/DebugLog.h"
#include "oolib/Properties/RuntimeRootProperties.h"

// the strip along the bottom of the editor, behind the preset wide Data2 and XFade controls
class WindowDecorator : public juce::Component
{
public:
    WindowDecorator () { setInterceptsMouseClicks (false, false); }
private:
    void paint (juce::Graphics& g) override
    {
        g.fillAll (findColour (A8Colours::listBackground));
        g.setColour (findColour (A8Colours::outline));
        g.fillRect (getLocalBounds ().removeFromTop (1));
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
    GuiControlProperties guiControlProperties;
    PresetProperties presetProperties;
    PresetProperties unEditedPresetProperties;
    PresetProperties defaultPresetProperties;
    PresetProperties minPresetProperties;
    PresetProperties maxPresetProperties;
    ChannelProperties defaultChannelProperties;
    ChannelProperties copyBufferChannelProperties;
    ZoneProperties copyBufferZoneProperties;
    bool copyBufferHasData { false };
    EditManager* editManager { nullptr };
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::Label titleLabel;
    ActionButton saveButton { "SAVE" };
    MenuButton toolsButton { "PRESET TOOLS" };
    bool presetHasUnsavedEdits { false };

    static constexpr int kHeaderHeight { 38 };
    static constexpr int kTabBarHeight { 31 };
    static constexpr int kFieldHeight { 23 };
    static constexpr int kBottomRowHeight { 28 };
    static inline const juce::String kUnsavedEditsText { "UNSAVED EDITS" };

    // set in resized, drawn in paint
    juce::Rectangle<int> headerBounds;
    juce::Rectangle<int> unsavedEditsBounds;

    LedTabbedComponent channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    WindowDecorator windowDecorator;

    // Preset Parameters
    juce::TextEditor nameEditor;
    juce::Label midiSetupLabel;
    CustomComboBox midiSetupComboBox;

    juce::Label data2AsCvLabel;
    CvInputGlobalComboBox data2AsCvComboBox;
    juce::Label xfadeGroupsLabel;
    struct XfadeGroupControls
    {
        juce::Label xfadeGroupLabel;
        juce::Label xfadeCvLabel;
        CvInputGlobalComboBox xfadeCvComboBox;
        juce::Label xfadeWidthLabel;
        CustomTextEditorDouble xfadeWidthEditor;
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
    void setWaveformExpanded (bool isExpanded);
    std::array<ChannelProperties, 8> channelProperties;

    void displayToolsMenu ();
    void explodeChannel (int channelIndex, int explodeCount);
    void exportPresetSettings ();
    void exportPresetSettingsAndSamples ();
    juce::String formatXfadeWidthString (double width);
    void importPresetSettings ();
    void importPresetSettingsAndSamples ();
    juce::PopupMenu createChannelCloneMenu (int channelIndex,   std::function <void (ChannelProperties&)> setter);
    bool isChannelActive (int channelIndex);
    void revertPreset ();
    void savePreset ();
    void setPresetToDefaults ();
    void setupPresetComponents ();
    void setupPresetPropertiesCallbacks ();
    void updateAllChannelTabNames ();
    void updateChannelTabName (int channelIndex);

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

    void applyExplicitColours ();

    void timerCallback () override;
    void resized () override;
    void lookAndFeelChanged () override;
    void paint (juce::Graphics& g) override;
};
