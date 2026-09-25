#include "Assimil8orEditorComponent.h"
#include "ParameterToolTipData.h"
#include "../../Theme/A8ColourIds.h"
#include "../../../SystemServices.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/PresetManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Assimil8or/Preset/PresetHelpers.h"
#include "oolib/Debug/DebugLog.h"
#include "oolib/Debug/DumpStack.h"
#include "oolib/GUI/ErrorHelpers.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include <algorithm>

Assimil8orEditorComponent::Assimil8orEditorComponent ()
{
    setOpaque (true);

    // Title : PRESET X
    titleLabel.setBorderSize ({ 0, 0, 0, 0 });
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (A8Type::sectionHeader ());
    addAndMakeVisible (titleLabel);

    saveButton.onClick = [this] () { savePreset (); };
    saveButton.setTooltip ("Save the current Preset");
    saveButton.setEnabled (false);
    addAndMakeVisible (saveButton);

    for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
    {
        channelTabs.addLedTab ("CH " + juce::String::charToString ('1' + curChannelIndex), &channelEditors [curChannelIndex]);
        channelEditors [curChannelIndex].onWaveformExpandedChange = [this] (bool isExpanded) { setWaveformExpanded (isExpanded); };
    }
    channelTabs.setTabBarDepth (kTabBarHeight);
    addAndMakeVisible (channelTabs);

    toolsButton.setTooltip ("Preset Tools");
    toolsButton.onClick = [this] () { displayToolsMenu (); };
    addAndMakeVisible (toolsButton);

    minPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MinParameterPresetType),
                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    maxPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    defaultPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                  PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    defaultChannelProperties.wrap (defaultPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);

    setupPresetComponents ();

    // NOTE: This must go last to overlay the entire window when activated
    //addChildComponent (midiConfigWindow);

    startTimer (250);
}

// one setting across all of the channels, so the expanded waveform stays up while moving between them
void Assimil8orEditorComponent::setWaveformExpanded (bool isExpanded)
{
    for (auto& channelEditor : channelEditors)
        channelEditor.setWaveformExpanded (isExpanded);
}

