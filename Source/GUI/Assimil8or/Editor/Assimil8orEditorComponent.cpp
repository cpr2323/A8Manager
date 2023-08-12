#include "Assimil8orEditorComponent.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/PersistentRootProperties.h"

// TODO - short list
//  Implement defaults/min/max data structures
//  Implement default support in the editor and preset data model
//  Add editor fields
//  Visual Edited indicator
//  Query for save if switching from edited Preset
//  Only enable Save button if preset was edited
//  Update Preset List (switch from dim to highlighted) when new preset created

Assimil8orEditorComponent::Assimil8orEditorComponent ()
{
    setOpaque (true);

    auto setupButton = [this] (juce::TextButton& button, juce::String text, std::function<void ()> buttonFunction)
    {
        button.setButtonText (text);
        button.onClick = buttonFunction;
        addAndMakeVisible (button);
    };
    setupButton (saveButton, "Save", [this] () { savePreset ();  });
    setupButton (importButton, "Import", [this] () { importPreset ();  });
    setupButton (exportButton, "Export", [this] () { exportPreset (); });
    importButton.setEnabled (false);
    exportButton.setEnabled (false);

    setupPresetControls ();
    setupChannelControls ();
    setupZoneControls ();
}
void Assimil8orEditorComponent::setupChannelControls ()
{
}
void Assimil8orEditorComponent::setupZoneControls ()
{
}
void Assimil8orEditorComponent::setupPresetControls ()
{
    nameEditor.setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::navajowhite);
    nameEditor.setColour (juce::TextEditor::ColourIds::textColourId, juce::Colours::black);
    nameEditor.onFocusLost = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { nameUiChanged (nameEditor.getText ()); };
    addAndMakeVisible (nameEditor);

    data2AsCvLabel.setBorderSize ({ 1, 0, 1, 0 });
    data2AsCvLabel.setText ("Data2 As CV", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (data2AsCvLabel);
    data2AsCvComboBox.onChange = [this] ()
    {
        data2AsCvUiChanged (data2AsCvComboBox.getSelectedItemText ());
    };
    addAndMakeVisible (data2AsCvComboBox);
#if ! XFADE_GROUPS_HORIZONTAL
    xfadeGroupSectionLabel.setText ("Crossfade Groups", juce::NotificationType::dontSendNotification);
    xfadeGroupSectionLabel.setJustificationType (juce::Justification::centredTop);
    addAndMakeVisible (xfadeGroupSectionLabel);
#endif
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };

        xfadeGroup.xfadeGroupLabel.setBorderSize ({ 0, 0, 0, 0 });
#if XFADE_GROUPS_HORIZONTAL
        xfadeGroup.xfadeGroupLabel.setJustificationType (juce::Justification::centredTop);
        xfadeGroup.xfadeGroupLabel.setText ("Crossfade\rGroup " + juce::String::charToString('A' + xfadeGroupIndex), juce::NotificationType::dontSendNotification);
#else
        xfadeGroup.xfadeGroupLabel.setJustificationType (juce::Justification::centredLeft);
        xfadeGroup.xfadeGroupLabel.setText (juce::String::charToString ('A' + xfadeGroupIndex), juce::NotificationType::dontSendNotification);
#endif
        addAndMakeVisible (xfadeGroup.xfadeGroupLabel);

        xfadeGroup.xfadeCvLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeCvLabel.setText ("CV:", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeCvLabel);
        xfadeGroup.xfadeCvComboBox.onChange = [this, xfadeGroupIndex] ()
        {
            xfadeCvUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeCvComboBox.getSelectedItemText ());
        };
        addAndMakeVisible (xfadeGroup.xfadeCvComboBox);

        xfadeGroup.xfadeWidthLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeWidthLabel.setText ("Width:", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeWidthLabel);
        xfadeGroup.xfadeWidthEditor.setIndents (2, 2);
        xfadeGroup.xfadeWidthEditor.setInputRestrictions (0, ".0123456789");
        xfadeGroup.xfadeWidthEditor.onFocusLost = [this, xfadeGroupIndex] () { xfadeWidthUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeWidthEditor.getText ()); };
        xfadeGroup.xfadeWidthEditor.onReturnKey = [this, xfadeGroupIndex] () { xfadeWidthUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeWidthEditor.getText ()); };
        addAndMakeVisible (xfadeGroup.xfadeWidthEditor);
    }
}

