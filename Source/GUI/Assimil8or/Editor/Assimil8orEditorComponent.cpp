#include "Assimil8orEditorComponent.h"
#include "FormatHelpers.h"
#include "ParameterToolTipData.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/PresetManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/PersistentRootProperties.h"
#include <algorithm>

// TODO - short list
//  Visual Edited indicator
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
    saveButton.setEnabled (false);
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

     startTimer (250);
}

void Assimil8orEditorComponent::setupPresetComponents ()
{
    juce::XmlDocument xmlDoc { BinaryData::Assimil8orToolTips_xml };
    auto xmlElement { xmlDoc.getDocumentElement (false) };
    if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
        juce::Logger::outputDebugString ("XML Parsing Error for Assimil8orToolTips_xml: " + parseError);
    // NOTE: this is a hard failure, which indicates there is a problem in the file the parameterPresetXml passed in
    jassert (xmlDoc.getLastParseError () == "");
    auto toolTipsVT { juce::ValueTree::fromXml (*xmlElement) };
    ParameterToolTipData parameterToolTipData (toolTipsVT, ParameterToolTipData::WrapperType::owner, ParameterToolTipData::EnableCallbacks::no);

    titleLabel.setText ("Preset _ :", juce::NotificationType::dontSendNotification);

    // NAME EDITOR
    //  length
    //  valid characters
    //  names doe not have to be unique, as the preset number is unique
    nameEditor.setJustification (juce::Justification::centredLeft);
    nameEditor.setIndents (1, 0);
    nameEditor.onFocusLost = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.setInputRestrictions (12, " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
    nameEditor.setTooltip (parameterToolTipData.getToolTip ("Preset", "Name"));
    addAndMakeVisible (nameEditor);

    addAndMakeVisible (windowDecorator);
    data2AsCvLabel.setBorderSize ({ 1, 0, 1, 0 });
    data2AsCvLabel.setText ("Data2 As", juce::NotificationType::dontSendNotification);
    data2AsCvLabel.setTooltip (parameterToolTipData.getToolTip ("Preset", "Data2asCV"));
    addAndMakeVisible (data2AsCvLabel);
    data2AsCvComboBox.onChange = [this] ()
    {
        data2AsCvUiChanged (data2AsCvComboBox.getSelectedItemText ());
    };
    data2AsCvComboBox.setTooltip (parameterToolTipData.getToolTip ("Preset", "Data2asCV"));
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
        xfadeGroup.xfadeCvLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
        xfadeGroup.xfadeCvLabel.setText ("CV", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeCvLabel);
        xfadeGroup.xfadeCvComboBox.onChange = [this, xfadeGroupIndex] ()
        {
            xfadeCvUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeCvComboBox.getSelectedItemText ());
        };
        // XfadeACV
        xfadeGroup.xfadeCvComboBox.setTooltip (parameterToolTipData.getToolTip ("Preset", "Xfade" + juce::String::charToString('A' + xfadeGroupIndex) + "CV"));
        addAndMakeVisible (xfadeGroup.xfadeCvComboBox);

        // xfade group width
        //      1 decimal place when above 1.0, 0.1 increment
        //      2 decimal places below 1.0, 0.01 increment
        //      10.V
        //      9.0V
        //      .99V
        xfadeGroup.xfadeWidthLabel.setBorderSize ({ 0, 0, 0, 0 });
        xfadeGroup.xfadeWidthLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
        xfadeGroup.xfadeWidthLabel.setText ("Width", juce::NotificationType::dontSendNotification);
        addAndMakeVisible (xfadeGroup.xfadeWidthLabel);
        xfadeGroup.xfadeWidthEditor.setJustification (juce::Justification::centred);
        xfadeGroup.xfadeWidthEditor.setIndents (0, 0);
        xfadeGroup.xfadeWidthEditor.setInputRestrictions (0, "+-.0123456789");
        auto xFadeGroupEditDone = [this] (int xfadeGroupIndex)
        {
            auto& widthEditor { xfadeGroups [xfadeGroupIndex].xfadeWidthEditor };
            auto width { std::clamp (widthEditor.getText ().getDoubleValue (), minPresetProperties.getXfadeAWidth (), maxPresetProperties.getXfadeAWidth ()) };
            xfadeWidthUiChanged (xfadeGroupIndex, width);

            auto text { formatXfadeWidthString (width) };
            widthEditor.setText (text);
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
        xfadeGroup.xfadeWidthEditor.setTooltip (parameterToolTipData.getToolTip ("Preset", "Xfade" + juce::String::charToString ('A' + xfadeGroupIndex) + "Width"));
        addAndMakeVisible (xfadeGroup.xfadeWidthEditor);
    }
}

