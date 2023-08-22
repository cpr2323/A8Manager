#include "Assimil8orEditorComponent.h"
#include "FormatHelpers.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/PersistentRootProperties.h"

// TODO - short list
//  Implement input validation, characters, min/max, etc
//  Implement proper formatting (decimal places, sign character, etc)
//  Implement tool tips for each parameter
//  Visual Edited indicator
//  Query for save if switching from edited Preset
//  Only enable Save button if preset was edited
//  Update Preset List (switch from dim to highlighted) when new preset created

Assimil8orEditorComponent::Assimil8orEditorComponent ()
{
    setOpaque (true);

    addAndMakeVisible (titleLabel);
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

    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
        channelTabs.addTab ("CH " + juce::String::charToString ('1' + curChannelIndex), juce::Colours::darkgrey, &channelEditors [curChannelIndex], false);
    addAndMakeVisible (channelTabs);

    minPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MinParameterPresetType),
                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    maxPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);

     setupPresetComponents ();
}

void Assimil8orEditorComponent::setupPresetComponents ()
{
    titleLabel.setText ("Preset _ :", juce::NotificationType::dontSendNotification);

    // NAME EDITOR
    //  length
    //  valid characters
    //  names doe not have to be unique, as the preset number is unique
    nameEditor.setJustification (juce::Justification::centredLeft);
    nameEditor.setIndents (1, 0);
    nameEditor.onFocusLost = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { nameUiChanged (nameEditor.getText ()); };
    //nameEditor.setInputRestrictions (0, ".0123456789");
    addAndMakeVisible (nameEditor);

    addAndMakeVisible (windowDecorator);
    data2AsCvLabel.setBorderSize ({ 1, 0, 1, 0 });
    data2AsCvLabel.setText ("Data2 As CV", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (data2AsCvLabel);
    data2AsCvComboBox.onChange = [this] ()
    {
        data2AsCvUiChanged (data2AsCvComboBox.getSelectedItemText ());
    };
    addAndMakeVisible (data2AsCvComboBox);

    xfadeGroupsLabel.setText ("XFade:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (xfadeGroupsLabel);

    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };

        xfadeGroup.xfadeGroupLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeGroupLabel.setText (juce::String::charToString ('A' + xfadeGroupIndex) + ":", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeGroupLabel);

        xfadeGroup.xfadeCvLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeCvLabel.setText ("CV", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeCvLabel);
        xfadeGroup.xfadeCvComboBox.onChange = [this, xfadeGroupIndex] ()
        {
            xfadeCvUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeCvComboBox.getSelectedItemText ());
        };
        addAndMakeVisible (xfadeGroup.xfadeCvComboBox);

        // xfade group width
        //      1 decimal place when above 1.0, 0.1 increment
        //      2 decimal places below 1.0, 0.01 increment
        //      10.V
        //      9.0V
        //      .99V
        xfadeGroup.xfadeWidthLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeWidthLabel.setText ("Width", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeWidthLabel);
        xfadeGroup.xfadeWidthEditor.setJustification (juce::Justification::centred);
        xfadeGroup.xfadeWidthEditor.setIndents (0, 0);
        xfadeGroup.xfadeWidthEditor.setInputRestrictions (0, ".0123456789");
        auto xFadeGroupEditDone = [this] (int xfadeGroupIndex)
        {
            auto& widthEditor { xfadeGroups [xfadeGroupIndex].xfadeWidthEditor };
            const auto value { widthEditor.getText ().getDoubleValue () };
            const auto numDecimals { value >= 1.0 ? 1 : 2 };
            auto newText { FormatHelpers::checkAndFormat (widthEditor.getText ().getDoubleValue (), minPresetProperties.getXfadeAWidth (), maxPresetProperties.getXfadeAWidth (), numDecimals, false) };
            if (value == 10)
                newText = newText.trimCharactersAtEnd ("0");
            else if (value < 1.0)
                newText = newText.trimCharactersAtStart ("0");
            newText += "V";
            widthEditor.setText (newText);
            xfadeWidthUiChanged (xfadeGroupIndex, widthEditor.getText ());
        };
        xfadeGroup.xfadeWidthEditor.onFocusLost = [this, xfadeGroupIndex, xFadeGroupEditDone] ()
        {
            xFadeGroupEditDone (xfadeGroupIndex);
        };
        xfadeGroup.xfadeWidthEditor.onReturnKey = [this, xfadeGroupIndex, xFadeGroupEditDone] ()
        {
            xFadeGroupEditDone (xfadeGroupIndex);
        };
        xfadeGroup.xfadeWidthEditor.onTextChange = [this, xfadeGroupIndex] ()
        {
            FormatHelpers::setColorIfError (xfadeGroups [xfadeGroupIndex].xfadeWidthEditor, minPresetProperties.getXfadeAWidth (), maxPresetProperties.getXfadeAWidth ());
        };
        addAndMakeVisible (xfadeGroup.xfadeWidthEditor);
    }
}

