#include "Assimil8orEditorComponent.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/PersistentRootProperties.h"

// TODO - short list
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
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };

        xfadeGroup.xfadeGroupLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeGroupLabel.setJustificationType (juce::Justification::centredTop);
        xfadeGroup.xfadeGroupLabel.setText ("Crossfade\rGroup " + juce::String::charToString('A' + xfadeGroupIndex), juce::NotificationType::dontSendNotification);
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

void Assimil8orEditorComponent::setupChannelControls ()
{
}

void Assimil8orEditorComponent::setupZoneControls ()
{
}

void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    setupPresetPropertiesCallbacks ();

    nameDataChanged (presetProperties.getName ());
    data2AsCvDataChanged (presetProperties.getData2AsCV ());
    xfadeCvDataChanged (0, presetProperties.getXfadeACV());
    xfadeCvDataChanged (1, presetProperties.getXfadeBCV ());
    xfadeCvDataChanged (2, presetProperties.getXfadeCCV ());
    xfadeCvDataChanged (3, presetProperties.getXfadeDCV ());
    xfadeWidthDataChanged (0, juce::String (presetProperties.getXfadeAWidth ()));
    xfadeWidthDataChanged (1, juce::String (presetProperties.getXfadeBWidth ()));
    xfadeWidthDataChanged (2, juce::String (presetProperties.getXfadeCWidth ()));
    xfadeWidthDataChanged (3, juce::String (presetProperties.getXfadeDWidth ()));
}

void Assimil8orEditorComponent::setupPresetPropertiesCallbacks ()
{
    presetProperties.onNameChange = [this] (juce::String name) { nameDataChanged (name); };
    presetProperties.onData2AsCVChange = [this] (juce::String cvInput) { data2AsCvDataChanged (cvInput); };
    // Xfade_CV
    presetProperties.onXfadeACVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (0, dataAndCv); };
    presetProperties.onXfadeBCVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (1, dataAndCv); };
    presetProperties.onXfadeCCVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (2, dataAndCv); };
    presetProperties.onXfadeDCVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (3, dataAndCv); };
    // Xfade_Width
    presetProperties.onXfadeAWidthChange = [this] (double width) { xfadeWidthDataChanged (0, juce::String (width)); };
    presetProperties.onXfadeBWidthChange = [this] (double width) { xfadeWidthDataChanged (1, juce::String (width)); };
    presetProperties.onXfadeCWidthChange = [this] (double width) { xfadeWidthDataChanged (2, juce::String (width)); };
    presetProperties.onXfadeDWidthChange = [this] (double width) { xfadeWidthDataChanged (3, juce::String (width)); };
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

    auto startX { nameEditor.getRight () };
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };
        xfadeGroup.xfadeGroupLabel.setBounds (startX + 18 + (xfadeGroupIndex * 100), nameEditor.getY (), 100, 35);

        xfadeGroup.xfadeCvLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), xfadeGroup.xfadeGroupLabel.getBottom () + 3, 20, 20);
        xfadeGroup.xfadeCvComboBox.setBounds (xfadeGroup.xfadeCvLabel.getRight () + 3, xfadeGroup.xfadeCvLabel.getY (), 60, 20);

        xfadeGroup.xfadeWidthLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), xfadeGroup.xfadeCvLabel.getBottom () + 3, 40, 20);
        xfadeGroup.xfadeWidthEditor.setBounds (xfadeGroup.xfadeWidthLabel.getRight () + 3, xfadeGroup.xfadeWidthLabel.getY (), 40, 20);
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
    juce::Logger::outputDebugString ("Assimil8orEditorComponent::xfadeWidthDataChanged: group: " + juce::String(group) + "= '" + widthString + "'");
    jassert (group >= 0 && group < 4);
    jassert (widthString != "0");
    xfadeGroups [group].xfadeWidthEditor.setText (widthString);
}

void Assimil8orEditorComponent::xfadeWidthUiChanged (int group, juce::String widthString)
{
    juce::Logger::outputDebugString ("Assimil8orEditorComponent::xfadeWidthUiChanged : group: " + juce::String (group) + "= '" + widthString + "'");
    switch (group)
    {
        case XfadeGroupIndex::groupA: presetProperties.setXfadeAWidth (widthString.getDoubleValue (), false); break;
        case XfadeGroupIndex::groupB: presetProperties.setXfadeBWidth (widthString.getDoubleValue (), false); break;
        case XfadeGroupIndex::groupC: presetProperties.setXfadeCWidth (widthString.getDoubleValue (), false); break;
        case XfadeGroupIndex::groupD: presetProperties.setXfadeDWidth (widthString.getDoubleValue (), false); break;
        default: jassertfalse; break;
    }
}