juce::String Assimil8orEditorComponent::formatXfadeWidthString (double width)
    {
        const auto numDecimals { width >= 1.0 ? 1 : 2 };
        auto newText { FormatHelpers::formatDouble (width, numDecimals, false) };
        if (width == 10)
            newText = newText.trimCharactersAtEnd ("0");
        else if (width < 1.0)
            newText = newText.trimCharactersAtStart ("0");
        return newText += "V";
    };

void Assimil8orEditorComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    runtimeRootProperties.wrap (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::yes);
    runtimeRootProperties.onSystemRequestedQuit = [this] ()
    {
        runtimeRootProperties.setPreferredQuitState (RuntimeRootProperties::QuitState::idle, false);
        overwritePresetOrCancel ([this] ()
        {
            juce::MessageManager::callAsync ([this] () { runtimeRootProperties.setQuitState (RuntimeRootProperties::QuitState::now, false); });
        }, [this] ()
        {
            // do nothing
        });
    };

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);

    unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.wrap (presetManagerProperties.getPreset("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    setupPresetPropertiesCallbacks ();
    auto channelEditorIndex { 0 };
    presetProperties.forEachChannel ([this, &channelEditorIndex, rootPropertiesVT] (juce::ValueTree channelPropertiesVT)
    {
        channelEditors [channelEditorIndex].init (channelPropertiesVT, rootPropertiesVT);
        ++channelEditorIndex;
        return true;
    });

    idDataChanged (presetProperties.getId ());
    nameDataChanged (presetProperties.getName ());
    data2AsCvDataChanged (presetProperties.getData2AsCV ());
    xfadeCvDataChanged (0, presetProperties.getXfadeACV ());
    xfadeCvDataChanged (1, presetProperties.getXfadeBCV ());
    xfadeCvDataChanged (2, presetProperties.getXfadeCCV ());
    xfadeCvDataChanged (3, presetProperties.getXfadeDCV ());
    xfadeWidthDataChanged (0, presetProperties.getXfadeAWidth ());
    xfadeWidthDataChanged (1, presetProperties.getXfadeBWidth ());
    xfadeWidthDataChanged (2, presetProperties.getXfadeCWidth ());
    xfadeWidthDataChanged (3, presetProperties.getXfadeDWidth ());
}

void Assimil8orEditorComponent::setupPresetPropertiesCallbacks ()
{
    presetProperties.onIdChange = [this] (int id) { idDataChanged (id); };
    presetProperties.onNameChange = [this] (juce::String name) { nameDataChanged (name); };
    presetProperties.onData2AsCVChange = [this] (juce::String cvInput) { data2AsCvDataChanged (cvInput); };
    // Xfade_CV
    presetProperties.onXfadeACVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (0, dataAndCv); };
    presetProperties.onXfadeBCVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (1, dataAndCv); };
    presetProperties.onXfadeCCVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (2, dataAndCv); };
    presetProperties.onXfadeDCVChange = [this] (juce::String dataAndCv) { xfadeCvDataChanged (3, dataAndCv); };
    // Xfade_Width
    presetProperties.onXfadeAWidthChange = [this] (double width) { xfadeWidthDataChanged (0, width); };
    presetProperties.onXfadeBWidthChange = [this] (double width) { xfadeWidthDataChanged (1, width); };
    presetProperties.onXfadeCWidthChange = [this] (double width) { xfadeWidthDataChanged (2, width); };
    presetProperties.onXfadeDWidthChange = [this] (double width) { xfadeWidthDataChanged (3, width); };
}

void Assimil8orEditorComponent::importPreset ()
{
    jassertfalse;
}

void Assimil8orEditorComponent::exportPreset ()
{
    jassertfalse;
}

void Assimil8orEditorComponent::receiveSampleLoadRequest (juce::File sampleFile)
{
    auto channelIndex { channelTabs.getCurrentTabIndex () };
    auto curChannelEditor { dynamic_cast<ChannelEditor*>(channelTabs.getTabContentComponent (channelIndex)) };
    curChannelEditor->receiveSampleLoadRequest (sampleFile);
}