void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    setupPresetPropertiesCallbacks ();
    auto channelEditorIndex { 0 };
    presetProperties.forEachChannel ([this, &channelEditorIndex] (juce::ValueTree channelPropertiesVT)
    {
        channelEditors [channelEditorIndex].init (channelPropertiesVT);
        ++channelEditorIndex;
        return true;
    });

    indexDataChanged (presetProperties.getIndex ());
    nameDataChanged (presetProperties.getName ());
    data2AsCvDataChanged (presetProperties.getData2AsCV ());
    xfadeCvDataChanged (0, presetProperties.getXfadeACV ());
    xfadeCvDataChanged (1, presetProperties.getXfadeBCV ());
    xfadeCvDataChanged (2, presetProperties.getXfadeCCV ());
    xfadeCvDataChanged (3, presetProperties.getXfadeDCV ());
    xfadeWidthDataChanged (0, juce::String (presetProperties.getXfadeAWidth (), 2)); // + "V");
    xfadeWidthDataChanged (1, juce::String (presetProperties.getXfadeBWidth (), 2)); // + "V");
    xfadeWidthDataChanged (2, juce::String (presetProperties.getXfadeCWidth (), 2)); // + "V");
    xfadeWidthDataChanged (3, juce::String (presetProperties.getXfadeDWidth (), 2)); // + "V");
}

void Assimil8orEditorComponent::setupPresetPropertiesCallbacks ()
{
    presetProperties.onIndexChange = [this] (int index) { indexDataChanged (index); };
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
    auto localBounds { getLocalBounds () };

    auto topRow { localBounds.removeFromTop (25) };
    topRow.removeFromTop (3);
    topRow.removeFromLeft (5);
    titleLabel.setBounds (topRow.removeFromLeft (75));
    topRow.removeFromLeft (3);
    // Name
    nameEditor.setBounds (topRow.removeFromLeft (150));

    topRow.removeFromRight (3);
    exportButton.setBounds (topRow.removeFromRight (75));
    topRow.removeFromRight (3);
    importButton.setBounds (topRow.removeFromRight (75));
    topRow.removeFromRight (3);
    saveButton.setBounds (topRow.removeFromRight (75));
    const auto tabHeight (600);
    const auto topRowY { titleLabel.getBottom () + 3 };
    channelTabs.setBounds (3, topRowY, 800, tabHeight);
    const auto bottomRowY (getLocalBounds ().getBottom () - 26);
    windowDecorator.setBounds (getLocalBounds ().removeFromBottom (26));

    // Data2 as CV
    data2AsCvLabel.setBounds (6, bottomRowY + 3, 80, 20);
    data2AsCvComboBox.setBounds (data2AsCvLabel.getRight () + 1, bottomRowY + 3, 28, 20);

    // Cross fade groups
    xfadeGroupsLabel.setBounds (data2AsCvComboBox.getRight () + 5, bottomRowY + 3, 50, 20);
    auto startX { xfadeGroupsLabel.getRight () + 2 };
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };
        xfadeGroup.xfadeGroupLabel.setBounds (startX + (xfadeGroupIndex * 155), bottomRowY + 3, 17, 20);

        xfadeGroup.xfadeCvLabel.setBounds (xfadeGroup.xfadeGroupLabel.getRight (), bottomRowY + 3, 20, 20);
        xfadeGroup.xfadeCvComboBox.setBounds (xfadeGroup.xfadeCvLabel.getRight () + 1, bottomRowY + 3, 28, 20);

        xfadeGroup.xfadeWidthLabel.setBounds (xfadeGroup.xfadeCvComboBox.getRight () + 3, bottomRowY + 3, 35, 20);
        xfadeGroup.xfadeWidthEditor.setBounds (xfadeGroup.xfadeWidthLabel.getRight () + 1, bottomRowY + 3, 40, 20);
    }
}

void Assimil8orEditorComponent::indexDataChanged (int index)
{
    titleLabel.setText ("Preset " + juce::String (index) + " :", juce::NotificationType::dontSendNotification);
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