void Assimil8orEditorComponent::setupPresetComponents ()
{
    juce::XmlDocument xmlDoc { BinaryData::Assimil8orToolTips_xml };
    auto xmlElement { xmlDoc.getDocumentElement (false) };
//     if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
//         juce::Logger::outputDebugString ("XML Parsing Error for Assimil8orToolTips_xml: " + parseError);
    // NOTE: this is a hard failure, which indicates there is a problem in the file the parameterPresetXml passed in
    jassert (xmlDoc.getLastParseError () == "");
    auto toolTipsVT { juce::ValueTree::fromXml (*xmlElement) };
    ParameterToolTipData parameterToolTipData (toolTipsVT, ParameterToolTipData::WrapperType::owner, ParameterToolTipData::EnableCallbacks::no);

    auto setupLabel = [this] (juce::Label& label, juce::String text, juce::Justification justification)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setFont (A8Type::parameterLabel ());
        label.setMinimumHorizontalScale (1.0f);
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };

    titleLabel.setText ("PRESET", juce::NotificationType::dontSendNotification);

    // NAME EDITOR
    //  length
    //  valid characters
    //  names doe not have to be unique, as the preset number is unique
    nameEditor.setJustification (juce::Justification::centredLeft);
    nameEditor.setIndents (7, 0);
    nameEditor.setFont (A8Type::nameField ());
    HoverHighlight::attach (nameEditor);
    nameEditor.onFocusLost = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.onReturnKey = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.onTextChange = [this] () { nameUiChanged (nameEditor.getText ()); };
    nameEditor.setInputRestrictions (12, " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
    nameEditor.setTooltip (parameterToolTipData.getToolTip ("Preset", "Name"));
    addAndMakeVisible (nameEditor);

    setupLabel (midiSetupLabel, "MIDI SETUP", juce::Justification::centredLeft);
    midiSetupLabel.setFont (A8Type::sectionHeader ());
    for (auto midiSetupId { 0 }; midiSetupId < 9; ++midiSetupId)
        midiSetupComboBox.addItem (juce::String (midiSetupId + 1), midiSetupId + 1);
    midiSetupComboBox.onChange = [this] ()
    {
        midiSetupUiChanged (midiSetupComboBox.getSelectedItemIndex ());
    };
    midiSetupComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        presetProperties.setMidiSetup (std::clamp (midiSetupComboBox.getSelectedItemIndex () + scrollAmount, 0, midiSetupComboBox.getNumItems () - 1), true);
    };
    midiSetupComboBox.onPopupMenuCallback = [this] ()
    {
    };
    HoverHighlight::attach (midiSetupComboBox);
    addAndMakeVisible (midiSetupComboBox);

    // the strip behind the preset wide controls along the bottom
    addAndMakeVisible (windowDecorator);

    // Data 2 CV
    setupLabel (data2AsCvLabel, "DATA2 AS", juce::Justification::centredLeft);
    data2AsCvLabel.setTooltip (parameterToolTipData.getToolTip ("Preset", "Data2asCV"));
    addAndMakeVisible (data2AsCvLabel);
    data2AsCvComboBox.onChange = [this] ()
    {
        data2AsCvUiChanged (data2AsCvComboBox.getSelectedItemText ());
    };
    data2AsCvComboBox.onDragCallback = [this] (double valueDelta)
    {
        const auto scrollAmount { static_cast<int> (valueDelta) };
        const auto newCvInputComboBoxIndex { data2AsCvComboBox.getSelectedItemIndex () + scrollAmount };
        data2AsCvComboBox.setSelectedItemIndex (std::clamp (newCvInputComboBoxIndex, 0, data2AsCvComboBox.getNumItems () - 1));
        presetProperties.setData2AsCV (data2AsCvComboBox.getSelectedItemText (), false);
    };
    data2AsCvComboBox.onPopupMenuCallback = [this] ()
    {
        juce::PopupMenu editMenu;
        editMenu.addItem ("Default", true, false, [this] () { presetProperties.setData2AsCV (defaultPresetProperties.getData2AsCV (), true); });
        editMenu.addItem ("Revert", true, false, [this] () { presetProperties.setData2AsCV (unEditedPresetProperties.getData2AsCV (), true); });
        editMenu.showMenuAsync ({}, [this] (int) {});

    };
    data2AsCvComboBox.setTooltip (parameterToolTipData.getToolTip ("Preset", "Data2asCV"));
    addAndMakeVisible (data2AsCvComboBox);

    setupLabel (xfadeGroupsLabel, "XFADE", juce::Justification::centredLeft);

    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };

        setupLabel (xfadeGroup.xfadeGroupLabel, juce::String::charToString ('A' + xfadeGroupIndex), juce::Justification::centredLeft);
        xfadeGroup.xfadeGroupLabel.setFont (A8Type::sectionHeader ());

        // Xfade Label
        setupLabel (xfadeGroup.xfadeCvLabel, "CV", juce::Justification::centredRight);

        // Xfade CV Input ComboBox
        xfadeGroup.xfadeCvComboBox.onChange = [this, xfadeGroupIndex] ()
        {
            xfadeCvUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeCvComboBox.getSelectedItemText ());
        };
        xfadeGroup.xfadeCvComboBox.onDragCallback = [this, xfadeGroupIndex] (double valueDelta)
        {
            const auto scrollAmount { static_cast<int> (valueDelta) };
            auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };
            const auto newCvInputComboBoxIndex { xfadeGroup.xfadeCvComboBox.getSelectedItemIndex () + scrollAmount };
            xfadeGroup.xfadeCvComboBox.setSelectedItemIndex (std::clamp (newCvInputComboBoxIndex, 0, xfadeGroup.xfadeCvComboBox.getNumItems () - 1));
            xfadeCvUiChanged (xfadeGroupIndex, xfadeGroups [xfadeGroupIndex].xfadeCvComboBox.getSelectedItemText ());
        };
        xfadeGroup.xfadeCvComboBox.onPopupMenuCallback = [this, xfadeGroupIndex] ()
        {
            juce::PopupMenu editMenu;
            juce::PopupMenu cloneMenu;
            for (auto curGroupIndex { 0 }; curGroupIndex < 4; ++curGroupIndex)
            {
                if (xfadeGroupIndex != curGroupIndex)
                    cloneMenu.addItem ("To Group " + juce::String::charToString ('A' + curGroupIndex), true, false, [this, curGroupIndex, xfadeGroupIndex] ()
                    {
                        editManager->setXfadeCvValueByIndex (curGroupIndex, editManager->getXfadeCvValueByIndex (xfadeGroupIndex), true);
                    });
            }
            cloneMenu.addItem ("To All", true, false, [this, xfadeGroupIndex] ()
            {
                const auto value { editManager->getXfadeCvValueByIndex (xfadeGroupIndex) };
                presetProperties.setXfadeACV (value, true);
                presetProperties.setXfadeBCV (value, true);
                presetProperties.setXfadeCCV (value, true);
                presetProperties.setXfadeDCV (value, true);
            });
            editMenu.addSubMenu ("Clone", cloneMenu, true);
            editMenu.addItem ("Default", true, false, [this, xfadeGroupIndex] ()
            {
                auto getDefaultXfadeCvValueByIndex = [this] (int xfadeGroupIndex)
                {
                    if (xfadeGroupIndex == 0)
                        return defaultPresetProperties.getXfadeACV ();
                    else if (xfadeGroupIndex == 1)
                        return defaultPresetProperties.getXfadeBCV ();
                    else if (xfadeGroupIndex == 2)
                        return defaultPresetProperties.getXfadeCCV ();
                    else if (xfadeGroupIndex == 3)
                        return defaultPresetProperties.getXfadeDCV ();
                    jassertfalse;
                        return juce::String ("Off");
                };
                editManager->setXfadeCvValueByIndex (xfadeGroupIndex, getDefaultXfadeCvValueByIndex (xfadeGroupIndex), true);
            });
            editMenu.addItem ("Revert", true, false, [this, xfadeGroupIndex] ()
            {
                auto getUneditedXfadeCvValueByIndex = [this] (int xfadeGroupIndex)
                {
                    if (xfadeGroupIndex == 0)
                        return unEditedPresetProperties.getXfadeACV ();
                    else if (xfadeGroupIndex == 1)
                        return unEditedPresetProperties.getXfadeBCV ();
                    else if (xfadeGroupIndex == 2)
                        return unEditedPresetProperties.getXfadeCCV ();
                    else if (xfadeGroupIndex == 3)
                        return unEditedPresetProperties.getXfadeDCV ();
                    jassertfalse;
                    return juce::String ("Off");
                };
                editManager->setXfadeCvValueByIndex (xfadeGroupIndex, getUneditedXfadeCvValueByIndex (xfadeGroupIndex), true);
            });
            editMenu.showMenuAsync ({}, [this] (int) {});
        };
        xfadeGroup.xfadeCvComboBox.setTooltip (parameterToolTipData.getToolTip ("Preset", "Xfade" + juce::String::charToString ('A' + xfadeGroupIndex) + "CV"));
        addAndMakeVisible (xfadeGroup.xfadeCvComboBox);

        // Xfade Group Width Label
        setupLabel (xfadeGroup.xfadeWidthLabel, "WIDTH", juce::Justification::centredRight);

        // Xfade Group Width
        //      1 decimal place when above 1.0, 0.1 increment
        //      2 decimal places below 1.0, 0.01 increment
        //      10.V
        //      9.0V
        //      .99V
        xfadeGroup.xfadeWidthEditor.setJustification (juce::Justification::centred);
        xfadeGroup.xfadeWidthEditor.setIndents (0, 0);
        xfadeGroup.xfadeWidthEditor.setInputRestrictions (0, "+-.0123456789");
        xfadeGroup.xfadeWidthEditor.setTooltip (parameterToolTipData.getToolTip ("Preset", "Xfade" + juce::String::charToString ('A' + xfadeGroupIndex) + "Width"));
        xfadeGroup.xfadeWidthEditor.setPopupMenuEnabled (false);
        xfadeGroup.xfadeWidthEditor.getMinValueCallback = [this] () { return minPresetProperties.getXfadeAWidth (); };
        xfadeGroup.xfadeWidthEditor.getMaxValueCallback = [this] () { return maxPresetProperties.getXfadeAWidth (); };
        xfadeGroup.xfadeWidthEditor.toStringCallback = [this] (double value) { return formatXfadeWidthString (value); };
        xfadeGroup.xfadeWidthEditor.updateDataCallback = [this, xfadeGroupIndex] (double value) { xfadeWidthUiChanged (xfadeGroupIndex, value); };
        xfadeGroup.xfadeWidthEditor.getIncrementCallback = [] () { return 0.01; };
        xfadeGroup.xfadeWidthEditor.onDragCallback = [this, xfadeGroupIndex] (double valueDelta)
        {
            // setValue constrains to the min/max callbacks, updates the text and writes the value back through
            // updateDataCallback, the same path a typed value takes
            const auto newValue { editManager->getXfadeGroupValueByIndex (xfadeGroupIndex) + valueDelta };
            xfadeGroups [xfadeGroupIndex].xfadeWidthEditor.setValue (newValue);
        };
        xfadeGroup.xfadeWidthEditor.onPopupMenuCallback = [this, xfadeGroupIndex] ()
        {
            juce::PopupMenu pm;
            juce::PopupMenu cloneMenu;
            for (auto curGroupIndex { 0 }; curGroupIndex < 4; ++curGroupIndex)
            {
                if (xfadeGroupIndex != curGroupIndex)
                    cloneMenu.addItem ("To Group " + juce::String::charToString ('A' + curGroupIndex), true, false, [this, curGroupIndex, xfadeGroupIndex] ()
                    {
                        editManager->setXfadeGroupValueByIndex (curGroupIndex, editManager->getXfadeGroupValueByIndex (xfadeGroupIndex), true);
                    });
            }
            cloneMenu.addItem ("To All", true, false, [this, xfadeGroupIndex] ()
            {
                const auto value { editManager->getXfadeGroupValueByIndex (xfadeGroupIndex) };
                presetProperties.setXfadeAWidth (value, true);
                presetProperties.setXfadeBWidth (value, true);
                presetProperties.setXfadeCWidth (value, true);
                presetProperties.setXfadeDWidth (value, true);
            });
            pm.addSubMenu ("Clone", cloneMenu, true);
            pm.addItem ("Default", true, false, [this, xfadeGroupIndex] ()
            {
                // the default values for all of the XFade Group Widths are the same, so we can just use A
                editManager->setXfadeGroupValueByIndex (xfadeGroupIndex, defaultPresetProperties.getXfadeAWidth (), true);
            });
            pm.addItem ("Revert", true, false, [this, xfadeGroupIndex]
            {
                auto getUneditedXfadeGroupValueByIndex = [this] (int xfadeGroupIndex)
                {
                    if (xfadeGroupIndex == 0)
                        return unEditedPresetProperties.getXfadeAWidth ();
                    else if (xfadeGroupIndex == 1)
                        return unEditedPresetProperties.getXfadeBWidth ();
                    else if (xfadeGroupIndex == 2)
                        return unEditedPresetProperties.getXfadeCWidth ();
                    else if (xfadeGroupIndex == 3)
                        return unEditedPresetProperties.getXfadeDWidth ();
                    jassertfalse;
                    return 0.0;
                };
                editManager->setXfadeGroupValueByIndex (xfadeGroupIndex, getUneditedXfadeGroupValueByIndex (xfadeGroupIndex), true);
            });
            pm.showMenuAsync ({}, [this] (int) {});
        };
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

