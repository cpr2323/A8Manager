#pragma once

#include <JuceHeader.h>
#include "ChannelEditor.h"
#include "CvInputComboBox.h"
#include "EditManager.h"
#include "../../GuiControlProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/RuntimeRootProperties.h"

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
                                  public juce::Timer,
                                  private juce::KeyListener,
                                  private juce::FocusChangeListener
{
public:
    Assimil8orEditorComponent ();
    ~Assimil8orEditorComponent () override;

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
    juce::TextButton saveButton;
    juce::TextButton toolsButton;

    juce::TabbedComponent channelTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
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
    void channelModeChanged (int channelIndex);
    void revertPreset ();
    void savePreset ();
    void setPresetToDefaults ();
    void setupPresetComponents ();
    void setupPresetPropertiesCallbacks ();
    void updateAllChannelTabNames ();
    void setChannelTabName (int channelIndex);
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

    // The hot keys: ctrl (cmd on the Mac) + 1-8 selects a channel, alt (option) + 1-8 a zone, Space is TRIGGER
    // and Shift+Space is LATCH. A key goes to the focused component, and then up through its parents, so a
    // text editor would take them first, typing the digit or space. A KeyListener on the focused component
    // hears the key before the component does, so the listener follows the focus, and also sits on the
    // window, which takes the keys when nothing has the focus.
    juce::Component::SafePointer<juce::Component> focusedKeyTarget;
    juce::Component::SafePointer<juce::Component> windowKeyTarget;
    // the zone the space bar is holding TRIGGER on, which is let go there, whichever zone is selected by then
    ZoneEditor* spaceKeyZoneEditor { nullptr };
    bool spaceKeyDown { false };
    // a key release can happen anywhere, so the space bar is watched until it is let go
    struct SpaceKeyWatcher : public juce::Timer
    {
        std::function<void ()> onTimer;
        void timerCallback () override { onTimer (); }
    };
    SpaceKeyWatcher spaceKeyWatcher;

    using juce::Component::keyPressed;
    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;
    void globalFocusChanged (juce::Component* focusedComponent) override;
    void spaceKeyReleased ();

    void timerCallback () override;
    void parentHierarchyChanged () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};
