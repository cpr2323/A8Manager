#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"

#define XFADE_GROUPS_HORIZONTAL 1

class CvInputComboBox : public juce::Component
{
public:
    CvInputComboBox ()
    {
        addAndMakeVisible (cvInputComboBox);
        {
            auto menuId { 1 };
            cvInputComboBox.addItem ("Off", menuId);
            ++menuId;
            for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
                for (auto columnIndex { 0 }; columnIndex < 3; ++columnIndex)
                {
                    cvInputComboBox.addItem (juce::String::charToString ('1' + channelIndex) + juce::String::charToString ('A' + columnIndex), menuId);
                    ++menuId;
                }
        }
        cvInputComboBox.onChange = [this] ()
        {
            if (onChange != nullptr)
                onChange ();
        };
    }

    void setSelectedItemText (juce::String cvInputString)
    {
        auto itemId { 1 };
        if (cvInputString.isEmpty ())
        {
            cvInputComboBox.setText ("", juce::NotificationType::sendNotification);
            return;
        }
        if (cvInputString.toLowerCase () != "off")
        {
            jassert (juce::String ("12345678").containsChar (cvInputString [0]) && juce::String ("ABC").containsChar (cvInputString [1]));
            itemId = 2 + ((cvInputString [0] - '1') * 3) + cvInputString [1] - 'A';
            jassert (itemId > 1 && itemId < 26);
        }
        cvInputComboBox.setSelectedId (itemId, juce::NotificationType::dontSendNotification);
    }

    juce::String getSelectedItemText ()
    {
        return cvInputComboBox.getItemText (cvInputComboBox.getSelectedItemIndex ());
    }

    std::function<void ()> onChange;
private:
    juce::ComboBox cvInputComboBox;

    void resized () override
    {
        cvInputComboBox.setBounds (getLocalBounds ());
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
    CvInputComboBox data2AsCvComboBox;
#if ! XFADE_GROUPS_HORIZONTAL
    juce::Label xfadeGroupSectionLabel;
#endif
    struct XfadeGroupControls
    {
        juce::Label xfadeGroupLabel;
        juce::Label xfadeCvLabel;
        CvInputComboBox xfadeCvComboBox;
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
    void setupPresetPropertiesCallbacks ();
    void setupZoneControls ();

    void nameDataChanged (juce::String name);
    void nameUiChanged (juce::String name);

    void data2AsCvDataChanged (juce::String data2AsCvString);
    void data2AsCvUiChanged (juce::String data2AsCvString);

    void xfadeCvDataChanged (int group, juce::String data2AsCvString);
    void xfadeCvUiChanged (int group, juce::String data2AsCvString);

    void xfadeWidthDataChanged (int group, juce::String width);
    void xfadeWidthUiChanged (int group, juce::String width);

    void resized () override;
    void paint (juce::Graphics& g) override;
};