// TODO - add options to check Preset Id as well
bool Assimil8orEditorComponent::arePresetsEqual (juce::ValueTree presetOneVT, juce::ValueTree presetTwoVT)
{
    auto arePresetsEqual = [this] (PresetProperties& presetPropertiesOne, PresetProperties& presetPropertiesTwo)
    {
        return  presetPropertiesOne.getData2AsCV () == presetPropertiesTwo.getData2AsCV () &&
                presetPropertiesOne.getName () == presetPropertiesTwo.getName () &&
                presetPropertiesOne.getXfadeACV () == presetPropertiesTwo.getXfadeACV () &&
                presetPropertiesOne.getXfadeAWidth () == presetPropertiesTwo.getXfadeAWidth () &&
                presetPropertiesOne.getXfadeBCV () == presetPropertiesTwo.getXfadeBCV () &&
                presetPropertiesOne.getXfadeBWidth () == presetPropertiesTwo.getXfadeBWidth () &&
                presetPropertiesOne.getXfadeCCV () == presetPropertiesTwo.getXfadeCCV () &&
                presetPropertiesOne.getXfadeCWidth () == presetPropertiesTwo.getXfadeCWidth () &&
                presetPropertiesOne.getXfadeDCV () == presetPropertiesTwo.getXfadeDCV () &&
                presetPropertiesOne.getXfadeDWidth () == presetPropertiesTwo.getXfadeDWidth ();
    };
    auto areChannelsEqual = [this] (ChannelProperties& channelPropertiesOne, ChannelProperties& channelPropertiesTwo)
    {
        return channelPropertiesOne.getAliasing () == channelPropertiesTwo.getAliasing () &&
                channelPropertiesOne.getAliasingMod () == channelPropertiesTwo.getAliasingMod () &&
                channelPropertiesOne.getAttack () == channelPropertiesTwo.getAttack () &&
                channelPropertiesOne.getAttackFromCurrent () == channelPropertiesTwo.getAttackFromCurrent () &&
                channelPropertiesOne.getAttackMod () == channelPropertiesTwo.getAttackMod () &&
                channelPropertiesOne.getAutoTrigger () == channelPropertiesTwo.getAutoTrigger () &&
                channelPropertiesOne.getBits () == channelPropertiesTwo.getBits () &&
                channelPropertiesOne.getBitsMod () == channelPropertiesTwo.getBitsMod () &&
                channelPropertiesOne.getChannelMode () == channelPropertiesTwo.getChannelMode () &&
                channelPropertiesOne.getExpAM () == channelPropertiesTwo.getExpAM () &&
                channelPropertiesOne.getExpFM () == channelPropertiesTwo.getExpFM () &&
                channelPropertiesOne.getLevel () == channelPropertiesTwo.getLevel () &&
                channelPropertiesOne.getLinAM () == channelPropertiesTwo.getLinAM () &&
                channelPropertiesOne.getLinAMisExtEnv () == channelPropertiesTwo.getLinAMisExtEnv () &&
                channelPropertiesOne.getLinFM () == channelPropertiesTwo.getLinFM () &&
                channelPropertiesOne.getLoopLengthIsEnd () == channelPropertiesTwo.getLoopLengthIsEnd () &&
                channelPropertiesOne.getLoopLengthMod () == channelPropertiesTwo.getLoopLengthMod () &&
                channelPropertiesOne.getLoopMode () == channelPropertiesTwo.getLoopMode () &&
                channelPropertiesOne.getLoopStartMod () == channelPropertiesTwo.getLoopStartMod () &&
                channelPropertiesOne.getMixLevel () == channelPropertiesTwo.getMixLevel () &&
                channelPropertiesOne.getMixMod () == channelPropertiesTwo.getMixMod () &&
                channelPropertiesOne.getMixModIsFader () == channelPropertiesTwo.getMixModIsFader () &&
                channelPropertiesOne.getPan () == channelPropertiesTwo.getPan () &&
                channelPropertiesOne.getPanMod () == channelPropertiesTwo.getPanMod () &&
                channelPropertiesOne.getPhaseCV () == channelPropertiesTwo.getPhaseCV () &&
                channelPropertiesOne.getPitch () == channelPropertiesTwo.getPitch () &&
                channelPropertiesOne.getPitchCV () == channelPropertiesTwo.getPitchCV () &&
                channelPropertiesOne.getPlayMode () == channelPropertiesTwo.getPlayMode () &&
                channelPropertiesOne.getPMIndex () == channelPropertiesTwo.getPMIndex () &&
                channelPropertiesOne.getPMIndexMod () == channelPropertiesTwo.getPMIndexMod () &&
                channelPropertiesOne.getPMSource () == channelPropertiesTwo.getPMSource () &&
                channelPropertiesOne.getRelease () == channelPropertiesTwo.getRelease () &&
                channelPropertiesOne.getReleaseMod () == channelPropertiesTwo.getReleaseMod () &&
                channelPropertiesOne.getReverse () == channelPropertiesTwo.getReverse () &&
                channelPropertiesOne.getSampleStartMod () == channelPropertiesTwo.getSampleStartMod () &&
                channelPropertiesOne.getSampleEndMod () == channelPropertiesTwo.getSampleEndMod () &&
                channelPropertiesOne.getSpliceSmoothing () == channelPropertiesTwo.getSpliceSmoothing () &&
                channelPropertiesOne.getXfadeGroup () == channelPropertiesTwo.getXfadeGroup () &&
                channelPropertiesOne.getZonesCV () == channelPropertiesTwo.getZonesCV () &&
                channelPropertiesOne.getZonesRT () == channelPropertiesTwo.getZonesRT ();
    };
    auto areZonesEqual = [this] (ZoneProperties& zonePropertiesOne, ZoneProperties& zonePropertiesTwo)
    {
        return zonePropertiesOne.getLevelOffset () == zonePropertiesTwo.getLevelOffset () &&
                zonePropertiesOne.getLoopLength () == zonePropertiesTwo.getLoopLength () &&
                zonePropertiesOne.getLoopStart () == zonePropertiesTwo.getLoopStart () &&
                zonePropertiesOne.getMinVoltage () == zonePropertiesTwo.getMinVoltage () &&
                zonePropertiesOne.getPitchOffset () == zonePropertiesTwo.getPitchOffset () &&
                zonePropertiesOne.getSample () == zonePropertiesTwo.getSample () &&
                zonePropertiesOne.getSampleStart () == zonePropertiesTwo.getSampleStart () &&
                zonePropertiesOne.getSampleEnd () == zonePropertiesTwo.getSampleEnd () &&
                zonePropertiesOne.getSide () == zonePropertiesTwo.getSide ();
    };

    auto presetsAreEqual { true };
    PresetProperties presetOne (presetOneVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    PresetProperties presetTwo (presetTwoVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    if (arePresetsEqual (presetOne, presetTwo))
    {
        for (auto channelIndex { 0 }; channelIndex < 8 && presetsAreEqual; ++channelIndex)
        {
            ChannelProperties presetOneChannelProperties (presetOne.getChannelVT (channelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            ChannelProperties presetTwoChannelProperties (presetTwo.getChannelVT (channelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            if (!areChannelsEqual (presetOneChannelProperties, presetTwoChannelProperties))
                presetsAreEqual = false;
            for (auto zoneIndex { 0 }; zoneIndex < 8 && presetsAreEqual; ++zoneIndex)
            {
                ZoneProperties presetOneZoneProperties (presetOneChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                ZoneProperties presetTwoZoneProperties (presetTwoChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                if (!areZonesEqual (presetOneZoneProperties, presetTwoZoneProperties))
                    presetsAreEqual = false;
            }
        }
    }

    return presetsAreEqual;
}

void Assimil8orEditorComponent::overwritePresetOrCancel (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
{
    jassert (overwriteFunction != nullptr);
    jassert (cancelFunction != nullptr);

    if (arePresetsEqual (unEditedPresetProperties.getValueTree(), presetProperties.getValueTree()))
    {
        overwriteFunction ();
    }
    else
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "Overwriting Edited Preset",
            "You are about to overwrite a preset that you have edited. Select OK to lose your changes, Select Cancel to go back and save.", "OK", "Cancel", nullptr,
            juce::ModalCallbackFunction::create ([this, overwriteFunction, cancelFunction] (int option)
            {
                if (option == 1)
                    overwriteFunction ();
                else
                    cancelFunction ();
            }));
    }
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
    const auto topRowY { titleLabel.getBottom () + 3 };
    channelTabs.setBounds (3, topRowY, 765, 400);
    const auto bottomRowY (getLocalBounds ().getBottom () - 26);
    windowDecorator.setBounds (getLocalBounds ().removeFromBottom (26));

    // Data2 as CV
    data2AsCvLabel.setBounds (6, bottomRowY + 3, 55, 20);
    data2AsCvComboBox.setBounds (data2AsCvLabel.getRight () + 3, bottomRowY + 3, 28, 20);

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

void Assimil8orEditorComponent::idDataChanged (int id)
{
    titleLabel.setText ("Preset " + juce::String (id) + " :", juce::NotificationType::dontSendNotification);
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

void Assimil8orEditorComponent::xfadeWidthDataChanged (int group, double width)
{
    jassert (group >= 0 && group < 4);
    xfadeGroups [group].xfadeWidthEditor.setText (formatXfadeWidthString(width));
}

void Assimil8orEditorComponent::xfadeWidthUiChanged (int group, double width)
{
    switch (group)
    {
        case XfadeGroupIndex::groupA: presetProperties.setXfadeAWidth (width, false); break;
        case XfadeGroupIndex::groupB: presetProperties.setXfadeBWidth (width, false); break;
        case XfadeGroupIndex::groupC: presetProperties.setXfadeCWidth (width, false); break;
        case XfadeGroupIndex::groupD: presetProperties.setXfadeDWidth (width, false); break;
        default: jassertfalse; break;
    }
}

void Assimil8orEditorComponent::timerCallback ()
{
    saveButton.setEnabled (! arePresetsEqual (unEditedPresetProperties.getValueTree (), presetProperties.getValueTree ()));
}