void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    setupPresetPropertiesCallbacks ();
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    auto getDoubleString = [this] (const juce::Identifier& propertyId, double value)
    {
        if (presetProperties.getValueTree ().hasProperty (propertyId))
            return juce::String (value);
        return juce::String ();
    };

    nameDataChanged (presetProperties.getName ());
    data2AsCvDataChanged (presetProperties.getData2AsCV ());
    xfadeCvDataChanged (0, presetProperties.getXfadeACV());
    xfadeWidthDataChanged (0, getDoubleString (PresetProperties::XfadeAWidthPropertyId, presetProperties.getXfadeAWidth ()));
    xfadeCvDataChanged (1, presetProperties.getXfadeBCV ());
    xfadeWidthDataChanged (1, getDoubleString (PresetProperties::XfadeAWidthPropertyId, presetProperties.getXfadeBWidth ()));
    xfadeCvDataChanged (2, presetProperties.getXfadeCCV ());
    xfadeWidthDataChanged (2, getDoubleString (PresetProperties::XfadeAWidthPropertyId, presetProperties.getXfadeCWidth ()));
    xfadeCvDataChanged (3, presetProperties.getXfadeDCV ());
    xfadeWidthDataChanged (3, getDoubleString (PresetProperties::XfadeAWidthPropertyId, presetProperties.getXfadeDWidth ()));
}

void Assimil8orEditorComponent::setupPresetPropertiesCallbacks ()
{
    auto getDoubleString = [this] (const juce::Identifier& propertyId, double value)
    {
        if (presetProperties.getValueTree ().hasProperty (propertyId))
            return juce::String (value);
        return juce::String ();
    };
    presetProperties.onNameChange = [this] (juce::String name) { nameDataChanged (name); };
    presetProperties.onData2AsCVChange = [this] (juce::String name) { data2AsCvDataChanged (name); };
    // Xfade_CV
    presetProperties.onXfadeACVChange = [this] (juce::String name) { xfadeCvDataChanged (0, name); };
    presetProperties.onXfadeBCVChange = [this] (juce::String name) { xfadeCvDataChanged (1, name); };
    presetProperties.onXfadeCCVChange = [this] (juce::String name) { xfadeCvDataChanged (2, name); };
    presetProperties.onXfadeDCVChange = [this] (juce::String name) { xfadeCvDataChanged (3, name); };
    // Xfade_Width
    presetProperties.onXfadeAWidthChange = [this, getDoubleString] (double width) { xfadeWidthDataChanged (0, getDoubleString (PresetProperties::XfadeAWidthPropertyId, width)); };
    presetProperties.onXfadeBWidthChange = [this, getDoubleString] (double width) { xfadeWidthDataChanged (1, getDoubleString (PresetProperties::XfadeBWidthPropertyId, width)); };
    presetProperties.onXfadeCWidthChange = [this, getDoubleString] (double width) { xfadeWidthDataChanged (2, getDoubleString (PresetProperties::XfadeCWidthPropertyId, width)); };
    presetProperties.onXfadeDWidthChange = [this, getDoubleString] (double width) { xfadeWidthDataChanged (3, getDoubleString (PresetProperties::XfadeDWidthPropertyId, width)); };
}

void Assimil8orEditorComponent::importPreset ()
{
    jassertfalse;
}

void Assimil8orEditorComponent::exportPreset ()
{
    jassertfalse;
}

juce::String Assimil8orEditorComponent::getPresetFileName (int presetIndex)
{
    const auto rawPresetIndexString { juce::String (presetIndex) };
    const auto presetIndexString { juce::String ("000").substring (0, 3 - rawPresetIndexString.length ()) + rawPresetIndexString };
    return "prst" + presetIndexString;
}

void Assimil8orEditorComponent::savePreset ()
{
    auto presetFile { juce::File (appProperties.getMRUList () [0]) };
    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.write (presetFile, presetProperties.getValueTree ());
}

void Assimil8orEditorComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey.darker (0.7f));
    //g.setColour (juce::Colours::red);
    //g.drawRect (xfadeGroupSectionLabel.getBounds ());
#if ! XFADE_GROUPS_HORIZONTAL
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (10, xfadeGroupSectionLabel.getBottom (), xfadeGroupSectionLabel.getWidth (),
                            xfadeGroups [3].xfadeWidthEditor.getBottom () - xfadeGroupSectionLabel.getBottom () + 2, 0.8, 1.0);
#endif
}

