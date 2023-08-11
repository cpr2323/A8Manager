#include "Assimil8orEditorComponent.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/PersistentRootProperties.h"

// TODO - short list
//  Visual Edited indicator
//  Query for save if switching from edited Preset
//  Only enable Save button if preset was edited
//  Update Preset List (switch from dim to highlighted) when new preset created
//  Add editor fields

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

    data2AsCvLabel.setText ("Data2 As CV", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (data2AsCvLabel);
    {
        auto menuId { 1 };
        data2AsCvComboBox.addItem ("Off", menuId);
        ++menuId;
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
            for (auto columnIndex { 0 }; columnIndex < 3; ++columnIndex)
            {
                data2AsCvComboBox.addItem (juce::String::charToString('1' + channelIndex) + juce::String::charToString ('A' + columnIndex), menuId);
                ++menuId;
            }
    }
    data2AsCvComboBox.onChange = [this] ()
    {
        data2AsCvUiChanged (data2AsCvComboBox.getItemText (data2AsCvComboBox.getSelectedItemIndex ()));
    };
    addAndMakeVisible (data2AsCvComboBox);
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };

        xfadeGroup.xfadeGroupLabel.setText ("Crossfade\rGroup " + juce::String::charToString('A' + xfadeGroupIndex), juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeGroupLabel);

        xfadeGroup.xfadeCvLabel.setText ("CV:", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeCvLabel);
        addAndMakeVisible (xfadeGroup.xfadeCvEditor);

        xfadeGroup.xfadeWidthLabel.setText ("Width:", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeWidthLabel);
        addAndMakeVisible (xfadeGroup.xfadeWidthLabel);
    }
}
void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.onNameChange = [this] (juce::String name) { nameDataChanged (name); };
    presetProperties.onData2AsCVChange = [this] (juce::String name) { data2AsCvDataChanged (name); };
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);

    nameDataChanged (presetProperties.getName ());
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

    nameEditor.setBounds ({ 10, 10, 150, 25 });

    data2AsCvLabel.setBorderSize ({ 1, 0, 1, 0 });
    data2AsCvLabel.setBounds (10, nameEditor.getBottom () + yOffsetBetweenControls, 80, 25);
    data2AsCvComboBox.setTextWhenNothingSelected ("");
    data2AsCvComboBox.setBounds (data2AsCvLabel.getRight () + 3, data2AsCvLabel.getY (), 67, 25);

    auto startX { nameEditor.getRight () };
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };
        xfadeGroup.xfadeGroupLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeGroupLabel.setJustificationType (juce::Justification::centredTop);
        xfadeGroup.xfadeGroupLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), nameEditor.getY (), 100, 35);

        xfadeGroup.xfadeCvLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeCvLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), xfadeGroup.xfadeGroupLabel.getBottom () + 3, 100, 20);
//        xfadeGroup.xfadeCvEditor.setBounds ();

        xfadeGroup.xfadeWidthLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeWidthLabel.setBounds (startX + 20 + (xfadeGroupIndex * 100), xfadeGroup.xfadeCvLabel.getBottom () + 3, 100, 20);
//        xfadeGroup.xfadeWidthLabel.setBounds ();
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
    auto itemId { 1 };
    if (data2AsCvString.isEmpty ())
    {
        data2AsCvComboBox.setText ("", juce::NotificationType::sendNotification);
        return;
    }
    if (data2AsCvString.toLowerCase () != "off")
        itemId = 2 + ((data2AsCvString [0] - '1') * 3) + data2AsCvString [1] - 'A';
    data2AsCvComboBox.setSelectedId (itemId, false);
}

void Assimil8orEditorComponent::data2AsCvUiChanged (juce::String data2AsCvString)
{
    presetProperties.setData2AsCV (data2AsCvString, false);
}