juce::PopupMenu Assimil8orEditorComponent::createChannelCloneMenu (int channelIndex, std::function <void (ChannelProperties&)> setter)
{
    jassert (setter != nullptr);
    juce::PopupMenu cloneMenu;
    for (auto destChannelIndex { 0 }; destChannelIndex < 8; ++destChannelIndex)
    {
        if (destChannelIndex != channelIndex)
        {
            cloneMenu.addItem ("To Channel " + juce::String (destChannelIndex + 1), true, false, [this, destChannelIndex, setter] ()
            {
                editManager->forChannels ({ destChannelIndex }, [this, setter] (juce::ValueTree channelPropertiesVT)
                {
                    ChannelProperties destChannelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    setter (destChannelProperties);
                });
            });
        }
    }
    cloneMenu.addItem ("To All", true, false, [this, setter, channelIndex] ()
    {
        std::vector<int> channelIndexList;
        // build list of other channels
        for (auto destChannelIndex { 0 }; destChannelIndex < 8; ++destChannelIndex)
            if (destChannelIndex != channelIndex)
                channelIndexList.emplace_back (destChannelIndex);
        // clone to other channels
        editManager->forChannels (channelIndexList, [this, setter] (juce::ValueTree channelPropertiesVT)
        {
            ChannelProperties destChannelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            setter (destChannelProperties);
        });
    });
    return cloneMenu;
}

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

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFileChange = [this] (juce::String fileName)
    {
        //DebugLog ("Assimil8orEditorComponent", "Assimil8orEditorComponent::init/appProperties.onMostRecentFileChange: " + fileName);
        //dumpStacktrace (-1, [this] (juce::String logLine) { DebugLog ("Assimil8orEditorComponent", logLine); });
        audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
        channelTabs.setCurrentTabIndex (0);
        // a new preset opens on the full channel view
        setWaveformExpanded (false);
    };

    guiControlProperties.wrap (runtimeRootProperties.getValueTree (), GuiControlProperties::WrapperType::client, GuiControlProperties::EnableCallbacks::no);

    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::client, PresetManagerProperties::EnableCallbacks::no);
    unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    editManager = systemServices.getEditManager ();
    setupPresetPropertiesCallbacks ();
    copyBufferZoneProperties.wrap ({}, ZoneProperties::WrapperType::owner, ZoneProperties::EnableCallbacks::no);
    presetProperties.forEachChannel ([this, rootPropertiesVT] (juce::ValueTree channelPropertiesVT, int channelIndex)
    {
        channelEditors [channelIndex].init (channelPropertiesVT, unEditedPresetProperties.getChannelVT (channelIndex), rootPropertiesVT, copyBufferZoneProperties.getValueTree (), &copyBufferHasData);
        channelProperties [channelIndex].wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
        channelProperties [channelIndex].onChannelModeChange = [this] (int)
        {
            updateAllChannelTabNames ();
        };
        channelEditors [channelIndex].displayToolsMenu = [this] (int channelIndex)
        {
            juce::PopupMenu toolsMenu;
            toolsMenu.addSectionHeader ("Channel " + juce::String (channelProperties [channelIndex].getId ()));
            toolsMenu.addSeparator ();
            {
                // Clone
                juce::PopupMenu cloneMenu;
                cloneMenu.addSubMenu ("Channel Settings", createChannelCloneMenu (channelIndex, [this, channelIndex] (ChannelProperties& destChannelProperties)
                {
                    destChannelProperties.copyFrom (channelProperties [channelIndex].getValueTree ());
                }));
                cloneMenu.addSubMenu ("Zones", createChannelCloneMenu (channelIndex, [this, channelIndex] (ChannelProperties& destChannelProperties)
                {
                    channelProperties [channelIndex].forEachZone ([this, &destChannelProperties] (juce::ValueTree zonePropertiesVT, int zoneIndex)
                    {
                        ZoneProperties destZoneProperties (destChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                        destZoneProperties.copyFrom (zonePropertiesVT, false);
                        return true;
                    });
                }));
                cloneMenu.addSubMenu ("Settings and Zones", createChannelCloneMenu (channelIndex, [this, channelIndex] (ChannelProperties& destChannelProperties)
                {
                    destChannelProperties.copyFrom (channelProperties [channelIndex].getValueTree ());

                    channelProperties [channelIndex].forEachZone ([this, &destChannelProperties] (juce::ValueTree zonePropertiesVT, int zoneIndex)
                    {
                        ZoneProperties destZoneProperties (destChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                        destZoneProperties.copyFrom (zonePropertiesVT, false);
                        return true;
                    });
                }));
                cloneMenu.addSubMenu ("MinVoltages", createChannelCloneMenu (channelIndex, [this, channelIndex] (ChannelProperties& destChannelProperties)
                {
                    if (const auto destNumUsedZones { editManager->getNumUsedZones (destChannelProperties.getId () - 1) };
                        destNumUsedZones > 1)
                    {
                        channelProperties[channelIndex].forEachZone ([this, &destChannelProperties, destNumUsedZones] (juce::ValueTree zonePropertiesVT, int curZoneIndex)
                        {
                            if (curZoneIndex == destNumUsedZones - 1)
                                return false;
                            ZoneProperties srcZone (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                            ZoneProperties destZone (destChannelProperties.getZoneVT (curZoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                            destZone.setMinVoltage (srcZone.getMinVoltage (), false);
                            return true;
                        });
                    }
                }));
                toolsMenu.addSubMenu ("Clone", cloneMenu);
            }
            {
                juce::PopupMenu editMenu;
                editMenu.addItem ("Copy", true, false, [this, channelIndex] ()
                {
                    copyBufferChannelProperties.copyFrom (channelProperties [channelIndex].getValueTree ());
                    copyBufferHasData = true;
                });
                editMenu.addItem ("Paste", copyBufferHasData, false, [this, channelIndex] ()
                {
                    channelProperties [channelIndex].copyFrom (copyBufferChannelProperties.getValueTree ());
                });
                toolsMenu.addSubMenu ("Edit", editMenu, true);
            }
            {
                juce::PopupMenu explodeMenu;
                for (auto explodeCount { 2 }; explodeCount < 9 - channelIndex; ++explodeCount)
                    explodeMenu.addItem (juce::String (explodeCount) + " channels", true, false, [this, channelIndex, explodeCount] ()
                    {
                        explodeChannel (channelIndex, explodeCount);
                    });
                toolsMenu.addSubMenu ("Explode", explodeMenu, channelIndex < 7);
            }
            toolsMenu.addItem ("Default", true, false, [this, channelIndex] ()
            {
                channelProperties [channelIndex].copyFrom (defaultChannelProperties.getValueTree ());
            });
            toolsMenu.addItem ("Revert", true, false, [this, channelIndex] ()
            {
                channelProperties [channelIndex].copyFrom (unEditedPresetProperties.getValueTree ());
            });
            toolsMenu.showMenuAsync ({});
        };

        channelProperties [channelIndex].wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
        channelProperties [channelIndex].onChannelModeChange = [this] (int)
        {
            updateAllChannelTabNames ();
        };
        return true;
    });

    idDataChanged (presetProperties.getId ());
    midiSetupDataChanged (presetProperties.getMidiSetup ());
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
    presetProperties.onMidiSetupChange = [this] (int midiSetupId) { midiSetupDataChanged (midiSetupId); };
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

void Assimil8orEditorComponent::receiveSampleLoadRequest (juce::File sampleFile)
{
    auto channelIndex { channelTabs.getCurrentTabIndex () };
    auto curChannelEditor { dynamic_cast<ChannelEditor*> (channelTabs.getTabContentComponent (channelIndex)) };
    curChannelEditor->receiveSampleLoadRequest (sampleFile);
}

void Assimil8orEditorComponent::overwritePresetOrCancel (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
{
    jassert (overwriteFunction != nullptr);
    jassert (cancelFunction != nullptr);

    if (PresetHelpers::areEntirePresetsEqual (unEditedPresetProperties.getValueTree (), presetProperties.getValueTree ()))
    {
        overwriteFunction ();
    }
    else
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "Overwriting Edited Preset",
            "You have not saved a preset that you have edited.\n  Select Continue to lose your changes.\n  Select Cancel to go back and save.", "Continue (lose changes)", "Cancel", nullptr,
            juce::ModalCallbackFunction::create ([this, overwriteFunction, cancelFunction] (int option)
            {
                juce::MessageManager::callAsync ([this, option, overwriteFunction,cancelFunction] ()
                {
                    if (option == 1) // Continue
                        overwriteFunction ();
                    else // cancel
                        cancelFunction ();
                });
            }));
    }
}

void Assimil8orEditorComponent::savePreset ()
{
    auto presetFile { juce::File (appProperties.getMRUList () [0]) };
    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.write (presetFile, presetProperties.getValueTree ());
    PresetProperties::copyTreeProperties (presetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
}

void Assimil8orEditorComponent::applyExplicitColours ()
{
    titleLabel.setColour (juce::Label::ColourIds::textColourId, findColour (A8Colours::accentText));
    midiSetupLabel.setColour (juce::Label::ColourIds::textColourId, findColour (A8Colours::accentText));
    // the preset wide controls along the bottom are named as the channel and zone parameters are
    data2AsCvLabel.setColour (juce::Label::ColourIds::textColourId, findColour (A8Colours::textDim));
    // XFADE heads the four groups after it, so it is set as a section header is
    xfadeGroupsLabel.setColour (juce::Label::ColourIds::textColourId, findColour (A8Colours::accentText));
    for (auto& xfadeGroup : xfadeGroups)
        for (auto* label : { &xfadeGroup.xfadeGroupLabel, &xfadeGroup.xfadeCvLabel, &xfadeGroup.xfadeWidthLabel })
            label->setColour (juce::Label::ColourIds::textColourId, findColour (A8Colours::textDim));
    // Save is the one action in this header worth making obvious - but only when
    // there is something to save, or it would look ready to press with nothing to write.
    saveButton.setPrimary (presetHasUnsavedEdits);
}

void Assimil8orEditorComponent::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    applyExplicitColours ();
    // a plain TextEditor stores a colour with the text it already holds, so the
    // existing contents have to be re-tinted (oolib's CustomTextEditor does this
    // for itself, which is why the value fields do not need it here)
    nameEditor.applyColourToAllText (findColour (juce::TextEditor::textColourId), true);
}

void Assimil8orEditorComponent::paint (juce::Graphics& g)
{
    // opaque, so the background behind the rounded corners is this component's to paint
    g.fillAll (findColour (A8Colours::windowBackground));
    A8Paint::card (g, *this, getLocalBounds (), A8Colours::windowBackground);

    g.setColour (findColour (A8Colours::listBackground));
    g.fillRect (headerBounds);
    g.setColour (findColour (A8Colours::outline));
    g.fillRect (headerBounds.withTop (headerBounds.getBottom () - 1));

    if (presetHasUnsavedEdits)
    {
        auto tagBounds { unsavedEditsBounds };
        const auto ledBounds { tagBounds.removeFromLeft (static_cast<int> (StatusLed::kDiameter)).toFloat ()
                                        .withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
        StatusLed::draw (g, ledBounds, true, *this, A8Colours::unsavedEdits);
        tagBounds.removeFromLeft (6);
        g.setFont (A8Type::statusTag ());
        g.setColour (findColour (A8Colours::unsavedEdits));
        g.drawText (kUnsavedEditsText, tagBounds, juce::Justification::centredLeft, false);
    }
}

void Assimil8orEditorComponent::explodeChannel (int channelIndex, int explodeCount)
{
    // clone channelIndex into explodeCount-1 subsequent channels
    for (auto destinationChannelIndex { channelIndex + 1 }; destinationChannelIndex < channelIndex + explodeCount; ++destinationChannelIndex)
    {
        auto& destChannelProperties { channelProperties [destinationChannelIndex] };
        destChannelProperties.copyFrom (channelProperties [channelIndex].getValueTree ());

        channelProperties [channelIndex].forEachZone ([this, &destChannelProperties] (juce::ValueTree zonePropertiesVT, int zoneIndex)
        {
            ZoneProperties destZoneProperties (destChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
            destZoneProperties.copyFrom (zonePropertiesVT, false);
            return true;
        });
    }
    // set sample/loop points for channelIndex and explodeCount-1 subsequent channels to sequential slice size pieces
    SampleManagerProperties sampleManagerProperties (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);
    channelProperties [channelIndex].forEachZone ([this, channelIndex, explodeCount, &sampleManagerProperties] (juce::ValueTree zonePropertiesVT, int zoneIndex)
    {
        ZoneProperties zoneProperties { zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no };
        SampleProperties sampleProperties (sampleManagerProperties.getSamplePropertiesVT (channelIndex, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
        juce::int64 sampleSize { sampleProperties.getLengthInSamples () };
        const auto sliceSize { sampleSize / explodeCount };
        auto setSamplePoints = [this, sliceSize] (ZoneProperties& zpToUpdate, int index)
        {
            const auto sampleStart { index * sliceSize };
            const auto sampleEnd { sampleStart + sliceSize };
            zpToUpdate.setSampleStart (sampleStart, true);
            zpToUpdate.setSampleEnd (sampleEnd, true);
            zpToUpdate.setLoopStart (sampleStart, true);
            zpToUpdate.setLoopLength (static_cast<double> (sliceSize), true);
        };
        setSamplePoints (zoneProperties, 0);
        for (auto channelCount { 0 }; channelCount < explodeCount - 1; ++channelCount)
        {
            auto& destChannelProperties { channelProperties [channelIndex + channelCount + 1] };
            ZoneProperties destZoneProperties { destChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no };
            setSamplePoints (destZoneProperties, channelCount + 1);
        }
        return true;
    });
}

void Assimil8orEditorComponent::updateAllChannelTabNames ()
{
    for (auto channelIndex { 0 }; channelIndex < channelTabs.getNumTabs (); ++channelIndex)
        updateChannelTabName (channelIndex);
}

bool Assimil8orEditorComponent::isChannelActive (int channelIndex)
{
    ZoneProperties zoneProperties (channelProperties [channelIndex].getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    return ! zoneProperties.getSample ().isEmpty ();
}

void Assimil8orEditorComponent::updateChannelTabName (int channelIndex)
{
    auto channelTabTitle { juce::String ("CH ") + juce::String::charToString ('1' + channelIndex) };

    if (channelIndex != 7 && channelProperties [channelIndex].getChannelMode () != ChannelProperties::ChannelMode::stereoRight && isChannelActive (channelIndex) &&
        channelProperties [channelIndex + 1].getChannelMode () == ChannelProperties::ChannelMode::stereoRight)
    {
        channelTabTitle += "-L";
    }
    else if (channelProperties [channelIndex].getChannelMode () == ChannelProperties::ChannelMode::stereoRight)
    {
        channelTabTitle = "R-" + channelTabTitle;
    }

    channelTabs.setTabName (channelIndex, channelTabTitle);
    // the left channel of a stereo pair has no divider between it and its right channel
    channelTabs.setTabJoinedToNext (channelIndex, channelTabTitle.endsWith ("-L"));
}

void Assimil8orEditorComponent::setPresetToDefaults ()
{
    PresetProperties::copyTreeProperties (defaultPresetProperties.getValueTree (), presetProperties.getValueTree ());
    presetProperties.setId (unEditedPresetProperties.getId (), true);
}

void Assimil8orEditorComponent::revertPreset ()
{
    PresetProperties::copyTreeProperties (unEditedPresetProperties.getValueTree (), presetProperties.getValueTree ());
}

void Assimil8orEditorComponent::displayToolsMenu ()
{
    juce::PopupMenu toolsMenu;
    toolsMenu.addSectionHeader ("Preset");
    toolsMenu.addSeparator ();

    {
        juce::PopupMenu importMenu;
        importMenu.addItem ("Settings and Samples", true, false, [this] () { importPresetSettingsAndSamples (); });
        importMenu.addItem ("Settings Only", true, false, [this] () { importPresetSettings (); });
        toolsMenu.addSubMenu ("Import", importMenu, true);
    }
    {
        juce::PopupMenu exportMenu;
        exportMenu.addItem ("Settings and Samples", true, false, [this] () { exportPresetSettingsAndSamples (); });
        exportMenu.addItem ("Settings Only", true, false, [this] () { exportPresetSettings (); });
        toolsMenu.addSubMenu ("Export", exportMenu, true);
    }
    toolsMenu.addItem ("Default", true, false, [this] () { setPresetToDefaults (); });
    toolsMenu.addItem ("Revert", true, false, [this] () { revertPreset (); });
    toolsMenu.addItem ("Midi Setups", true, false, [this] () { guiControlProperties.showMidiConfigWindow (true); });

    toolsMenu.showMenuAsync (juce::PopupMenu::Options ().withTargetComponent (&toolsButton));
}

void Assimil8orEditorComponent::exportPresetSettings ()
{
    fileChooser.reset (new juce::FileChooser ("Please select the file to export to...", appProperties.getImportExportMruFolder (), ""));
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
        {
            auto exportPresetFile { fc.getURLResults () [0].getLocalFile () };
            appProperties.setImportExportMruFolder (exportPresetFile.getParentDirectory ().getFullPathName ());
            Assimil8orPreset assimil8orPreset;
            assimil8orPreset.write (exportPresetFile, presetProperties.getValueTree ());
        }
    }, nullptr);
}

void Assimil8orEditorComponent::exportPresetSettingsAndSamples ()
{
    // query user for folder/file name to export to. This will be a zip file containing the preset settings and samples
    fileChooser.reset (new juce::FileChooser ("Please select the zip file to export to...", appProperties.getImportExportMruFolder (), "*.zip"));
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
        {
            auto exportContainerFile { fc.getURLResults () [0].getLocalFile () };
            if (exportContainerFile.getFileExtension ().isEmpty ())
                exportContainerFile = exportContainerFile.withFileExtension (".zip");
            appProperties.setImportExportMruFolder (exportContainerFile.getParentDirectory ().getFullPathName ());

            auto sampleFolder { juce::File (appProperties.getMostRecentFolder ()) };
            // we use a std::set because a sample may be used by more than one channel/zone, but we only need to export/copy it once
            std::set<juce::File> sampleFiles;
            SampleManagerProperties sampleManagerProperties (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);
            for (auto channelIndex { 0 }; channelIndex < kNumChannels; ++channelIndex)
                for (auto zoneIndex { 0 }; zoneIndex < kNumZones; ++zoneIndex)
                {
                    SampleProperties sampleProperties (sampleManagerProperties.getSamplePropertiesVT (channelIndex,zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::no);
                     if (sampleProperties.getStatus () == SampleStatus::exists)
                         sampleFiles.emplace (sampleFolder.getChildFile (sampleProperties.getName ()));
                }
            // create temp folder to hold temp preset file
            auto tempFolder { exportContainerFile.getParentDirectory ().getChildFile ("temp_" + juce::String::toHexString (juce::Random::getSystemRandom ().nextInt ())) };
            tempFolder.createDirectory ();

            // write out temp preset file
            Assimil8orPreset assimil8orPreset;
            auto presetFile { tempFolder.getChildFile (exportContainerFile.getFileNameWithoutExtension () + (".yml")) };
            assimil8orPreset.write (presetFile, presetProperties.getValueTree ());

            juce::ZipFile::Builder exportContainerBuilder;
            // add preset file to zip file
            exportContainerBuilder.addFile (presetFile, 9);
            // add samples to zip file
            for (auto& sampleFile : sampleFiles)
                exportContainerBuilder.addFile (sampleFile, 9);

            // write out zip file
            {
                auto outputStream { exportContainerFile.createOutputStream () };
                exportContainerBuilder.writeToStream (*outputStream, nullptr);
            }
            // clean up temp folder
            tempFolder.deleteRecursively ();
        }
    }, nullptr);
}

void Assimil8orEditorComponent::importPresetSettings ()
{
    overwritePresetOrCancel ([this] ()
    {
        fileChooser.reset (new juce::FileChooser ("Please select the file to import from...", appProperties.getImportExportMruFolder (), ""));
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
            {
                auto importPresetFile { fc.getURLResults () [0].getLocalFile () };

                appProperties.setImportExportMruFolder (importPresetFile.getParentDirectory ().getFullPathName ());
                juce::StringArray fileContents;
                importPresetFile.readLines (fileContents);

                // TODO - check for import errors and handle accordingly
                Assimil8orPreset assimil8orPreset;
                assimil8orPreset.parse (fileContents);

                // change the imported Preset Id to the current Preset Id
                PresetProperties importedPresetProperties (assimil8orPreset.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
                importedPresetProperties.setId (presetProperties.getId (), false);

                // copy imported preset to current preset
                PresetProperties::copyTreeProperties (importedPresetProperties.getValueTree (), presetProperties.getValueTree ());
            }
        }, nullptr);
    },
    [] () {});
}

void Assimil8orEditorComponent::importPresetSettingsAndSamples ()
{
    // query user for folder/file name to import from. This will be a zip file containing the preset settings and samples
    fileChooser.reset (new juce::FileChooser ("Please select the zip file to import from...", appProperties.getImportExportMruFolder (), "*.zip"));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
        {
            auto importContainerFile { fc.getURLResults () [0].getLocalFile () };
            appProperties.setImportExportMruFolder (importContainerFile.getParentDirectory ().getFullPathName ());

            juce::ZipFile importContainerReader (importContainerFile);

            auto sampleFolder { juce::File (appProperties.getMostRecentFolder ()) };

            for (auto curEntryIndex { 0 }; curEntryIndex < importContainerReader.getNumEntries (); ++curEntryIndex)
            {
                auto entry { importContainerReader.getEntry (curEntryIndex) };
                if (entry->filename.endsWithIgnoreCase (".yml"))
                {
                     // import settings from this file into the current preset
                    auto tempFolder { sampleFolder.getChildFile ("temp_" + juce::String::toHexString (juce::Random::getSystemRandom ().nextInt ())) };
                    tempFolder.createDirectory ();

                    // extract the preset file into the temp folder and read it
                    importContainerReader.uncompressEntry (curEntryIndex, tempFolder, true);
                    auto tempPresetFile { tempFolder.getChildFile (entry->filename) };
                    juce::StringArray fileContents;
                    tempPresetFile.readLines (fileContents);
                    // TODO - check for import errors and handle accordingly
                    Assimil8orPreset assimil8orPreset;
                    assimil8orPreset.parse (fileContents);

                    // change the imported Preset Id to the current Preset Id
                    PresetProperties importedPresetProperties (assimil8orPreset.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
                    importedPresetProperties.setId (presetProperties.getId (), false);
                    // copy imported preset to current preset
                    PresetProperties::copyTreeProperties (importedPresetProperties.getValueTree (), presetProperties.getValueTree ());

                    tempFolder.deleteRecursively ();
                }
                else
                {
                    // extract this file into the current preset folder, overwrite if necessary
                    importContainerReader.uncompressEntry (curEntryIndex, sampleFolder, true);
                }
            }
        }
    }, nullptr);
}

void Assimil8orEditorComponent::resized ()
{
    // inside the pane's outline
    auto localBounds { getLocalBounds ().reduced (1) };

    headerBounds = localBounds.removeFromTop (kHeaderHeight);
    static constexpr auto kGap { 9 };
    auto headerRow { headerBounds.withTrimmedBottom (1).reduced (kGap, 0) };
    auto placeLeft = [&headerRow] (juce::Component& component, int width, int height)
    {
        component.setBounds (headerRow.removeFromLeft (width).withSizeKeepingCentre (width, height));
        headerRow.removeFromLeft (kGap);
    };
    // sized for the widest preset number, so the name field does not move as presets are loaded
    placeLeft (titleLabel, A8Paint::textWidth (A8Type::sectionHeader (), "PRESET 199") + 2, headerRow.getHeight ());
    placeLeft (nameEditor, 150, kFieldHeight);
    placeLeft (midiSetupLabel, A8Paint::textWidth (A8Type::sectionHeader (), midiSetupLabel.getText ()) + 2, headerRow.getHeight ());
    placeLeft (midiSetupComboBox, 50, kFieldHeight);
    unsavedEditsBounds = headerRow.removeFromLeft (static_cast<int> (StatusLed::kDiameter) + 6 + A8Paint::textWidth (A8Type::statusTag (), kUnsavedEditsText) + 2);

    const auto saveWidth { saveButton.getIdealWidth () };
    saveButton.setBounds (headerRow.removeFromRight (saveWidth).withSizeKeepingCentre (saveWidth, ActionButton::kNormalHeight));
    headerRow.removeFromRight (kGap);
    const auto toolsWidth { toolsButton.getIdealWidth () };
    toolsButton.setBounds (headerRow.removeFromRight (toolsWidth).withSizeKeepingCentre (toolsWidth, ActionButton::kNormalHeight));

    // the preset wide controls along the bottom
    auto bottomRow { localBounds.removeFromBottom (kBottomRowHeight) };
    windowDecorator.setBounds (bottomRow);
    bottomRow = bottomRow.withTrimmedTop (1).reduced (6, 0);
    static constexpr auto kBottomFieldHeight { 20 };
    auto placeBottom = [&bottomRow] (juce::Component& component, int width, int gapAfter)
    {
        component.setBounds (bottomRow.removeFromLeft (width).withSizeKeepingCentre (width, kBottomFieldHeight));
        bottomRow.removeFromLeft (gapAfter);
    };
    auto labelWidth = [] (const juce::Label& label) { return A8Paint::textWidth (label.getFont (), label.getText ()) + 2; };

    // packed to fit the narrowest editor: four groups of about 155 pixels each after the Data2 control
    placeBottom (data2AsCvLabel, labelWidth (data2AsCvLabel), 3);
    placeBottom (data2AsCvComboBox, 28, 10);
    placeBottom (xfadeGroupsLabel, labelWidth (xfadeGroupsLabel), 6);
    for (auto xfadeGroupIndex { 0 }; xfadeGroupIndex < XfadeGroupIndex::numberOfGroups; ++xfadeGroupIndex)
    {
        auto& xfadeGroup { xfadeGroups [xfadeGroupIndex] };
        placeBottom (xfadeGroup.xfadeGroupLabel, labelWidth (xfadeGroup.xfadeGroupLabel), 3);
        placeBottom (xfadeGroup.xfadeCvLabel, labelWidth (xfadeGroup.xfadeCvLabel), 2);
        placeBottom (xfadeGroup.xfadeCvComboBox, 28, 5);
        placeBottom (xfadeGroup.xfadeWidthLabel, labelWidth (xfadeGroup.xfadeWidthLabel), 2);
        placeBottom (xfadeGroup.xfadeWidthEditor, 40, 10);
    }

    // The channel editor lays itself out to whatever size it is given, but below
    // this it would have to crowd its controls, so it is clipped instead.
    constexpr auto kMinimumEditorWidth { 765 };
    channelTabs.setBounds (localBounds.withWidth (std::max (localBounds.getWidth (), kMinimumEditorWidth)));
}

void Assimil8orEditorComponent::idDataChanged (int id)
{
    titleLabel.setText ("PRESET " + juce::String (id), juce::NotificationType::dontSendNotification);
    resized ();
}

void Assimil8orEditorComponent::midiSetupDataChanged (int midiSetupId)
{
    midiSetupComboBox.setSelectedItemIndex (midiSetupId);
}

void Assimil8orEditorComponent::midiSetupUiChanged (int midiSetupId)
{
    presetProperties.setMidiSetup (midiSetupId, false);
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
    xfadeGroups [group].xfadeWidthEditor.setText (formatXfadeWidthString (width));
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
    // check if data has changed
    const auto hasUnsavedEdits { ! PresetHelpers::areEntirePresetsEqual (unEditedPresetProperties.getValueTree (), presetProperties.getValueTree ()) };
    saveButton.setEnabled (hasUnsavedEdits);
    if (hasUnsavedEdits != presetHasUnsavedEdits)
    {
        presetHasUnsavedEdits = hasUnsavedEdits;
        // the app cannot say which value changed, only that something did
        repaint (unsavedEditsBounds);
        applyExplicitColours ();
    }

    // a channel tab lights when that channel holds a sample
    for (auto channelIndex { 0 }; channelIndex < channelTabs.getNumTabs (); ++channelIndex)
        channelTabs.setTabHasContent (channelIndex, isChannelActive (channelIndex));
}