void Assimil8orEditorComponent::resized ()
{
    const auto yOffsetBetweenControls { 3 };
    auto localBounds { getLocalBounds () };

    auto buttonRow { localBounds.removeFromBottom (28).withTrimmedLeft (3).withTrimmedBottom (3)};
    saveButton.setBounds (buttonRow.removeFromLeft (100));
    buttonRow.removeFromLeft (3);
    importButton.setBounds (buttonRow.removeFromLeft (100));
    buttonRow.removeFromLeft (3);
    exportButton.setBounds (buttonRow.removeFromLeft (100));

    // Name
    nameEditor.setBounds ({ 10, 10, 150, 25 });

    // Data2 as CV
    data2AsCvLabel.setBounds (10, nameEditor.getBottom () + yOffsetBetweenControls, 80, 25);
    data2AsCvComboBox.setBounds (data2AsCvLabel.getRight () + 3, data2AsCvLabel.getY () + 3, 67, 20);

    // Cross fade groups
#if ! XFADE_GROUPS_HORIZONTAL
    xfadeGroupSectionLabel.setBounds (10, data2AsCvLabel.getBottom () + 3, data2AsCvComboBox.getRight() - data2AsCvLabel.getX (), 20);
#endif

    auto startX { nameEditor.getRight () };
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };
#if XFADE_GROUPS_HORIZONTAL
        xfadeGroup.xfadeGroupLabel.setBounds (startX + 18 + (xfadeGroupIndex * 100), nameEditor.getY (), 100, 35);

        xfadeGroup.xfadeCvLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), xfadeGroup.xfadeGroupLabel.getBottom () + 3, 20, 20);
        xfadeGroup.xfadeCvComboBox.setBounds (xfadeGroup.xfadeCvLabel.getRight () + 3, xfadeGroup.xfadeCvLabel.getY (), 60, 20);

        xfadeGroup.xfadeWidthLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), xfadeGroup.xfadeCvLabel.getBottom () + 3, 40, 20);
        xfadeGroup.xfadeWidthEditor.setBounds (xfadeGroup.xfadeWidthLabel.getRight () + 3, xfadeGroup.xfadeWidthLabel.getY (), 40, 20);
#else
        const auto groupHeight { 55 };

        xfadeGroup.xfadeGroupLabel.setBounds (20, xfadeGroupSectionLabel.getBottom() + 4 + (xfadeGroupIndex * groupHeight) + 7, 10, 35);

        xfadeGroup.xfadeCvLabel.setBounds (xfadeGroup.xfadeGroupLabel.getRight () + 6, xfadeGroupSectionLabel.getBottom () + 3 + xfadeGroupIndex * groupHeight, 20, 20);
        xfadeGroup.xfadeCvComboBox.setBounds (xfadeGroup.xfadeCvLabel.getRight () + 3, xfadeGroup.xfadeCvLabel.getY (), 60, 20);

        xfadeGroup.xfadeWidthLabel.setBounds (xfadeGroup.xfadeGroupLabel.getRight () + 6, xfadeGroup.xfadeCvLabel.getBottom () + 3, 40, 20);
        xfadeGroup.xfadeWidthEditor.setBounds (xfadeGroup.xfadeWidthLabel.getRight () + 3, xfadeGroup.xfadeWidthLabel.getY (), 40, 20);
#endif
    }
}

void Assimil8orEditorComponent::nameDataChanged (juce::String name)
{
    nameEditor.setText (name, false);
}

void Assimil8orEditorComponent::nameUiChanged (juce::String name)
{
    presetProperties.setName (name, false);
}

void Assimil8orEditorComponent::data2AsCvDataChanged (juce::String data2AsCvString)
{
    data2AsCvComboBox.setSelectedItemText (data2AsCvString);
}

void Assimil8orEditorComponent::data2AsCvUiChanged (juce::String data2AsCvString)
{
    presetProperties.setData2AsCV (data2AsCvString, false);
}

void Assimil8orEditorComponent::xfadeCvDataChanged (int group, juce::String data2AsCvString)
{
    jassert (group >= 0 && group < 4);
    xfadeGroups [group].xfadeCvComboBox.setSelectedItemText (data2AsCvString);
}

void Assimil8orEditorComponent::xfadeCvUiChanged (int group, juce::String data2AsCvString)
{
    switch (group)
    {
        case XfadeGroupIndex::groupA : presetProperties.setXfadeACV (data2AsCvString, false); break;
        case XfadeGroupIndex::groupB : presetProperties.setXfadeBCV (data2AsCvString, false); break;
        case XfadeGroupIndex::groupC : presetProperties.setXfadeCCV (data2AsCvString, false); break;
        case XfadeGroupIndex::groupD : presetProperties.setXfadeDCV (data2AsCvString, false); break;
        default: jassertfalse; break;
    }
}

void Assimil8orEditorComponent::xfadeWidthDataChanged (int group, juce::String widthString)
{
    jassert (group >= 0 && group < 4);
    xfadeGroups [group].xfadeWidthEditor.setText (widthString);
}

void Assimil8orEditorComponent::xfadeWidthUiChanged (int group, juce::String widthString)
{
    switch (group)
    {
        case XfadeGroupIndex::groupA: presetProperties.setXfadeAWidth (widthString.getDoubleValue (), false); break;
        case XfadeGroupIndex::groupB: presetProperties.setXfadeBWidth (widthString.getDoubleValue (), false); break;
        case XfadeGroupIndex::groupC: presetProperties.setXfadeCWidth (widthString.getDoubleValue (), false); break;
        case XfadeGroupIndex::groupD: presetProperties.setXfadeDWidth (widthString.getDoubleValue (), false); break;
        default: jassertfalse; break;
    }
}
