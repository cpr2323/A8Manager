#include "ChannelEditor.h"
#include "FormatHelpers.h"
#include "MinVoltageTests.h"
#include "ParameterToolTipData.h"
#include "../../../Assimil8or/Preset/PresetHelpers.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

#define LOG_MIN_VOLTAGE_DISRIBUTION 0
#if LOG_MIN_VOLTAGE_DISRIBUTION
#define LogMinVoltageDistribution(text) DebugLog ("MinVoltageDistribution", text);
#else
#define LogMinVoltageDistribution(text) ;
#endif

#define LOG_DATA_AND_UI_CHANGES 0
#if LOG_DATA_AND_UI_CHANGES
#define LogDataAndUiChanges(text) DebugLog ("ChannelEditor", text);
#else
#define LogDataAndUiChanges(text) ;
#endif

const auto kLargeLabelSize { 20.0f };
const auto kMediumLabelSize { 14.0f };
const auto kSmallLabelSize { 12.0f };
const auto kLargeLabelIntSize { static_cast<int> (kLargeLabelSize) };
const auto kMediumLabelIntSize { static_cast<int> (kMediumLabelSize) };
const auto kSmallLabelIntSize { static_cast<int> (kSmallLabelSize) };

const auto kParameterLineHeight { 20 };
const auto kFirstControlSectionYOffset { 1 };
const auto kInterControlYOffset { 2 };
const auto kInitialYOffset { 5 };
const auto kNewSectionOffset { 5 };

const auto kMaxEnvelopeTime { 99.0 };

ChannelEditor::ChannelEditor ()
{
    // TODO - these lambdas are copies of what is in ChannelEditor::setupChannelComponents, need to DRY
    auto setupLabel = [this] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        const auto textColor { juce::Colours::black };
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setColour (juce::Label::ColourIds::textColourId, textColor);
        label.setFont (label.getFont ().withPointHeight (fontSize));
        label.setMinimumHorizontalScale (1.0f);
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupComboBox = [this] (juce::ComboBox& comboBox, std::function<void ()> onChangeCallback)
    {
        jassert (onChangeCallback != nullptr);
        comboBox.onChange = onChangeCallback;
        addAndMakeVisible (comboBox);
    };

    setupLabel (zonesLabel, "ZONES", kMediumLabelSize, juce::Justification::centredLeft);
    zonesLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
    setupLabel (zoneMaxVoltage, "+5.00", 10.0, juce::Justification::centredLeft);
    zoneMaxVoltage.setColour (juce::Label::ColourIds::textColourId, ZonesTabbedLookAndFeel::kPositiveVoltageColor);

    setupLabel (loopLengthIsEndLabel, "LENGTH/END", kSmallLabelIntSize, juce::Justification::centredRight);
    loopLengthIsEndComboBox.addItem ("Length", 1); // 0 = Length, 1 = End
    loopLengthIsEndComboBox.addItem ("End", 2);
    setupComboBox (loopLengthIsEndComboBox, [this] ()
    {
        const auto loopLengthIsEnd { loopLengthIsEndComboBox.getSelectedId () == 2 };
        loopLengthIsEndUiChanged (loopLengthIsEnd);
        // inform all the zones of the change
        for (auto& zoneEditor : zoneEditors)
            zoneEditor.setLoopLengthIsEnd (loopLengthIsEnd);
    });
    loopLengthIsEndComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setLoopLengthIsEnd (std::clamp (loopLengthIsEndComboBox.getSelectedItemIndex () + scrollAmount, 0, loopLengthIsEndComboBox.getNumItems () - 1), true);
    };
    loopLengthIsEndComboBox.onPopupMenuCallback = [this] ()
    {
    };

    for (auto curZoneIndex { 0 }; curZoneIndex < 8; ++curZoneIndex)
    {
        zoneTabs.addTab (juce::String::charToString ('1' + curZoneIndex), juce::Colours::darkgrey, &zoneEditors [curZoneIndex], false);
        zoneTabs.setTabBackgroundColour (curZoneIndex, zoneTabs.getTabBackgroundColour (curZoneIndex).darker (0.2f));
    }
    zoneTabs.setTabBarDepth (zoneTabs.getTabBarDepth () + 5);
    zoneTabs.setLookAndFeel (&zonesTabbedLookAndFeel);
    zoneTabs.onSelectedTabChanged = [this] (int)
    {
        configAudioPlayer ();
    };
    addAndMakeVisible (zoneTabs);

    attackFromCurrentComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    autoTriggerComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    channelModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    linAMisExtEnvComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    loopLengthIsEndComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    loopModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    mixModIsFaderComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    playModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    pMSourceComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    xfadeGroupComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    zonesRTComboBox.setLookAndFeel (&noArrowComboBoxLnF);

    {
        PresetProperties minPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MinParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        minChannelProperties.wrap (minPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    }
    {
        PresetProperties maxPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        maxChannelProperties.wrap (maxPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    }
    {
        PresetProperties defaultPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                                  PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        defaultChannelProperties.wrap (defaultPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        defaultZoneProperties.wrap (defaultChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }

    toolsButton.setButtonText ("TOOLS");
    toolsButton.onClick = [this] ()
    {
        if (displayToolsMenu != nullptr)
            displayToolsMenu (channelProperties.getId () - 1);
    };
    addAndMakeVisible (toolsButton);
    setupChannelComponents ();
    arEnvelopeProperties.wrap (arEnvelopeComponent.getPropertiesVT (), AREnvelopeProperties::WrapperType::client, AREnvelopeProperties::EnableCallbacks::yes);
    arEnvelopeProperties.onAttackPercentChanged = [this] (double attackPercent)
    {
        const auto rawAttackValue { (kMaxEnvelopeTime * 2) * attackPercent };
        const auto curAttackFractionalValue { channelProperties.getAttack () - static_cast<int> (channelProperties.getAttack ()) };
        attackDataChanged (snapEnvelopeValue (static_cast<int> (rawAttackValue) + curAttackFractionalValue));
    };
    arEnvelopeProperties.onReleasePercentChanged = [this] (double releasePercent)
    {
        const auto rawReleaseValue { (kMaxEnvelopeTime *2) * releasePercent };
        const auto curReleaseFractionalValue { channelProperties.getRelease () - static_cast<int> (channelProperties.getRelease ()) };
        releaseDataChanged (snapEnvelopeValue (static_cast<int> (rawReleaseValue) + curReleaseFractionalValue));
    };
    addAndMakeVisible (arEnvelopeComponent);
    updateAllZoneTabNames ();
    addChildComponent (stereoRightTransparantOverly);

    channelProperties.setAttack ((kMaxEnvelopeTime * 2) * arEnvelopeProperties.getAttackPercent (), false);
    channelProperties.setRelease ((kMaxEnvelopeTime * 2) * arEnvelopeProperties.getReleasePercent (), false);
}

ChannelEditor::~ChannelEditor ()
{
    zoneTabs.setLookAndFeel (nullptr);
    attackFromCurrentComboBox.setLookAndFeel (nullptr);
    autoTriggerComboBox.setLookAndFeel (nullptr);
    channelModeComboBox.setLookAndFeel (nullptr);
    linAMisExtEnvComboBox.setLookAndFeel (nullptr);
    loopLengthIsEndComboBox.setLookAndFeel (nullptr);
    loopModeComboBox.setLookAndFeel (nullptr);
    mixModIsFaderComboBox.setLookAndFeel (nullptr);
    playModeComboBox.setLookAndFeel (nullptr);
    pMSourceComboBox.setLookAndFeel (nullptr);
    xfadeGroupComboBox.setLookAndFeel (nullptr);
    zonesRTComboBox.setLookAndFeel (nullptr);
}

void ChannelEditor::visibilityChanged ()
{
    if (isVisible ())
        configAudioPlayer ();
}

void ChannelEditor::clearAllZones ()
{
    const auto numZones { getNumUsedZones () };
    for (auto curZoneIndex { 0 }; curZoneIndex < numZones; ++curZoneIndex)
        zoneProperties [curZoneIndex].copyFrom (defaultZoneProperties.getValueTree ());
}

void ChannelEditor::copyZone (int zoneIndex)
{
    copyBufferZoneProperties.copyFrom (zoneProperties [zoneIndex].getValueTree ());
    *zoneCopyBufferHasData = true;
}

void ChannelEditor::deleteZone (int zoneIndex)
{
    zoneProperties [zoneIndex].copyFrom (defaultZoneProperties.getValueTree ());
    // if this zone was the last in the list, but not also the first, then set the minVoltage for the new last in list to -5
    if (zoneIndex == getNumUsedZones () && zoneIndex != 0)
        zoneProperties [zoneIndex - 1].setMinVoltage (-5.0, false);
    removeEmptyZones ();
}

void ChannelEditor::duplicateZone (int zoneIndex)
{
    // if the list is full, we can't copy the end anywhere, so we'll start with the one before the end, otherwise start at the end
    const auto startingZoneIndex { getNumUsedZones () - (getNumUsedZones () == zoneProperties.size () ? 2 : 1) };
    for (auto curZoneIndex { startingZoneIndex }; curZoneIndex >= zoneIndex; --curZoneIndex)
    {
        ZoneProperties destZoneProperties (channelProperties.getZoneVT (curZoneIndex + 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        destZoneProperties.copyFrom (channelProperties.getZoneVT (curZoneIndex));
    }
    // if our duplicated zone is not on the end, set the voltage 1/2 between it's neighbors
    if (zoneIndex + 1 < getNumUsedZones () - 1)
    {
        auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneIndex + 1, 0) };
        zoneProperties [zoneIndex + 1].setMinVoltage (bottomBoundary + ((topBoundary - bottomBoundary) / 2), false);
    }

    // if the zone on the end does not have a -5 minVoltage, then set it to -5
    const auto indexOfLastZone { getNumUsedZones () - 1 };
    if (zoneProperties[indexOfLastZone].getMinVoltage () != -5.0)
        zoneProperties [indexOfLastZone].setMinVoltage (-5.0, false);
}

void ChannelEditor::pasteZone (int zoneIndex)
{
    zoneProperties [zoneIndex].copyFrom (copyBufferZoneProperties.getValueTree ());
    // if this is not on the end
    if (zoneIndex < getNumUsedZones () - 1)
    {
        // ensure pasted minVoltage is valid
        const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneIndex, 0) };
        const auto curMinVolatage { zoneProperties [zoneIndex].getMinVoltage () };
        if (curMinVolatage >= topBoundary || curMinVolatage <= bottomBoundary)
            zoneProperties [zoneIndex].setMinVoltage (bottomBoundary + ((topBoundary - bottomBoundary) / 2), false);
    }
    else
    {
        // this handles pasting to the end, and pasting past the end, which cannot discern at this point
        // so first we ensure that the last zone is -5
        zoneProperties [zoneIndex].setMinVoltage (-5.0, false);

        // if this isn't the first item, verify the previous item is valid
        if (zoneIndex > 0)
        {
            const auto minValue { -5.0 };
            const auto [topBoundary, _] { getVoltageBoundaries (zoneIndex, 1) };
            const auto prevMinVolatage { zoneProperties [zoneIndex - 1].getMinVoltage () };
            if (prevMinVolatage >= topBoundary || prevMinVolatage <= minValue)
                zoneProperties [zoneIndex - 1].setMinVoltage (minValue + ((topBoundary - minValue) / 2), false);
        }
    }
}

// TODO - does this function really need to look for multiple empty zones? assuming it gets called when a zone is deleted, there should only be one
void ChannelEditor::removeEmptyZones ()
{
    for (auto zoneIndex { 0 }; zoneIndex < zoneTabs.getNumTabs () - 1; ++zoneIndex)
    {
        ZoneProperties curZoneProperties (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        if (curZoneProperties.getSample ().isEmpty ())
        {
            bool moveHappened { false };
            for (auto nextZoneIndex { zoneIndex + 1 }; nextZoneIndex < zoneTabs.getNumTabs (); ++nextZoneIndex)
            {
                ZoneProperties nextZoneProperties (channelProperties.getZoneVT (nextZoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                if (nextZoneProperties.getSample ().isNotEmpty ())
                {
                    curZoneProperties.copyFrom (nextZoneProperties.getValueTree ());
                    nextZoneProperties.copyFrom (defaultZoneProperties.getValueTree ());
                    curZoneProperties.wrap (channelProperties.getZoneVT (zoneIndex + (nextZoneIndex - zoneIndex)), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);

                    moveHappened = true;
                }
            }
            // there were none others to move
            if (! moveHappened)
                break;
        }
    }
}

// TODO - this works well enough, in that I believe it ensures a valid zone is selected
//        I should add comments describing what problem is solves
void ChannelEditor::ensureProperZoneIsSelected ()
{
    auto& tabbedButtonBar { zoneTabs.getTabbedButtonBar () };
    auto lastEnabledZoneTab { -1 };
    // set enabled state based on sample loaded or not
    for (auto zoneIndex { 0 }; zoneIndex < zoneTabs.getNumTabs (); ++zoneIndex)
    {
        ZoneProperties curZoneProperties (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        if (curZoneProperties.getSample ().isEmpty ())
        {
            tabbedButtonBar.getTabButton (zoneIndex)->setEnabled (false);
        }
        else
        {
            tabbedButtonBar.getTabButton (zoneIndex)->setEnabled (true);
            lastEnabledZoneTab = zoneIndex;
        }
    }

    if (lastEnabledZoneTab == - 1)
    {
        // there are no samples loaded
        zoneTabs.setCurrentTabIndex (0);
        tabbedButtonBar.getTabButton (0)->setEnabled (true);
    }
    else if (lastEnabledZoneTab < 7)
    {
        auto curTabIndex { zoneTabs.getCurrentTabIndex () };
        jassert (curTabIndex != -1);
        // there are samples loaded, and there are still empty zones
        tabbedButtonBar.getTabButton (lastEnabledZoneTab + 1)->setEnabled (true);
        if (! tabbedButtonBar.getTabButton (curTabIndex)->isEnabled ())
        {
            if (curTabIndex > 0)
            {
                for (auto zoneIndexToCheck { curTabIndex - 1 }; zoneIndexToCheck > 0; --zoneIndexToCheck)
                {
                    if (tabbedButtonBar.getTabButton (zoneIndexToCheck)->isEnabled ())
                    {
                        zoneTabs.setCurrentTabIndex (zoneIndexToCheck);
                        break;
                    }
                }
            }
            else
            {
                zoneTabs.setCurrentTabIndex (0);
            }
        }
    }
}

int ChannelEditor::getEnvelopeValueResolution (double envelopeValue)
{
    if (envelopeValue < 0.01)
        return 4;
    else if (envelopeValue < 0.1)
        return 3;
    else if (envelopeValue < 100)
        return 2;
    else
        return 0;
}

double ChannelEditor::snapEnvelopeValue (double rawValue)
{
    const auto scalerValue = [rawValue] ()
    {
        if (rawValue < 0.01)
            return 10000.0;
        else if (rawValue < 0.1)
            return 1000.0;
        else if (rawValue < 1.0)
            return 100.0;
        else if (rawValue < 10.0)
            return 10.0;
        else
            return 1.0;
    } ();
    return snapValue (rawValue, scalerValue);
}

double ChannelEditor::snapBitsValue (double rawValue)
{
    const auto scalerValue = [rawValue] ()
    {
        if (rawValue < 10.0)
            return 10.0;
        else
            return 1.0;
    } ();
    return snapValue (rawValue, scalerValue);
}

double ChannelEditor::truncateToDecimalPlaces (double rawValue, int decimalPlaces)
{
    return snapValue (rawValue, decimalPlaces == 0 ? 1.0 : std::pow (10, decimalPlaces));
}

double ChannelEditor::snapValue (double rawValue, double snapAmount)
{
    const double multipliedAmount { rawValue * snapAmount };
    const double truncatedAmount { std::trunc (multipliedAmount) };
    const auto snappedAmount { truncatedAmount / snapAmount };
    return snappedAmount;
}

void ChannelEditor::setupChannelComponents ()
{
    juce::XmlDocument xmlDoc { BinaryData::Assimil8orToolTips_xml };
    auto xmlElement { xmlDoc.getDocumentElement (false) };
//     if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
//         juce::Logger::outputDebugString ("XML Parsing Error for Assimil8orToolTips_xml: " + parseError);
    // NOTE: this is a hard failure, which indicates there is a problem in the file the parameterPresetXml passed in
    jassert (xmlDoc.getLastParseError () == "");
    auto toolTipsVT { juce::ValueTree::fromXml (*xmlElement) };
    ParameterToolTipData parameterToolTipData (toolTipsVT, ParameterToolTipData::WrapperType::owner, ParameterToolTipData::EnableCallbacks::no);

    auto setupLabel = [this] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        const auto textColor { juce::Colours::black };
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setColour (juce::Label::ColourIds::textColourId, textColor);
        label.setFont (label.getFont ().withPointHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this, &parameterToolTipData] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters,
                                                          juce::String parameterName)
    {
        textEditor.setJustification (justification);
        textEditor.setIndents (1, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        addAndMakeVisible (textEditor);
    };
    auto setupCvInputComboBox = [this, &parameterToolTipData] (CvInputComboBox& cvInputComboBox, juce::String parameterName, std::function<void ()> onChangeCallback)
    {
        jassert (onChangeCallback != nullptr);
        cvInputComboBox.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        cvInputComboBox.onChange = onChangeCallback;
        addAndMakeVisible (cvInputComboBox);
    };
    auto setupComboBox = [this, &parameterToolTipData] (juce::ComboBox& comboBox, juce::String parameterName, std::function<void ()> onChangeCallback)
    {
        jassert (onChangeCallback != nullptr);
        comboBox.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        comboBox.onChange = onChangeCallback;
        addAndMakeVisible (comboBox);
    };
    auto setupButton = [this, &parameterToolTipData] (juce::TextButton& textButton, juce::String text, juce::String parameterName, std::function<void ()> onClickCallback)
    {
        textButton.setButtonText (text);
        textButton.setClickingTogglesState (true);
        textButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, textButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
        textButton.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        textButton.onClick = onClickCallback;
        addAndMakeVisible (textButton);
    };

    loopLengthIsEndComboBox.setTooltip (parameterToolTipData.getToolTip ("Channel", "LoopLengthIsEnd"));

    /////////////////////////////////////////
    // column one
    // 
    // Pitch Editor
    setupLabel (pitchLabel, "PITCH", kLargeLabelSize, juce::Justification::centred);
    pitchTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getPitch (); };
    pitchTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getPitch (); };
    pitchTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    pitchTextEditor.updateDataCallback = [this] (double value) { pitchUiChanged (value); };
    setupTextEditor (pitchTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Pitch");
    pitchTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 1.0 : 0.1) * static_cast<float> (direction) };
        const auto newAmount { channelProperties.getPitch () + scrollAmount };
        pitchTextEditor.setValue (newAmount);
    };
    pitchTextEditor.onPopupMenuCallback = [this] ()
    {
        juce::PopupMenu pm;
        pm.addItem ("Copy", true, false, [this] () {});
        pm.addItem ("Paste", true, false, [this] () {});
        pm.addItem ("Default", true, false, [this] ()
        {
            channelProperties.setPitch (defaultChannelProperties.getPitch (), true);
        });
        juce::PopupMenu special;
        const auto curChannel { channelProperties.getId () - 1 };
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
        {
            if (channelIndex != curChannel)
                special.addItem ("To Channel " + juce::String (channelIndex + 1), true, false, [this, channelIndex] ()
                {
                    editManager->forChannel (channelIndex, [this, value = channelProperties.getPitch ()] (juce::ValueTree channelPropertiesVT)
                    {
                        ChannelProperties channelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                        channelProperties.setPitch (value, false);
                    });
                });
        }
        special.addItem ("To All", true, false, [this] ()
        {
            std::vector<int> channelIndexList;
            const auto srcChannelIndex { channelProperties.getId () - 1 };
            for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
                if (curChannelIndex != srcChannelIndex)
                    channelIndexList.emplace_back (curChannelIndex);
            editManager->forChannels (channelIndexList, [this, value = channelProperties.getPitch ()] (juce::ValueTree channelPropertiesVT)
            {
                ChannelProperties channelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                channelProperties.setPitch (value, false);
            });
        });
        pm.addSubMenu ("Clone", special, true);
        pm.showMenuAsync ({}, [this] (int) {});
    };
    // Pitch CV Input Selector
    setupLabel (pitchSemiLabel, "SEMI", kSmallLabelSize, juce::Justification::centredLeft);
    setupCvInputComboBox (pitchCVComboBox, "PitchCV", [this] () { pitchCVUiChanged (pitchCVComboBox.getSelectedItemText (), pitchCVTextEditor.getText ().getDoubleValue ()); });
    pitchCVComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        pitchCVComboBox.setSelectedItemIndex (std::clamp (pitchCVComboBox.getSelectedItemIndex () + scrollAmount, 0, pitchCVComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPitchCV () };
        channelProperties.setPitchCV (pitchCVComboBox.getSelectedItemText (), amount, false);
    };
    pitchCVComboBox.onPopupMenuCallback = [this] ()
    {
        juce::PopupMenu pm;
        pm.addItem ("Copy", true, false, [this] () {});
        pm.addItem ("Paste", true, false, [this] () {});
        pm.addItem ("Default", true, false, [this] ()
        {
            //channelProperties.setPitchCV (defaultChannelProperties.getPitchCV (), true);
        });
        juce::PopupMenu special;
        const auto curChannel { channelProperties.getId () - 1 };
        for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
        {
            if (channelIndex != curChannel)
                special.addItem ("To Channel " + juce::String (channelIndex + 1), true, false, [this, channelIndex] ()
                {
                    editManager->forChannel (channelIndex, [this, value = channelProperties.getPitch ()] (juce::ValueTree channelPropertiesVT)
                    {
                        ChannelProperties channelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                        //channelProperties.setPitch (value, false);
                    });
                });
        }
        special.addItem ("To All", true, false, [this] ()
        {
            std::vector<int> channelIndexList;
            const auto srcChannelIndex { channelProperties.getId () - 1 };
            for (auto curChannelIndex { 0 }; curChannelIndex < 8; ++curChannelIndex)
                if (curChannelIndex != srcChannelIndex)
                    channelIndexList.emplace_back (curChannelIndex);
            editManager->forChannels (channelIndexList, [this, value = channelProperties.getPitch ()] (juce::ValueTree channelPropertiesVT)
            {
                ChannelProperties channelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                //channelProperties.setPitch (value, false);
            });
        });
        pm.addSubMenu ("Clone", special, true);
        pm.showMenuAsync ({}, [this] (int) {});
    };
    // Pitch CV Offset Editor
    pitchCVTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPitchCV ()); };
    pitchCVTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPitchCV ()); };
    pitchCVTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPitchCV ();  };
    pitchCVTextEditor.updateDataCallback = [this] (double amount) { pitchCVUiChanged (FormatHelpers::getCvInput (channelProperties.getPitchCV ()), amount); };
    setupTextEditor (pitchCVTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PitchCV");

    // LINFM
    setupLabel (linFMLabel, "LIN FM", kLargeLabelSize, juce::Justification::centred);
    setupCvInputComboBox (linFMComboBox, "LinFM", [this] () { linFMUiChanged (linFMComboBox.getSelectedItemText (), linFMTextEditor.getText ().getDoubleValue ()); });
    linFMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        linFMComboBox.setSelectedItemIndex (std::clamp (linFMComboBox.getSelectedItemIndex () + scrollAmount, 0, linFMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLinFM () };
        channelProperties.setLinFM (linFMComboBox.getSelectedItemText (), amount, false);
    };
    linFMComboBox.onPopupMenuCallback = [this] () {};
    linFMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLinFM ()); };
    linFMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLinFM ()); };
    linFMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLinFM ();  };
    linFMTextEditor.updateDataCallback = [this] (double amount) { linFMUiChanged (FormatHelpers::getCvInput (channelProperties.getLinFM ()), amount); };
    setupTextEditor (linFMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LinFM");

    // EXPFM
    setupLabel (expFMLabel, "EXP FM", kLargeLabelSize, juce::Justification::centred);
    setupCvInputComboBox (expFMComboBox, "ExpFM", [this] () { expFMUiChanged (expFMComboBox.getSelectedItemText (), expFMTextEditor.getText ().getDoubleValue ()); });
    expFMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        expFMComboBox.setSelectedItemIndex (std::clamp (expFMComboBox.getSelectedItemIndex () + scrollAmount, 0, expFMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getExpFM () };
        channelProperties.setExpFM (expFMComboBox.getSelectedItemText (), amount, false);
    };
    expFMComboBox.onPopupMenuCallback = [this] () {};
    expFMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getExpFM ()); };
    expFMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getExpFM ()); };
    expFMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getExpFM ();  };
    expFMTextEditor.updateDataCallback = [this] (double amount) { expFMUiChanged (FormatHelpers::getCvInput (channelProperties.getExpFM ()), amount); };
    setupTextEditor (expFMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ExpFM");

    // LEVEL
    setupLabel (levelLabel, "LEVEL", kLargeLabelSize, juce::Justification::centred);
    levelTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getLevel (); };
    levelTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getLevel (); };
    levelTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, false); };
    levelTextEditor.updateDataCallback = [this] (double value) { levelUiChanged (value); };
    levelTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 0.1;
            else if (dragSpeed == DragSpeed::medium)
                return 1.0;
            else
                return (maxChannelProperties.getLevel () - minChannelProperties.getLevel ()) / 10.0;
        } ();
        const auto newValue { channelProperties.getLevel () + (multiplier * static_cast<double> (direction)) };
        levelTextEditor.setValue (newValue);
    };
    levelTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (levelTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Level");
    setupLabel (levelDbLabel, "dB", kSmallLabelSize, juce::Justification::centredLeft);

    // LINAM
    setupLabel (linAMLabel, "LIN AM", kLargeLabelSize, juce::Justification::centred);
    setupLabel (linAMisExtEnvLabel, "BIAS", kMediumLabelSize, juce::Justification::centredRight);
    linAMisExtEnvComboBox.addItem ("Normal", 1); // 0 = Normal, 1 = External Envelope
    linAMisExtEnvComboBox.addItem ("External", 2);
    setupComboBox (linAMisExtEnvComboBox, "LinAMisExtEnv", [this] ()
    {
        const auto linAMisExtEnv { linAMisExtEnvComboBox.getSelectedId () == 2 };
        linAMisExtEnvUiChanged (linAMisExtEnv);
    });
    linAMisExtEnvComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setLinAMisExtEnv (std::clamp (linAMisExtEnvComboBox.getSelectedItemIndex () + scrollAmount, 0, linAMisExtEnvComboBox.getNumItems () - 1) == 1, true);
    };
    linAMisExtEnvComboBox.onPopupMenuCallback = [this] ()
    {
    };

    setupCvInputComboBox (linAMComboBox, "LinAM", [this] () { linAMUiChanged (linAMComboBox.getSelectedItemText (), linAMTextEditor.getText ().getDoubleValue ()); });
    linAMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        linAMComboBox.setSelectedItemIndex (std::clamp (linAMComboBox.getSelectedItemIndex () + scrollAmount, 0, linAMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLinAM () };
        channelProperties.setLinAM (linAMComboBox.getSelectedItemText (), amount, false);
    };
    linAMComboBox.onPopupMenuCallback = [this] () {};
    linAMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLinAM ()); };
    linAMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLinAM ()); };
    linAMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLinAM ();  };
    linAMTextEditor.updateDataCallback = [this] (double amount) { linAMUiChanged (FormatHelpers::getCvInput (channelProperties.getLinAM ()), amount); };
    setupTextEditor (linAMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LinAM");

    // EXPAM
    setupLabel (expAMLabel, "EXP AM", kLargeLabelSize, juce::Justification::centred);
    setupCvInputComboBox (expAMComboBox, "ExpAM", [this] () { expAMUiChanged (expAMComboBox.getSelectedItemText (), expAMTextEditor.getText ().getDoubleValue ()); });
    expAMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        expAMComboBox.setSelectedItemIndex (std::clamp (expAMComboBox.getSelectedItemIndex () + scrollAmount, 0, expAMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getExpAM () };
        channelProperties.setExpAM (expAMComboBox.getSelectedItemText (), amount, false);
    };
    expAMComboBox.onPopupMenuCallback = [this] () {};
    expAMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getExpAM ()); };
    expAMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getExpAM ()); };
    expAMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getExpAM ();  };
    expAMTextEditor.updateDataCallback = [this] (double amount) { expAMUiChanged (FormatHelpers::getCvInput (channelProperties.getExpAM ()), amount); };
    setupTextEditor (expAMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ExpAM");

    /////////////////////////////////////////
    // column two
    // PHASE MOD SOURCE
    setupLabel (phaseSourceSectionLabel, "PHASE MOD", kLargeLabelSize, juce::Justification::centred);
    // PM Source Index - Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
    for (auto pmSourceIndex { 0 }; pmSourceIndex < 8; ++pmSourceIndex)
    {
        pMSourceComboBox.addItem ("Channel " + juce::String::charToString ('1' + pmSourceIndex), pmSourceIndex + 1);
    }
    pMSourceComboBox.addItem ("Right Input", 9);
    pMSourceComboBox.addItem ("Left Input", 10);
    pMSourceComboBox.addItem ("Phase CV", 11);
    setupComboBox (pMSourceComboBox, "PMSource", [this] () { pMSourceUiChanged (pMSourceComboBox.getSelectedId () - 1); });
    pMSourceComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setPMSource (std::clamp (pMSourceComboBox.getSelectedItemIndex () + scrollAmount, 0, pMSourceComboBox.getNumItems () - 1), true);
    };
    pMSourceComboBox.onPopupMenuCallback = [this] ()
    {
    };
    setupLabel (pMSourceLabel, "SRC", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (phaseCVComboBox, "PhaseCV", [this] () { phaseCVUiChanged (phaseCVComboBox.getSelectedItemText (), phaseCVTextEditor.getText ().getDoubleValue ()); });
    phaseCVComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        phaseCVComboBox.setSelectedItemIndex (std::clamp (phaseCVComboBox.getSelectedItemIndex () + scrollAmount, 0, phaseCVComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPhaseCV () };
        channelProperties.setPhaseCV (phaseCVComboBox.getSelectedItemText (), amount, false);
    };
    phaseCVComboBox.onPopupMenuCallback = [this] () {};
    phaseCVTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPhaseCV ()); };
    phaseCVTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPhaseCV ()); };
    phaseCVTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPhaseCV ();  };
    phaseCVTextEditor.updateDataCallback = [this] (double amount) { phaseCVUiChanged (FormatHelpers::getCvInput (channelProperties.getPhaseCV ()), amount); };
    setupTextEditor (phaseCVTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PhaseCV");

    // PHASE MOD INDEX
    setupLabel (phaseModIndexSectionLabel, "PHASE MOD", kLargeLabelSize, juce::Justification::centred);
    pMIndexTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getPMIndex (); };
    pMIndexTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getPMIndex (); };
    pMIndexTextEditor.updateDataCallback = [this] (double value) { pMIndexUiChanged (value); };
    pMIndexTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    pMIndexTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    pMIndexTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (pMIndexTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PMIndex");

    setupLabel (pMIndexLabel, "INDEX", kMediumLabelSize, juce::Justification::centredLeft);
    setupCvInputComboBox (pMIndexModComboBox, "PMIndexMod", [this] () { pMIndexModUiChanged (pMIndexModComboBox.getSelectedItemText (), pMIndexModTextEditor.getText ().getDoubleValue ()); });
    pMIndexModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        pMIndexModComboBox.setSelectedItemIndex (std::clamp (pMIndexModComboBox.getSelectedItemIndex () + scrollAmount, 0, pMIndexModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPMIndexMod () };
        channelProperties.setPMIndexMod (pMIndexModComboBox.getSelectedItemText (), amount, false);
    };
    pMIndexModComboBox.onPopupMenuCallback = [this] () {};
    pMIndexModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPMIndexMod ()); };
    pMIndexModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPMIndexMod ()); };
    pMIndexModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPMIndexMod ();  };
    pMIndexModTextEditor.updateDataCallback = [this] (double amount) { pMIndexModUiChanged (FormatHelpers::getCvInput (channelProperties.getPMIndexMod ()), amount); };
    setupTextEditor (pMIndexModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PMIndexMod");

    // ENVELOPE
    setupLabel (envelopeLabel, "ENVELOPE", kLargeLabelSize, juce::Justification::centred);
    // ATTACK
    setupLabel (attackFromCurrentLabel, "FROM", kMediumLabelSize, juce::Justification::centredRight);
    attackFromCurrentComboBox.addItem ("Zero", 1);
    attackFromCurrentComboBox.addItem ("Current", 2);
    setupComboBox (attackFromCurrentComboBox, "AttackFromCurrent", [this] ()
    {
        const auto attackFromCurrent { attackFromCurrentComboBox.getSelectedId () == 2 };
        attackFromCurrentUiChanged (attackFromCurrent);
    });
    attackFromCurrentComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setAttackFromCurrent (std::clamp (attackFromCurrentComboBox.getSelectedItemIndex () + scrollAmount, 0, attackFromCurrentComboBox.getNumItems () - 1) == 1, true);
    };
    attackFromCurrentComboBox.onPopupMenuCallback = [this] ()
    {
    };
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centredRight);
    attackTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getAttack (); };
    attackTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getAttack (); };
    attackTextEditor.snapValueCallback = [this] (double value) { return snapEnvelopeValue (value); };
    attackTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, getEnvelopeValueResolution (value), false); };
    attackTextEditor.updateDataCallback = [this] (double value)
    {
        arEnvelopeProperties.setAttackPercent (value / static_cast<double> (kMaxEnvelopeTime * 2), false);
        attackUiChanged (value);
    };
    attackTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    attackTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, ".0123456789", "Attack");

    setupCvInputComboBox (attackModComboBox, "AttackMod", [this] () { attackModUiChanged (attackModComboBox.getSelectedItemText (), attackModTextEditor.getText ().getDoubleValue ()); });
    attackModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        attackModComboBox.setSelectedItemIndex (std::clamp (attackModComboBox.getSelectedItemIndex () + scrollAmount, 0, attackModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getAttackMod () };
        channelProperties.setAttackMod (attackModComboBox.getSelectedItemText (), amount, false);
    };
    attackModComboBox.onPopupMenuCallback = [this] () {};
    attackModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getAttackMod ()); };
    attackModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getAttackMod ()); };
    attackModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getAttackMod ();  };
    attackModTextEditor.updateDataCallback = [this] (double amount) { attackModUiChanged (FormatHelpers::getCvInput (channelProperties.getAttackMod ()), amount); };
    setupTextEditor (attackModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "AttackMod");

    // RELEASE
    releaseTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getRelease (); };
    releaseTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getRelease (); };
    releaseTextEditor.snapValueCallback = [this] (double value) { return snapEnvelopeValue (value); };
    releaseTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, getEnvelopeValueResolution (value), false); };
    releaseTextEditor.updateDataCallback = [this] (double value)
    {
        arEnvelopeProperties.setReleasePercent (value / static_cast<double> (kMaxEnvelopeTime * 2), false);
        releaseUiChanged (value);
    };
    releaseTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    releaseTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (releaseTextEditor, juce::Justification::centred, 0, ".0123456789", "Release");

    setupLabel (releaseLabel, "RELEASE", kMediumLabelSize, juce::Justification::centredLeft);
    setupCvInputComboBox (releaseModComboBox, "ReleaseMod", [this] () { releaseModUiChanged (releaseModComboBox.getSelectedItemText (), releaseModTextEditor.getText ().getDoubleValue ()); });
    releaseModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        releaseModComboBox.setSelectedItemIndex (std::clamp (releaseModComboBox.getSelectedItemIndex () + scrollAmount, 0, releaseModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getReleaseMod () };
        channelProperties.setReleaseMod (releaseModComboBox.getSelectedItemText (), amount, false);
    };
    releaseModComboBox.onPopupMenuCallback = [this] () {};
    releaseModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getReleaseMod ()); };
    releaseModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getReleaseMod ()); };
    releaseModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getReleaseMod ();  };
    releaseModTextEditor.updateDataCallback = [this] (double amount) { releaseModUiChanged (FormatHelpers::getCvInput (channelProperties.getReleaseMod ()), amount); };
    setupTextEditor (releaseModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ReleaseMod");

    /////////////////////////////////////////
    // column three
    setupLabel (mutateLabel, "MUTATE", kLargeLabelSize, juce::Justification::centred);

    // BITS
    bitsTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getBits (); };
    bitsTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getBits (); };
    bitsTextEditor.snapValueCallback = [this] (double value) { return snapBitsValue (value); };
    bitsTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, false); };
    bitsTextEditor.updateDataCallback = [this] (double value) { bitsUiChanged (value); };
    bitsTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    bitsTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (bitsTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Bits");

    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centredRight);
    bitsModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        bitsModComboBox.setSelectedItemIndex (std::clamp (bitsModComboBox.getSelectedItemIndex () + scrollAmount, 0, bitsModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getBitsMod () };
        channelProperties.setBitsMod (bitsModComboBox.getSelectedItemText (), amount, false);
    };
    bitsModComboBox.onPopupMenuCallback = [this] () {};
    setupCvInputComboBox (bitsModComboBox, "BitsMod", [this] () { bitsModUiChanged (bitsModComboBox.getSelectedItemText (), bitsModTextEditor.getText ().getDoubleValue ()); });
    bitsModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getBitsMod ()); };
    bitsModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getBitsMod ()); };
    bitsModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getBitsMod ();  };
    bitsModTextEditor.updateDataCallback = [this] (double amount) { bitsModUiChanged (FormatHelpers::getCvInput (channelProperties.getBitsMod ()), amount); };
    setupTextEditor (bitsModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "BitsMod");

    // ALIASING
    aliasingTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getAliasing (); };
    aliasingTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getAliasing (); };
    aliasingTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    aliasingTextEditor.updateDataCallback = [this] (int value) { aliasingUiChanged (value); };
    aliasingTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    aliasingTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (aliasingTextEditor, juce::Justification::centred, 0, "0123456789", "Aliasing");

    setupLabel (aliasingLabel, "ALIAS", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (aliasingModComboBox, "AliasingMod", [this] () { aliasingModUiChanged (aliasingModComboBox.getSelectedItemText (), aliasingModTextEditor.getText ().getDoubleValue ()); });
    aliasingModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        aliasingModComboBox.setSelectedItemIndex (std::clamp (aliasingModComboBox.getSelectedItemIndex () + scrollAmount, 0, aliasingModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getAliasingMod () };
        channelProperties.setAliasingMod (aliasingModComboBox.getSelectedItemText (), amount, false);
    };
    aliasingModComboBox.onPopupMenuCallback = [this] () {};
    aliasingModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getAliasingMod ()); };
    aliasingModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getAliasingMod ()); };
    aliasingModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getAliasingMod ();  };
    aliasingModTextEditor.updateDataCallback = [this] (double amount) { aliasingModUiChanged (FormatHelpers::getCvInput (channelProperties.getAliasingMod ()), amount); };
    setupTextEditor (aliasingModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "AliasingMod");

    // REVERSE/SMOOTH
    setupButton (reverseButton, "REV", "Reverse", [this] () { reverseUiChanged (reverseButton.getToggleState ()); });
    setupButton (spliceSmoothingButton, "SMOOTH", "SpliceSmoothing", [this] () { reverseUiChanged (spliceSmoothingButton.getToggleState ()); });

    // PAN/MIX
    setupLabel (panMixLabel, "PAN/MIX", kLargeLabelSize, juce::Justification::centred);
    panTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getPan (); };
    panTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getPan (); };
    panTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    panTextEditor.updateDataCallback = [this] (double value) { panUiChanged (value); };
    panTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    panTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (panTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Pan");
    setupLabel (panLabel, "PAN", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (panModComboBox, "PanMod", [this] () { panModUiChanged (panModComboBox.getSelectedItemText (), panModTextEditor.getText ().getDoubleValue ()); });
    panModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        panModComboBox.setSelectedItemIndex (std::clamp (panModComboBox.getSelectedItemIndex () + scrollAmount, 0, panModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPanMod () };
        channelProperties.setPanMod (panModComboBox.getSelectedItemText (), amount, false);
    };
    panModComboBox.onPopupMenuCallback = [this] () {};
    panModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPanMod ()); };
    panModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPanMod ()); };
    panModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPanMod ();  };
    panModTextEditor.updateDataCallback = [this] (double amount) { panModUiChanged (FormatHelpers::getCvInput (channelProperties.getPanMod ()), amount); };
    setupTextEditor (panModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PanMod");

    mixLevelTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getMixLevel (); };
    mixLevelTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getMixLevel (); };
    mixLevelTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, false); };
    mixLevelTextEditor.updateDataCallback = [this] (double value) { mixLevelUiChanged (value); };
    mixLevelTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    mixLevelTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (mixLevelTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MixLevel");

    setupLabel (mixLevelLabel, "MIX", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (mixModComboBox, "MixMod", [this] () { mixModUiChanged (mixModComboBox.getSelectedItemText (), mixModTextEditor.getText ().getDoubleValue ()); });
    mixModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        mixModComboBox.setSelectedItemIndex (std::clamp (mixModComboBox.getSelectedItemIndex () + scrollAmount, 0, mixModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getMixMod () };
        channelProperties.setMixMod (mixModComboBox.getSelectedItemText (), amount, false);
    };
    mixModComboBox.onPopupMenuCallback = [this] () {};
    mixModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getMixMod ()); };
    mixModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getMixMod ()); };
    mixModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getMixMod ();  };
    mixModTextEditor.updateDataCallback = [this] (double amount) { mixModUiChanged (FormatHelpers::getCvInput (channelProperties.getMixMod ()), amount); };
    setupTextEditor (mixModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MixMod");
    setupLabel (mixModIsFaderLabel, "Mix Mod", kMediumLabelSize, juce::Justification::centredRight);
    mixModIsFaderComboBox.addItem ("Normal", 1);
    mixModIsFaderComboBox.addItem ("Fader", 2);
    setupComboBox (mixModIsFaderComboBox, "MixModIsFader", [this] ()
    {
        const auto mixModIsFader { mixModIsFaderComboBox.getSelectedId () == 2 };
        mixModIsFaderUiChanged (mixModIsFader);
    });
    mixModIsFaderComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setMixModIsFader (std::clamp (mixModIsFaderComboBox.getSelectedItemIndex () + scrollAmount, 0, mixModIsFaderComboBox.getNumItems () - 1) == 1, true);
    };
    mixModIsFaderComboBox.onPopupMenuCallback = [this] ()
    {
    };

    // /AUTO TRIGGER
    setupLabel (autoTriggerLabel, "TRIGGER", kMediumLabelSize, juce::Justification::centredRight);
    autoTriggerComboBox.addItem ("Normal", 1);
    autoTriggerComboBox.addItem ("Auto", 2);
    setupComboBox (autoTriggerComboBox, "AutoTrigger", [this] ()
    {
        const auto autoTrigger { autoTriggerComboBox.getSelectedId () == 2 };
        attackFromCurrentUiChanged (autoTrigger);
    });
    autoTriggerComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setAutoTrigger (std::clamp (autoTriggerComboBox.getSelectedItemIndex () + scrollAmount, 0, autoTriggerComboBox.getNumItems () - 1) == 1, true);
    };
    autoTriggerComboBox.onPopupMenuCallback = [this] ()
    {
    };

    // PLAY MODE
    setupLabel (playModeLabel, "PLAY", kMediumLabelSize, juce::Justification::centredRight);
    playModeComboBox.addItem ("Gated", 1); // 0 = Gated, 1 = One Shot
    playModeComboBox.addItem ("One Shot", 2);
    setupComboBox (playModeComboBox, "PlayMode", [this] () { playModeUiChanged (playModeComboBox.getSelectedId () - 1); });
    playModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setPlayMode (std::clamp (playModeComboBox.getSelectedItemIndex () + scrollAmount, 0, playModeComboBox.getNumItems () - 1), true);
    };
    playModeComboBox.onPopupMenuCallback = [this] ()
    {
    };

    // /LOOP MODE
    setupLabel (loopModeLabel, "LOOP", kMediumLabelSize, juce::Justification::centredRight);
    loopModeComboBox.addItem ("No Loop", 1); // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    loopModeComboBox.addItem ("Loop", 2);
    loopModeComboBox.addItem ("Loop/Release", 3);
    setupComboBox (loopModeComboBox, "LoopMode", [this] () { loopModeUiChanged (loopModeComboBox.getSelectedId () - 1); });
    loopModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setLoopMode (std::clamp (loopModeComboBox.getSelectedItemIndex () + scrollAmount, 0, loopModeComboBox.getNumItems () - 1), true);
    };
    loopModeComboBox.onPopupMenuCallback = [this] () {};

    /////////////////////////////////////////
    // column four

    // CHANNEL MODE
    setupLabel (channelModeLabel, "MODE", kMediumLabelSize, juce::Justification::centred);
    channelModeComboBox.addItem ("Master", ChannelProperties::ChannelMode::master + 1); // 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    channelModeComboBox.addItem ("Link", ChannelProperties::ChannelMode::link + 1);
    channelModeComboBox.addItem ("Stereo/Right", ChannelProperties::ChannelMode::stereoRight + 1);
    channelModeComboBox.addItem ("Cycle", ChannelProperties::ChannelMode::cycle + 1);
    setupComboBox (channelModeComboBox, "ChannelMode", [this] ()
    {
        channelModeUiChanged (channelModeComboBox.getSelectedId () - 1);
    });
    channelModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setChannelMode (std::clamp (channelModeComboBox.getSelectedItemIndex () + scrollAmount, 0, channelModeComboBox.getNumItems () - 1), true);
    };
    channelModeComboBox.onPopupMenuCallback = [this] ()
    {
    };

    // SAMPLE START MOD
    setupLabel (sampleStartModLabel, "SAMPLE START", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (sampleStartModComboBox, "SampleStartMod", [this] () { sampleStartModUiChanged (sampleStartModComboBox.getSelectedItemText (), sampleStartModTextEditor.getText ().getDoubleValue ()); });
    sampleStartModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        sampleStartModComboBox.setSelectedItemIndex (std::clamp (sampleStartModComboBox.getSelectedItemIndex () + scrollAmount, 0, sampleStartModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getSampleStartMod () };
        channelProperties.setSampleStartMod (sampleStartModComboBox.getSelectedItemText (), amount, false);
    };
    sampleStartModComboBox.onPopupMenuCallback = [this] () {};
    sampleStartModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getSampleStartMod ()); };
    sampleStartModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getSampleStartMod ()); };
    sampleStartModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getSampleStartMod ();  };
    sampleStartModTextEditor.updateDataCallback = [this] (double amount) { sampleStartModUiChanged (FormatHelpers::getCvInput (channelProperties.getSampleStartMod ()), amount); };
    setupTextEditor (sampleStartModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "SampleStartMod");

    // SAMPLE END MOD
    setupLabel (sampleEndModLabel, "SAMPLE END", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (sampleEndModComboBox, "SampleEndMod", [this] () { sampleEndModUiChanged (sampleEndModComboBox.getSelectedItemText (), sampleEndModTextEditor.getText ().getDoubleValue ()); });
    sampleEndModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        sampleEndModComboBox.setSelectedItemIndex (std::clamp (sampleEndModComboBox.getSelectedItemIndex () + scrollAmount, 0, sampleEndModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getSampleEndMod () };
        channelProperties.setSampleEndMod (sampleEndModComboBox.getSelectedItemText (), amount, false);
    };
    sampleEndModComboBox.onPopupMenuCallback = [this] () {};
    sampleEndModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getSampleEndMod ()); };
    sampleEndModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getSampleEndMod ()); };
    sampleEndModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getSampleEndMod ();  };
    sampleEndModTextEditor.updateDataCallback = [this] (double amount) { sampleEndModUiChanged (FormatHelpers::getCvInput (channelProperties.getSampleEndMod ()), amount); };
    setupTextEditor (sampleEndModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "SampleEndMod");

    // LOOP START MOD
    setupLabel (loopStartModLabel, "LOOP START", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (loopStartModComboBox, "LoopStartMod", [this] () { loopStartModUiChanged (loopStartModComboBox.getSelectedItemText (), loopStartModTextEditor.getText ().getDoubleValue ()); });
    loopStartModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        loopStartModComboBox.setSelectedItemIndex (std::clamp (loopStartModComboBox.getSelectedItemIndex () + scrollAmount, 0, loopStartModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLoopStartMod () };
        channelProperties.setLoopStartMod (loopStartModComboBox.getSelectedItemText (), amount, false);
    };
    loopStartModComboBox.onPopupMenuCallback = [this] () {};
    loopStartModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLoopStartMod ()); };
    loopStartModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLoopStartMod ()); };
    loopStartModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLoopStartMod ();  };
    loopStartModTextEditor.updateDataCallback = [this] (double amount) { loopStartModUiChanged (FormatHelpers::getCvInput (channelProperties.getLoopStartMod ()), amount); };
    setupTextEditor (loopStartModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LoopStartMod");

    // LOOP END MOD
    setupLabel (loopLengthModLabel, "LOOP LENGTH", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (loopLengthModComboBox, "LoopLengthMod", [this] () { loopLengthModUiChanged (loopLengthModComboBox.getSelectedItemText (), loopLengthModTextEditor.getText ().getDoubleValue ()); });
    loopLengthModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        loopLengthModComboBox.setSelectedItemIndex (std::clamp (loopLengthModComboBox.getSelectedItemIndex () + scrollAmount, 0, loopLengthModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLoopLengthMod () };
        channelProperties.setLoopLengthMod (loopLengthModComboBox.getSelectedItemText (), amount, false);
    };
    loopLengthModComboBox.onPopupMenuCallback = [this] () {};
    loopLengthModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLoopLengthMod ()); };
    loopLengthModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLoopLengthMod ()); };
    loopLengthModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLoopLengthMod ();  };
    loopLengthModTextEditor.updateDataCallback = [this] (double amount) { loopLengthModUiChanged (FormatHelpers::getCvInput (channelProperties.getLoopLengthMod ()), amount); };
    setupTextEditor (loopLengthModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LoopLengthMod");

    // CROSSFADE GROUP
    setupLabel (xfadeGroupLabel, "XFADE GRP", kSmallLabelSize, juce::Justification::centredRight);
    xfadeGroupComboBox.addItem ("None", 1); // Off, A, B, C, D
    xfadeGroupComboBox.addItem ("A", 2);
    xfadeGroupComboBox.addItem ("B", 3);
    xfadeGroupComboBox.addItem ("C", 4);
    xfadeGroupComboBox.addItem ("D", 5);
    xfadeGroupComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        xfadeGroupComboBox.setSelectedItemIndex (std::clamp (xfadeGroupComboBox.getSelectedItemIndex () + scrollAmount, 0, xfadeGroupComboBox.getNumItems () - 1));
        channelProperties.setXfadeGroup (xfadeGroupComboBox.getText() ,false);
    };
    xfadeGroupComboBox.onPopupMenuCallback = [this] () {};
    setupComboBox (xfadeGroupComboBox, "XfadeGroup", [this] () { xfadeGroupUiChanged (xfadeGroupComboBox.getText ()); });

    // CV ZONE SELECT
    setupLabel (zonesCVLabel, "CV", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (zonesCVComboBox, "ZonesCV", [this] () { zonesCVUiChanged (zonesCVComboBox.getSelectedItemText ()); });
    zonesCVComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        const auto newCvInputComboBoxIndex { zonesCVComboBox.getSelectedItemIndex () + scrollAmount};
        zonesCVComboBox.setSelectedItemIndex (std::clamp (newCvInputComboBoxIndex, 0, zonesCVComboBox.getNumItems () - 1));
        channelProperties.setZonesCV (zonesCVComboBox.getSelectedItemText (), false);
    };
    zonesCVComboBox.onPopupMenuCallback = [this] () {};

    // ZONE SELECT MODE
    setupLabel (zonesRTLabel, "SELECT", kSmallLabelSize, juce::Justification::centredRight);
    zonesRTComboBox.addItem ("Gate Rise", 1); // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
    zonesRTComboBox.addItem ("Continuous", 2);
    zonesRTComboBox.addItem ("Advance", 3);
    zonesRTComboBox.addItem ("Random", 4);
    setupComboBox (zonesRTComboBox, "ZonesRT", [this] () { zonesRTUiChanged (zonesRTComboBox.getSelectedId () - 1); });
    zonesRTComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setZonesRT (std::clamp (zonesRTComboBox.getSelectedItemIndex () + scrollAmount, 0, zonesRTComboBox.getNumItems () - 1), true);
    };
    zonesRTComboBox.onPopupMenuCallback = [this] () {};
}

void ChannelEditor::balanceVoltages (VoltageBalanceType balanceType)
{
    const auto numUsedZones { getNumUsedZones () };
    if (numUsedZones < 2)
        return;

    auto fillMinVoltages = [this, numUsedZones] (std::function<double (int zoneIndex)> minVoltageUpdateFunction)
    {
        jassert (minVoltageUpdateFunction != nullptr);
        for (auto curZoneIndex { numUsedZones - 2 }; curZoneIndex >= 0; --curZoneIndex)
        {
            zoneProperties [curZoneIndex].setMinVoltage (minVoltageUpdateFunction (curZoneIndex), false);
        }
    };
    switch (balanceType)
    {
        case VoltageBalanceType::distributeAcross5V:
        {
            const auto voltageRange { 5.0 / numUsedZones };
            auto curVoltage { 0.0 };
            fillMinVoltages ([&curVoltage, voltageRange] (int)
            {
                curVoltage += voltageRange;
                return curVoltage;
            });
        }
        break;
        case VoltageBalanceType::distributeAcross10V:
        {
            const auto voltageRange { 10.0 / numUsedZones };
            auto curVoltage { -5.0 };
            fillMinVoltages ([&curVoltage, voltageRange] (int)
            {
                curVoltage += voltageRange;
                return curVoltage;
            });
        }
        break;
        case VoltageBalanceType::distribute1vPerOct:
        {
            const auto voltageRange { 0.0833 };
            auto curVoltage { 0.04 };
            fillMinVoltages ([&curVoltage, voltageRange] (int)
            {
                const auto oldVoltage { curVoltage };
                curVoltage += voltageRange;
                return oldVoltage;
            });
        }
        break;
        case VoltageBalanceType::distribute1vPerOctMajor:
        {
            std::array<double, 7> majorNoteVoltages {0.08, 0.25, 0.37, 0.5, 0.67, 0.83, 0.96};
            fillMinVoltages ([majorNoteVoltages, numUsedZones] (int curZoneIndex)
            {
                return majorNoteVoltages [numUsedZones - 2 - curZoneIndex];
            });
        }
        break;
        default: jassertfalse; break;
    }
    updateAllZoneTabNames ();
}

int ChannelEditor::getNumUsedZones ()
{
    auto numUsedZones { 0 };
    for (auto curZoneIndex { 0 }; curZoneIndex < zoneProperties.size (); ++curZoneIndex)
        numUsedZones += zoneProperties [curZoneIndex].getSample ().isNotEmpty () ? 1 : 0;
    return numUsedZones;
};

std::tuple<double, double> ChannelEditor::getVoltageBoundaries (int zoneIndex, int topDepth)
    {
        auto topBoundary { 5.0 };
        auto bottomBoundary { -5.0 };

        // neither index 0 or 1 can look at the 'top boundary' index (ie. index - 2, the previous previous one)
        if (zoneIndex > topDepth)
            topBoundary = zoneProperties [zoneIndex - topDepth - 1].getMinVoltage ();
        if (zoneIndex < getNumUsedZones () - 1)
            bottomBoundary = zoneProperties [zoneIndex + 1].getMinVoltage ();
        return { topBoundary, bottomBoundary };
    };

void ChannelEditor::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT, EditManager* theEditManager,
                          SamplePool* theSamplePool, juce::ValueTree copyBufferZonePropertiesVT, bool* theZoneCopyBufferHasData)
{
    //DebugLog ("ChannelEditor["+ juce::String (channelProperties.getId ()) + "]", "init");
    jassert (theSamplePool != nullptr);
    samplePool = theSamplePool;

    jassert (theEditManager != nullptr);
    editManager = theEditManager;

    jassert (theZoneCopyBufferHasData != nullptr);
    zoneCopyBufferHasData = theZoneCopyBufferHasData;

    copyBufferZoneProperties.wrap (copyBufferZonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::no);

    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    setupChannelPropertiesCallbacks ();
    auto zoneEditorIndex { 0 };
    channelProperties.forEachZone ([this, &zoneEditorIndex, rootPropertiesVT] (juce::ValueTree zonePropertiesVT)
    {
        // Zone Editor setup
        auto& zoneEditor { zoneEditors [zoneEditorIndex] };
        zoneEditor.init (zonePropertiesVT, rootPropertiesVT, samplePool);
        zoneEditor.displayToolsMenu = [this] (int zoneIndex)
        {
            juce::PopupMenu pmBalance;
            pmBalance.addItem ("5V", true, false, [this] () { balanceVoltages (VoltageBalanceType::distributeAcross5V); });
            pmBalance.addItem ("10V", true, false, [this] () { balanceVoltages (VoltageBalanceType::distributeAcross10V); });
            pmBalance.addItem ("Kbd", true, false, [this] () { balanceVoltages (VoltageBalanceType::distribute1vPerOct); });
            pmBalance.addItem ("Maj", true, false, [this] () { balanceVoltages (VoltageBalanceType::distribute1vPerOctMajor); });

            juce::PopupMenu pm;
            pm.addSubMenu ("Balance", pmBalance, true);
            pm.addItem ("Copy", true, false, [this, zoneIndex] ()
            {
                copyZone (zoneIndex);
            });
            pm.addItem ("Paste", *zoneCopyBufferHasData, false, [this, zoneIndex] ()
            {
                pasteZone (zoneIndex);
                updateAllZoneTabNames ();
                ensureProperZoneIsSelected ();
            });
            pm.addItem ("Delete", zoneProperties [zoneIndex].getSample ().isNotEmpty (), false, [this, zoneIndex] ()
            {
                deleteZone (zoneIndex);
                ensureProperZoneIsSelected ();
                updateAllZoneTabNames ();
            });
            pm.addItem ("Insert", zoneProperties [zoneIndex].getSample ().isNotEmpty () && zoneIndex > 0 && zoneIndex < zoneProperties.size () - 1, false, [this, zoneIndex] ()
            {
                duplicateZone (zoneIndex);
                ensureProperZoneIsSelected ();
                updateAllZoneTabNames ();
            });
            pm.addItem ("Clear All", getNumUsedZones () > 0, false, [this] ()
            {
                clearAllZones ();
                ensureProperZoneIsSelected ();
                updateAllZoneTabNames ();
            });
            pm.showMenuAsync ({}, [this] (int) {});
        };
        zoneEditor.isMinVoltageInRange = [this, zoneEditorIndex] (double voltage)
        {
            const auto numUsedZones { getNumUsedZones () };
            if (zoneEditorIndex + 1 == numUsedZones)
            {
                return voltage == -5.0;
            }
            else if (zoneEditorIndex + 1 > numUsedZones)
            {
                return voltage == 0.0;
            }
            else
            {
                const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneEditorIndex, 0) };
                return voltage > bottomBoundary && voltage < topBoundary;
            }
        };
        zoneEditor.clampMinVoltage = [this, zoneEditorIndex] (double voltage)
        {
            const auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneEditorIndex, 0) };
            return std::clamp (voltage, bottomBoundary + 0.01, topBoundary - 0.01);
        };
        zoneEditor.onSampleChange = [this, zoneEditorIndex] (juce::String sampleFileName)
        {
            // TODO - is this callback even needed anymore, because assignSamples is doing the same thing this used to?
            ensureProperZoneIsSelected ();
            updateAllZoneTabNames ();
        };
        zoneEditor.assignSamples = [this] (int zoneIndex, const juce::StringArray& files)
        {
            // TODO - should this have been checked prior to this call?
            for (auto fileName : files)
                if (! zoneEditors [zoneIndex].isSupportedAudioFile (fileName))
                    return false;

            const auto initialNumZones { getNumUsedZones () };
            const auto initialEndIndex { initialNumZones - 1 };
            const auto dropZoneStartIndex { zoneIndex };
            const auto dropZoneEndIndex { zoneIndex + files.size () - 1 };
            const auto maxValue { 5.0 };
            const auto minValue { -5.0 };
            LogMinVoltageDistribution("  initialNumZones: " + juce::String (initialNumZones));
            LogMinVoltageDistribution("  initialEndIndex: " + juce::String (initialEndIndex));
            LogMinVoltageDistribution("  numFiles: " + juce::String (files.size ()));
            LogMinVoltageDistribution("  dropZoneStartIndex: " + juce::String (dropZoneStartIndex));
            LogMinVoltageDistribution("  dropZoneEndIndex: " + juce::String (dropZoneEndIndex));

            // assign the samples
            for (auto filesIndex { 0 }; filesIndex < files.size () && zoneIndex + filesIndex < 8; ++filesIndex)
            {
                auto& zoneProperty { zoneProperties [zoneIndex + filesIndex] };
                juce::File file (files [filesIndex]);
                // if file not in preset folder, then copy
                if (appProperties.getMostRecentFolder () != file.getParentDirectory ().getFullPathName ())
                {
                    // TODO handle case where file of same name already exists
                    // TODO should copy be moved to a thread?
                    file.copyFileTo (juce::File (appProperties.getMostRecentFolder ()).getChildFile (file.getFileName ()));
                    // TODO handle failure
                }
                //juce::Logger::outputDebugString ("assigning '" + file.getFileName () + "' to Zone " + juce::String (zoneIndex + filesIndex));
                // assign file to zone
                zoneProperty.setSample (file.getFileName (), false);
            }

            // update the minVoltages if needed
            if (dropZoneEndIndex - initialEndIndex > 0)
            {
                const auto initialValue { dropZoneStartIndex == 0 || (dropZoneStartIndex == 1 && initialNumZones == 1) ? maxValue : zoneProperties [initialEndIndex - 1].getMinVoltage () };
                const auto initialIndex { dropZoneStartIndex > 0 && dropZoneStartIndex == initialNumZones ? dropZoneStartIndex - 1 : dropZoneStartIndex };
                // Calculate the step size for even distribution
                const auto stepSize { (minValue - initialValue) / (dropZoneEndIndex - initialIndex + 1) };
                const auto updateIndexThreshold { initialEndIndex - 1 };

                // Update values for the requested section
                for (int curZoneIndex = initialIndex; curZoneIndex < dropZoneEndIndex; ++curZoneIndex)
                {
                    if (curZoneIndex > updateIndexThreshold)
                        zoneProperties [curZoneIndex].setMinVoltage (initialValue + (curZoneIndex - initialIndex + 1) * stepSize, false);
                }
            }

            // ensure the last zone is always -5.0
            zoneProperties [getNumUsedZones () - 1].setMinVoltage (minValue, false);
#if JUCE_DEBUG
            // verifying that all minVoltages are valid
            for (auto curZoneIndex { 0 }; curZoneIndex < getNumUsedZones () - 1; ++curZoneIndex)
                if (zoneProperties [curZoneIndex].getMinVoltage () <= zoneProperties [curZoneIndex + 1].getMinVoltage ())
                {
                    juce::Logger::outputDebugString("[" +juce::String(curZoneIndex) + "]=" + juce::String (zoneProperties [curZoneIndex].getMinVoltage ()) +
                                                    " > [" + juce::String(curZoneIndex + 1) + "]=" + juce::String (zoneProperties [curZoneIndex + 1].getMinVoltage ()));
                    jassertfalse;
                }
#endif
            updateAllZoneTabNames ();
            return true;
        };

        // Zone Properties setup
        auto& curZoneProperties { zoneProperties [zoneEditorIndex] };
        curZoneProperties.wrap (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
        curZoneProperties.onMinVoltageChange = [this, zoneEditorIndex] ([[maybe_unused]] double minVoltage)
        {
            updateAllZoneTabNames ();
        };
        ++zoneEditorIndex;
        return true;
    });

    aliasingDataChanged (channelProperties.getAliasing ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getAliasingMod (), [this] (juce::String cvInput, double value) { aliasingModDataChanged (cvInput, value); });
    attackDataChanged (channelProperties.getAttack ());
    attackFromCurrentDataChanged (channelProperties.getAttackFromCurrent ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getAttackMod (), [this] (juce::String cvInput, double value) { attackModDataChanged (cvInput, value); });
    autoTriggerDataChanged (channelProperties.getAutoTrigger ());
    bitsDataChanged (channelProperties.getBits ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getBitsMod (), [this] (juce::String cvInput, double value) { bitsModDataChanged (cvInput, value); });
    channelModeDataChanged (channelProperties.getChannelMode ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getExpAM (), [this] (juce::String cvInput, double value) { expAMDataChanged (cvInput, value); });
    PresetHelpers::setCvInputAndAmount (channelProperties.getExpFM (), [this] (juce::String cvInput, double value) { expFMDataChanged (cvInput, value); });
    levelDataChanged (channelProperties.getLevel ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getLinAM (), [this] (juce::String cvInput, double value) { linAMDataChanged (cvInput, value); });
    linAMisExtEnvDataChanged (channelProperties.getLinAMisExtEnv ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getLinFM (), [this] (juce::String cvInput, double value) { linFMDataChanged (cvInput, value); });
    loopLengthIsEndDataChanged (channelProperties.getLoopLengthIsEnd ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getLoopLengthMod (), [this] (juce::String cvInput, double value) { loopLengthModDataChanged (cvInput, value); });
    loopModeDataChanged (channelProperties.getLoopMode ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getLoopStartMod (), [this] (juce::String cvInput, double value) { loopStartModDataChanged (cvInput, value); });
    mixLevelDataChanged (channelProperties.getMixLevel ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getMixMod (), [this] (juce::String cvInput, double value) { mixModDataChanged (cvInput, value); });
    mixModIsFaderDataChanged (channelProperties.getMixModIsFader ());
    panDataChanged (channelProperties.getPan ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getPanMod (), [this] (juce::String cvInput, double value) { panModDataChanged (cvInput, value); });
    PresetHelpers::setCvInputAndAmount (channelProperties.getPhaseCV (), [this] (juce::String cvInput, double value) { phaseCVDataChanged (cvInput, value); });
    pitchDataChanged (channelProperties.getPitch ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getPitchCV (), [this] (juce::String cvInput, double value) { pitchCVDataChanged (cvInput, value); });
    playModeDataChanged (channelProperties.getPlayMode ());
    pMIndexDataChanged (channelProperties.getPMIndex ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getPMIndexMod (), [this] (juce::String cvInput, double value) { pMIndexModDataChanged (cvInput, value); });
    pMSourceDataChanged (channelProperties.getPMSource ());
    releaseDataChanged (channelProperties.getRelease ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getReleaseMod (), [this] (juce::String cvInput, double value) { releaseModDataChanged (cvInput, value); });
    reverseDataChanged (channelProperties.getReverse ());
    PresetHelpers::setCvInputAndAmount (channelProperties.getSampleStartMod (), [this] (juce::String cvInput, double value) { sampleStartModDataChanged (cvInput, value); });
    PresetHelpers::setCvInputAndAmount (channelProperties.getSampleEndMod (), [this] (juce::String cvInput, double value) { sampleEndModDataChanged (cvInput, value); });
    spliceSmoothingDataChanged (channelProperties.getSpliceSmoothing ());
    xfadeGroupDataChanged (channelProperties.getXfadeGroup ());
    zonesCVDataChanged (channelProperties.getZonesCV ());
    zonesRTDataChanged (channelProperties.getZonesRT ());

    ensureProperZoneIsSelected ();
    updateAllZoneTabNames ();
    if (isVisible ())
        configAudioPlayer ();
}

void ChannelEditor::receiveSampleLoadRequest (juce::File sampleFile)
{
    const auto zoneIndex { zoneTabs.getCurrentTabIndex () };
    auto curZoneEditor { dynamic_cast<ZoneEditor*> (zoneTabs.getTabContentComponent (zoneIndex)) };
    curZoneEditor->receiveSampleLoadRequest (sampleFile);
}

void ChannelEditor::checkSampleFileExistence ()
{
    for (auto& zoneEditor : zoneEditors)
        zoneEditor.checkSampleExistence ();
}

void ChannelEditor::setupChannelPropertiesCallbacks ()
{
    channelProperties.onIdChange = [this] ([[maybe_unused]] int id) { jassertfalse; };
    channelProperties.onAliasingChange = [this] (int aliasing) { aliasingDataChanged (aliasing);  };
    channelProperties.onAliasingModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; aliasingModDataChanged (cvInput, value); };
    channelProperties.onAttackChange = [this] (double attack) { attackDataChanged (attack); };
    channelProperties.onAttackFromCurrentChange = [this] (bool attackFromCurrent) { attackFromCurrentDataChanged (attackFromCurrent);  };
    channelProperties.onAttackModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; attackModDataChanged (cvInput, value); };
    channelProperties.onAutoTriggerChange = [this] (bool autoTrigger) { autoTriggerDataChanged (autoTrigger);  };
    channelProperties.onBitsChange = [this] (double bits) { bitsDataChanged (bits);  };
    channelProperties.onBitsModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; bitsModDataChanged (cvInput, value); };
    channelProperties.onChannelModeChange = [this] (int channelMode) { channelModeDataChanged (channelMode);  };
    channelProperties.onExpAMChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; expAMDataChanged (cvInput, value); };
    channelProperties.onExpFMChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; expFMDataChanged (cvInput, value); };
    channelProperties.onLevelChange = [this] (double level) { levelDataChanged (level);  };
    channelProperties.onLinAMChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; linAMDataChanged (cvInput, value); };
    channelProperties.onLinAMisExtEnvChange = [this] (bool linAMisExtEnv) { linAMisExtEnvDataChanged (linAMisExtEnv);  };
    channelProperties.onLinFMChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; linFMDataChanged (cvInput, value); };
    channelProperties.onLoopLengthIsEndChange = [this] (bool loopLengthIsEnd) { loopLengthIsEndDataChanged (loopLengthIsEnd); };
    channelProperties.onLoopLengthModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; loopLengthModDataChanged (cvInput, value); };
    channelProperties.onLoopModeChange = [this] (int loopMode) { loopModeDataChanged (loopMode);  };
    channelProperties.onLoopStartModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; loopStartModDataChanged (cvInput, value); };
    channelProperties.onMixLevelChange = [this] (double mixLevel) { mixLevelDataChanged (mixLevel);  };
    channelProperties.onMixModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; mixModDataChanged (cvInput, value); };
    channelProperties.onMixModIsFaderChange = [this] (bool mixModIsFader) { mixModIsFaderDataChanged (mixModIsFader);  };
    channelProperties.onPanChange = [this] (double pan) { panDataChanged (pan);  };
    channelProperties.onPanModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; panModDataChanged (cvInput, value); };
    channelProperties.onPhaseCVChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; phaseCVDataChanged (cvInput, value); };
    channelProperties.onPitchChange = [this] (double pitch) { pitchDataChanged (pitch);  };
    channelProperties.onPitchCVChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; pitchCVDataChanged (cvInput, value); };
    channelProperties.onPlayModeChange = [this] (int playMode) { playModeDataChanged (playMode);  };
    channelProperties.onPMIndexChange = [this] (double pMIndex) { pMIndexDataChanged (pMIndex);  };
    channelProperties.onPMIndexModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; pMIndexModDataChanged (cvInput, value); };
    channelProperties.onPMSourceChange = [this] (int pMSource) { pMSourceDataChanged (pMSource);  };
    channelProperties.onReleaseChange = [this] (double release) { releaseDataChanged (release); };
    channelProperties.onReleaseModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; releaseModDataChanged (cvInput, value); };
    channelProperties.onReverseChange = [this] (bool reverse) { reverseDataChanged (reverse);  };
    channelProperties.onSampleStartModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; sampleStartModDataChanged (cvInput, value); };
    channelProperties.onSampleEndModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; sampleEndModDataChanged (cvInput, value); };
    channelProperties.onSpliceSmoothingChange = [this] (bool spliceSmoothing) { spliceSmoothingDataChanged (spliceSmoothing);  };
    channelProperties.onXfadeGroupChange = [this] (juce::String xfadeGroup) { xfadeGroupDataChanged (xfadeGroup);  };
    channelProperties.onZonesCVChange = [this] (juce::String zonesCV) { zonesCVDataChanged (zonesCV);  };
    channelProperties.onZonesRTChange = [this] (int zonesRT) { zonesRTDataChanged (zonesRT);  };
}

void ChannelEditor::checkStereoRightOverlay ()
{
    stereoRightTransparantOverly.setVisible (channelProperties.getChannelMode () == ChannelProperties::ChannelMode::stereoRight);
}

void ChannelEditor::configAudioPlayer ()
{
    audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
}

void ChannelEditor::paint ([[maybe_unused]] juce::Graphics& g)
{
    auto zoneMaxVoltageBounds { zoneMaxVoltage.getBounds () };
    zoneMaxVoltageBounds = zoneMaxVoltageBounds.withX (zoneMaxVoltageBounds.getX () - 1).withY (zoneMaxVoltageBounds.getY () - 2).withHeight (zoneMaxVoltageBounds.getHeight () + 6).withTrimmedRight (5);
    g.setColour (zoneTabs.getTabBackgroundColour (0).darker (0.2f));
    g.fillRoundedRectangle (zoneMaxVoltageBounds.toFloat (), 2.0f);
    g.setColour (juce::Colours::white.darker (0.2f));
    g.drawRoundedRectangle (zoneMaxVoltageBounds.toFloat (), 1.5f, 0.4f);
}

void ChannelEditor::positionColumnOne (int xOffset, int width)
{
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    auto curYOffset { kInitialYOffset };
    pitchLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    pitchTextEditor.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);

    pitchSemiLabel.setBounds (pitchTextEditor.getRight () + 3, curYOffset + 4, scaleWidth (0.5f), kSmallLabelIntSize);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    pitchCVComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    pitchCVTextEditor.setBounds (pitchCVComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    linFMLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    linFMComboBox.setBounds (linFMLabel.getX (), curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    linFMTextEditor.setBounds (linFMComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    expFMLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    expFMComboBox.setBounds (expFMLabel.getX (), curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    expFMTextEditor.setBounds (expFMComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    levelLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    levelTextEditor.setBounds (levelLabel.getX (), curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    levelDbLabel.setBounds (levelTextEditor.getRight () + 3, curYOffset + 5, scaleWidth (0.5f), kSmallLabelIntSize);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    linAMLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    linAMisExtEnvLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.33f), kMediumLabelIntSize);
    linAMisExtEnvComboBox.setBounds (linAMisExtEnvLabel.getRight () + 3, curYOffset, scaleWidth (0.66f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    linAMComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    linAMTextEditor.setBounds (linAMComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    expAMLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    expAMComboBox.setBounds (expAMLabel.getX (), curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    expAMTextEditor.setBounds (expAMComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
}

void ChannelEditor::positionColumnTwo (int xOffset, int width)
{
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    // PHASE MOD
    auto curYOffset { kInitialYOffset };
    phaseSourceSectionLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;

    // SOURCE
    curYOffset += kFirstControlSectionYOffset;
    pMSourceLabel.setBounds (xOffset, curYOffset, scaleWidth (0.25f), kMediumLabelIntSize);
    pMSourceComboBox.setBounds (pMSourceLabel.getRight () + 3, curYOffset, scaleWidth (0.75f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    phaseCVComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    phaseCVTextEditor.setBounds (phaseCVComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    phaseModIndexSectionLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    pMIndexLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    pMIndexTextEditor.setBounds (pMIndexLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    pMIndexModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    pMIndexModTextEditor.setBounds (pMIndexModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kNewSectionOffset;
    envelopeLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    // ATTACK
    attackFromCurrentLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    attackFromCurrentComboBox.setBounds (attackFromCurrentLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    attackLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    attackTextEditor.setBounds (attackLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    attackModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    attackModTextEditor.setBounds (attackModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kInterControlYOffset;
    // RELEASE
    releaseLabel.setBounds (attackModComboBox.getX (), curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    releaseTextEditor.setBounds (releaseLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    releaseModComboBox.setBounds (releaseLabel.getX (), curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    releaseModTextEditor.setBounds (releaseModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;

    curYOffset += kInterControlYOffset;
    arEnvelopeComponent.setBounds (releaseModComboBox.getX (), curYOffset, width + 3, (kParameterLineHeight * 2) + (kInterControlYOffset * 2));
}

void ChannelEditor::positionColumnThree (int xOffset, int width)
{
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    // MUTATE
    auto curYOffset { kInitialYOffset };
    mutateLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    // BITS
    bitsLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    bitsTextEditor.setBounds (bitsLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    bitsModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    bitsModTextEditor.setBounds (bitsModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    // ALIASING
    aliasingLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    aliasingTextEditor.setBounds (aliasingLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    aliasingModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    aliasingModTextEditor.setBounds (aliasingModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    // REVERSE/SMOOTH
    reverseButton.setBounds (xOffset, curYOffset, scaleWidth (0.4f), kParameterLineHeight);
    spliceSmoothingButton.setBounds (reverseButton.getRight () + 3, curYOffset , scaleWidth (0.6f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    // PAN/MIX
    curYOffset += kNewSectionOffset;
    panMixLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    panLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    panTextEditor.setBounds (panLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    panModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    panModTextEditor.setBounds (panModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    // MIX LEEL
    mixLevelLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    mixLevelTextEditor.setBounds (mixLevelLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    mixModIsFaderLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.5f), kMediumLabelIntSize);
    mixModIsFaderComboBox.setBounds (mixModIsFaderLabel.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    mixModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    mixModTextEditor.setBounds (mixModComboBox.getRight () + 3, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
}
void ChannelEditor::positionColumnFour (int xOffset, int width)
{
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    auto curYOffset { kInitialYOffset };
    channelModeLabel.setBounds (xOffset, curYOffset, width, kMediumLabelIntSize);
    curYOffset += kMediumLabelIntSize;
    curYOffset += kInterControlYOffset;
    channelModeComboBox.setBounds (xOffset, curYOffset, width + 3, kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kNewSectionOffset;
    // AUTO TRIGGER
    autoTriggerLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.55f), kMediumLabelIntSize);
    autoTriggerComboBox.setBounds (autoTriggerLabel.getRight () + 3, curYOffset, scaleWidth (0.45f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    // PLAY MODE
    playModeLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.33f), kMediumLabelIntSize);
    playModeComboBox.setBounds (playModeLabel.getRight () + 3, curYOffset, scaleWidth (0.66f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    // SAMPLE START/END
    sampleStartModLabel.setBounds (xOffset, curYOffset, width, kMediumLabelIntSize);
    curYOffset += kMediumLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    sampleStartModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    sampleStartModTextEditor.setBounds (sampleStartModComboBox.getRight () + 3, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    sampleEndModLabel.setBounds (xOffset, curYOffset, width, kMediumLabelIntSize);
    curYOffset += kMediumLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    sampleEndModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    sampleEndModTextEditor.setBounds (sampleEndModComboBox.getRight () + 3, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kNewSectionOffset;
    // LOOP MODE
    loopModeLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.33f), kMediumLabelIntSize);
    loopModeComboBox.setBounds (loopModeLabel.getRight () + 3, curYOffset, scaleWidth (0.66f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    // LOOP START/LENGTH/END
    loopStartModLabel.setBounds (xOffset, curYOffset, width, kMediumLabelIntSize);
    curYOffset += kMediumLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    loopStartModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    loopStartModTextEditor.setBounds (loopStartModComboBox.getRight () + 3, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kInterControlYOffset;
    loopLengthModLabel.setBounds (xOffset, curYOffset, width, kMediumLabelIntSize);
    curYOffset += kMediumLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    loopLengthModComboBox.setBounds (xOffset, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    loopLengthModTextEditor.setBounds (loopLengthModComboBox.getRight () + 3, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
    curYOffset += kParameterLineHeight;
    curYOffset += kNewSectionOffset;
    // XFADE GRP
    xfadeGroupLabel.setBounds (xOffset, curYOffset + 2, scaleWidth (0.50f), kMediumLabelIntSize);
    xfadeGroupComboBox.setBounds (xfadeGroupLabel.getRight () + 3, curYOffset, scaleWidth (0.50f), kParameterLineHeight);
}

void ChannelEditor::resized ()
{
    const auto columnWidth { 100 };
    const auto spaceBetweenColumns { 40 };

    // this is the overlay that is used to indicate a channel is in Stereo/Right mode
    stereoRightTransparantOverly.setBounds (getLocalBounds ());

    if (displayToolsMenu != nullptr)
        toolsButton.setBounds (5, getHeight () - 5 - 20, 40, 20);

    // layout the Zones section. ie. the tabs and the channel level controls
    auto zoneColumn {getLocalBounds ().removeFromRight (200)};
    zoneColumn.removeFromTop (3);
    auto zoneTopSection { zoneColumn.removeFromTop (75).withTrimmedBottom (5).withTrimmedRight (3)};
    zonesLabel.setBounds (zoneTopSection.getX (), zoneTopSection.getHeight () / 2 - kMediumLabelIntSize / 2, 80, kMediumLabelIntSize);
    const auto zoneSectionInputWidth { 65 };
    const auto zoneSectionLabelWidth { 85 };
    zonesCVComboBox.setBounds (zoneTopSection.getRight () - zoneSectionInputWidth, zoneTopSection.getY () + 3, zoneSectionInputWidth, kParameterLineHeight);
    zonesCVLabel.setBounds (zonesCVComboBox.getX () - zoneSectionLabelWidth - 3, zonesCVComboBox.getY (), zoneSectionLabelWidth, kParameterLineHeight);
    zonesRTComboBox.setBounds (zoneTopSection.getRight () - zoneSectionInputWidth, zonesCVComboBox.getBottom () + 3, zoneSectionInputWidth, kParameterLineHeight);
    zonesRTLabel.setBounds (zonesRTComboBox.getX () - zoneSectionLabelWidth - 3, zonesRTComboBox.getY (), zoneSectionLabelWidth, kParameterLineHeight);
    loopLengthIsEndComboBox.setBounds (zoneTopSection.getRight () - zoneSectionInputWidth, zonesRTComboBox.getBottom () + 3, zoneSectionInputWidth, kParameterLineHeight);
    loopLengthIsEndLabel.setBounds (loopLengthIsEndComboBox.getX () - zoneSectionLabelWidth - 3, loopLengthIsEndComboBox.getY (), zoneSectionLabelWidth, kParameterLineHeight);
    zoneMaxVoltage.setBounds (zoneColumn.getX () + 2, zoneColumn.getY () - 12, 40, 11);
    zoneTabs.setBounds (zoneColumn);

    // layout the four columns of controls
    auto xOffSet { 15 };
    positionColumnOne (xOffSet, columnWidth);
    xOffSet += columnWidth + spaceBetweenColumns;
    positionColumnTwo (xOffSet, columnWidth);
    xOffSet += columnWidth + spaceBetweenColumns;
    positionColumnThree (xOffSet, columnWidth);
    xOffSet += columnWidth + spaceBetweenColumns;
    positionColumnFour (xOffSet, columnWidth);
}

void ChannelEditor::updateAllZoneTabNames ()
{
    for (auto zoneIndex { 0 }; zoneIndex < zoneTabs.getNumTabs (); ++zoneIndex)
        updateZoneTabName (zoneIndex);
}

void ChannelEditor::updateZoneTabName (int zoneIndex)
{
    auto zoneTabName { juce::String (zoneIndex + 1) };
    if (zoneProperties [zoneIndex].getSample ().isNotEmpty ())
    {
        // TODO - can/should we cache this
        auto lastUsedZone { 0 };
        for (auto curZoneIndex { 0 }; curZoneIndex < zoneProperties.size (); ++curZoneIndex)
            lastUsedZone += zoneProperties [curZoneIndex].getSample ().isNotEmpty () ? 1 : 0;

        auto minVoltage { zoneProperties [zoneIndex].getMinVoltage () };
        if (zoneIndex == lastUsedZone - 1)
           minVoltage = -5.00;

        zoneTabName += "\r" + juce::String (minVoltage >= 0.0 ? "+" : "") + juce::String (minVoltage, 2);
    }
    zoneTabs.setTabName (zoneIndex, zoneTabName);
}

void ChannelEditor::aliasingDataChanged (int aliasing)
{
    LogDataAndUiChanges ("aliasingDataChanged");
    aliasingTextEditor.setText (juce::String (aliasing));
}

void ChannelEditor::aliasingUiChanged (int aliasing)
{
    LogDataAndUiChanges ("aliasingUiChanged");
    channelProperties.setAliasing (aliasing, false);
}

void ChannelEditor::aliasingModDataChanged (juce::String cvInput, double aliasingMod)
{
    LogDataAndUiChanges ("aliasingModDataChanged");
    aliasingModComboBox.setSelectedItemText (cvInput);
    aliasingModTextEditor.setText (FormatHelpers::formatDouble (aliasingMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::aliasingModUiChanged (juce::String cvInput, double aliasingMod)
{
    LogDataAndUiChanges ("aliasingModUiChanged");
    channelProperties.setAliasingMod (cvInput, aliasingMod, false);
}

void ChannelEditor::attackDataChanged (double attack)
{
    LogDataAndUiChanges ("attackDataChanged");
    arEnvelopeProperties.setAttackPercent (attack / static_cast<double> (kMaxEnvelopeTime * 2), false);
    attackTextEditor.setText (FormatHelpers::formatDouble (attack, getEnvelopeValueResolution (attack), false));
}

void ChannelEditor::attackUiChanged (double attack)
{
    LogDataAndUiChanges ("attackUiChanged");
    channelProperties.setAttack (attack, false);
}

void ChannelEditor::attackFromCurrentDataChanged (bool attackFromCurrent)
{
    LogDataAndUiChanges ("attackFromCurrentDataChanged");
    attackFromCurrentComboBox.setSelectedId (attackFromCurrent ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::attackFromCurrentUiChanged (bool attackFromCurrent)
{
    LogDataAndUiChanges ("attackFromCurrentUiChanged");
    channelProperties.setAttackFromCurrent (attackFromCurrent, false);
}

void ChannelEditor::attackModDataChanged (juce::String cvInput, double attackMod)
{
    LogDataAndUiChanges ("attackModDataChanged");
    attackModComboBox.setSelectedItemText (cvInput);
    attackModTextEditor.setText (FormatHelpers::formatDouble (attackMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::attackModUiChanged (juce::String cvInput, double attackMod)
{
    LogDataAndUiChanges ("attackModUiChanged");
    channelProperties.setAttackMod (cvInput, attackMod, false);
}

void ChannelEditor::autoTriggerDataChanged (bool autoTrigger)
{
    LogDataAndUiChanges ("autoTriggerDataChanged");
    autoTriggerComboBox.setSelectedId (autoTrigger ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::autoTriggerUiChanged (bool autoTrigger)
{
    LogDataAndUiChanges ("autoTriggerUiChanged");
    channelProperties.setAutoTrigger (autoTrigger, false);
}

void ChannelEditor::bitsDataChanged (double bits)
{
    LogDataAndUiChanges ("bitsDataChanged");
    bitsTextEditor.setText (FormatHelpers::formatDouble (bits, 1, false));
}

void ChannelEditor::bitsUiChanged (double bits)
{
    LogDataAndUiChanges ("bitsUiChanged");
    channelProperties.setBits (bits, false);
}

void ChannelEditor::bitsModDataChanged (juce::String cvInput, double bitsMod)
{
    LogDataAndUiChanges ("bitsModDataChanged");
    bitsModComboBox.setSelectedItemText (cvInput);
    bitsModTextEditor.setText (FormatHelpers::formatDouble (bitsMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::bitsModUiChanged (juce::String cvInput, double bitsMod)
{
    LogDataAndUiChanges ("bitsModUiChanged");
    channelProperties.setBitsMod (cvInput, bitsMod, false);
}

void ChannelEditor::channelModeDataChanged (int channelMode)
{
    LogDataAndUiChanges ("channelModeDataChanged");
    channelModeComboBox.setSelectedItemIndex (channelMode, juce::NotificationType::dontSendNotification);
    checkStereoRightOverlay ();
}

void ChannelEditor::channelModeUiChanged (int channelMode)
{
    LogDataAndUiChanges ("channelModeUiChanged");
    channelProperties.setChannelMode (channelMode, false);
    checkStereoRightOverlay ();
}

void ChannelEditor::expAMDataChanged (juce::String cvInput, double expAM)
{
    LogDataAndUiChanges ("expAMDataChanged");
    expAMComboBox.setSelectedItemText (cvInput);
    expAMTextEditor.setText (FormatHelpers::formatDouble (expAM, 2, true));
}

void ChannelEditor::expAMUiChanged (juce::String cvInput, double expAM)
{
    LogDataAndUiChanges ("expAMUiChanged");
    channelProperties.setExpAM (cvInput, expAM, false);
}

void ChannelEditor::expFMDataChanged (juce::String cvInput, double expFM)
{
    LogDataAndUiChanges ("expFMDataChanged");
    expFMComboBox.setSelectedItemText (cvInput);
    expFMTextEditor.setText (FormatHelpers::formatDouble (expFM, 2, true));
}

void ChannelEditor::expFMUiChanged (juce::String cvInput, double expFM)
{
    LogDataAndUiChanges ("expFMUiChanged");
    channelProperties.setExpFM (cvInput, expFM, false);
}

void ChannelEditor::levelDataChanged (double level)
{
    LogDataAndUiChanges ("levelDataChanged");
    levelTextEditor.setText (FormatHelpers::formatDouble (level, 1, false));
}

void ChannelEditor::levelUiChanged (double level)
{
    LogDataAndUiChanges ("levelUiChanged");
    channelProperties.setLevel (level, false);
}

void ChannelEditor::linAMDataChanged (juce::String cvInput, double linAM)
{
    LogDataAndUiChanges ("linAMDataChanged");
    linAMComboBox.setSelectedItemText (cvInput);
    linAMTextEditor.setText (FormatHelpers::formatDouble (linAM, 2, true));
}

void ChannelEditor::linAMUiChanged (juce::String cvInput, double linAM)
{
    LogDataAndUiChanges ("linAMUiChanged");
    channelProperties.setLinAM (cvInput, linAM, false);
}

void ChannelEditor::linAMisExtEnvDataChanged (bool linAMisExtEnv)
{
    LogDataAndUiChanges ("linAMisExtEnvDataChanged");
    linAMisExtEnvComboBox.setSelectedId (linAMisExtEnv ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::linAMisExtEnvUiChanged (bool linAMisExtEnv)
{
    LogDataAndUiChanges ("linAMisExtEnvUiChanged");
    channelProperties.setLinAMisExtEnv (linAMisExtEnv, false);
}

void ChannelEditor::linFMDataChanged (juce::String cvInput, double linFM)
{
    LogDataAndUiChanges ("linFMDataChanged");
    linFMComboBox.setSelectedItemText (cvInput);
    linFMTextEditor.setText (FormatHelpers::formatDouble (linFM, 2, true));
}

void ChannelEditor::linFMUiChanged (juce::String cvInput, double linFM)
{
    LogDataAndUiChanges ("linFMUiChanged");
    channelProperties.setLinFM (cvInput, linFM, false);
}

void ChannelEditor::loopLengthIsEndDataChanged (bool loopLengthIsEnd)
{
    LogDataAndUiChanges ("loopLengthIsEndDataChanged");
    // inform all the zones of the change
    for (auto& zoneEditor : zoneEditors)
        zoneEditor.setLoopLengthIsEnd (loopLengthIsEnd);

    loopLengthIsEndComboBox.setSelectedId (loopLengthIsEnd ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopLengthIsEndUiChanged (bool loopLengthIsEnd)
{
    LogDataAndUiChanges ("loopLengthIsEndUiChanged");
    channelProperties.setLoopLengthIsEnd (loopLengthIsEnd, false);
}

void ChannelEditor::loopLengthModDataChanged (juce::String cvInput, double loopLengthMod)
{
    LogDataAndUiChanges ("loopLengthModDataChanged");
    loopLengthModComboBox.setSelectedItemText (cvInput);
    loopLengthModTextEditor.setText (FormatHelpers::formatDouble (loopLengthMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopLengthModUiChanged (juce::String cvInput, double loopLengthMod)
{
    LogDataAndUiChanges ("loopLengthModUiChanged");
    channelProperties.setLoopLengthMod (cvInput, loopLengthMod, false);
}

void ChannelEditor::loopModeDataChanged (int loopMode)
{
    LogDataAndUiChanges ("loopModeDataChanged");
    loopModeComboBox.setSelectedItemIndex (loopMode, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopModeUiChanged (int loopMode)
{
    LogDataAndUiChanges ("loopModeUiChanged");
    channelProperties.setLoopMode (loopMode, false);
}

void ChannelEditor::loopStartModDataChanged (juce::String cvInput, double loopStartMod)
{
    LogDataAndUiChanges ("loopStartModDataChanged");
    loopStartModComboBox.setSelectedItemText (cvInput);
    loopStartModTextEditor.setText (FormatHelpers::formatDouble (loopStartMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopStartModUiChanged (juce::String cvInput, double loopStartMod)
{
    LogDataAndUiChanges ("loopStartModUiChanged");
    channelProperties.setLoopStartMod (cvInput, loopStartMod, false);
}

void ChannelEditor::mixLevelDataChanged (double mixLevel)
{
    LogDataAndUiChanges ("mixLevelDataChanged");
    mixLevelTextEditor.setText (FormatHelpers::formatDouble (mixLevel, 1, false));
}

void ChannelEditor::mixLevelUiChanged (double mixLevel)
{
    LogDataAndUiChanges ("mixLevelUiChanged");
    channelProperties.setMixLevel (mixLevel, false);
}

void ChannelEditor::mixModDataChanged (juce::String cvInput, double mixMod)
{
    LogDataAndUiChanges ("mixModDataChanged");
    mixModComboBox.setSelectedItemText (cvInput);
    mixModTextEditor.setText (FormatHelpers::formatDouble (mixMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::mixModUiChanged (juce::String cvInput, double mixMod)
{
    LogDataAndUiChanges ("mixModUiChanged");
    channelProperties.setMixMod (cvInput, mixMod, false);
}

void ChannelEditor::mixModIsFaderDataChanged (bool mixModIsFader)
{
    LogDataAndUiChanges ("mixModIsFaderDataChanged");
    mixModIsFaderComboBox.setSelectedId (mixModIsFader ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::mixModIsFaderUiChanged (bool mixModIsFader)
{
    LogDataAndUiChanges ("mixModIsFaderUiChanged");
    channelProperties.setMixModIsFader (mixModIsFader, false);
}

void ChannelEditor::panDataChanged (double pan)
{
    LogDataAndUiChanges ("panDataChanged");
    panTextEditor.setText (FormatHelpers::formatDouble (pan, 2, true));
}

void ChannelEditor::panUiChanged (double pan)
{
    LogDataAndUiChanges ("panUiChanged");
    channelProperties.setPan (pan, false);
}

void ChannelEditor::panModDataChanged (juce::String cvInput, double panMod)
{
    LogDataAndUiChanges ("panModDataChanged");
    panModComboBox.setSelectedItemText (cvInput);
    panModTextEditor.setText (FormatHelpers::formatDouble (panMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::panModUiChanged (juce::String cvInput, double panMod)
{
    LogDataAndUiChanges ("panModUiChanged");
    channelProperties.setPanMod (cvInput, panMod, false);
}

void ChannelEditor::phaseCVDataChanged (juce::String cvInput, double phaseCV)
{
    LogDataAndUiChanges ("phaseCVDataChanged");
    phaseCVComboBox.setSelectedItemText (cvInput);
    phaseCVTextEditor.setText (FormatHelpers::formatDouble (phaseCV, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::phaseCVUiChanged (juce::String cvInput, double phaseCV)
{
    LogDataAndUiChanges ("phaseCVUiChanged");
    channelProperties.setPhaseCV (cvInput, phaseCV, false);
}

void ChannelEditor::pitchDataChanged (double pitch)
{
    LogDataAndUiChanges ("pitchDataChanged");
    pitchTextEditor.setText (FormatHelpers::formatDouble (pitch, 2, true));
}

void ChannelEditor::pitchUiChanged (double pitch)
{
    LogDataAndUiChanges ("pitchUiChanged");
    channelProperties.setPitch (pitch, false);
}

void ChannelEditor::pitchCVDataChanged (juce::String cvInput, double pitchCV)
{
    LogDataAndUiChanges ("pitchCVDataChanged");
    pitchCVComboBox.setSelectedItemText (cvInput);
    pitchCVTextEditor.setText (FormatHelpers::formatDouble (pitchCV, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::pitchCVUiChanged (juce::String cvInput, double pitchCV)
{
    LogDataAndUiChanges ("pitchCVUiChanged");
    channelProperties.setPitchCV (cvInput, pitchCV, false);
}

void ChannelEditor::playModeDataChanged (int playMode)
{
    LogDataAndUiChanges ("playModeDataChanged");
    playModeComboBox.setSelectedItemIndex (playMode, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::playModeUiChanged (int playMode)
{
    LogDataAndUiChanges ("playModeUiChanged");
    channelProperties.setPlayMode (playMode, false);
}

void ChannelEditor::pMIndexDataChanged (double pMIndex)
{
    LogDataAndUiChanges ("pMIndexDataChanged");
    pMIndexTextEditor.setText (FormatHelpers::formatDouble (pMIndex, 2, true));
}

void ChannelEditor::pMIndexUiChanged (double pMIndex)
{
    LogDataAndUiChanges ("pMIndexUiChanged");
    channelProperties.setPMIndex (pMIndex, false);
}

void ChannelEditor::pMIndexModDataChanged (juce::String cvInput, double pMIndexMod)
{
    LogDataAndUiChanges ("pMIndexModDataChanged");
    pMIndexModComboBox.setSelectedItemText (cvInput);
    pMIndexModTextEditor.setText (FormatHelpers::formatDouble (pMIndexMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::pMIndexModUiChanged (juce::String cvInput, double pMIndexMod)
{
    LogDataAndUiChanges ("pMIndexModUiChanged");
    channelProperties.setPMIndexMod (cvInput, pMIndexMod, false);
}

void ChannelEditor::pMSourceDataChanged (int pMSource)
{
    LogDataAndUiChanges ("pMSourceDataChanged");
    pMSourceComboBox.setSelectedItemIndex (pMSource, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::pMSourceUiChanged (int pMSource)
{
    LogDataAndUiChanges ("pMSourceUiChanged");
    channelProperties.setPMSource (pMSource, false);
}

void ChannelEditor::releaseDataChanged (double release)
{
    LogDataAndUiChanges ("releaseDataChanged");
    arEnvelopeProperties.setReleasePercent (release / static_cast<double> (kMaxEnvelopeTime * 2), false);
    releaseTextEditor.setText (FormatHelpers::formatDouble (release, getEnvelopeValueResolution (release), false));
}

void ChannelEditor::releaseUiChanged (double release)
{
    LogDataAndUiChanges ("releaseUiChanged");
    channelProperties.setRelease (release, false);
}

void ChannelEditor::releaseModDataChanged (juce::String cvInput, double releaseMod)
{
    LogDataAndUiChanges ("releaseModDataChanged");
    releaseModComboBox.setSelectedItemText (cvInput);
    releaseModTextEditor.setText (FormatHelpers::formatDouble (releaseMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::releaseModUiChanged (juce::String cvInput, double releaseMod)
{
    LogDataAndUiChanges ("releaseModUiChanged");
    channelProperties.setReleaseMod (cvInput, releaseMod, false);
}

void ChannelEditor::reverseDataChanged (bool reverse)
{
    LogDataAndUiChanges ("reverseDataChanged");
    reverseButton.setToggleState (reverse, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::reverseUiChanged (bool reverse)
{
    LogDataAndUiChanges ("reverseUiChanged");
    channelProperties.setReverse (reverse, false);
}

void ChannelEditor::sampleEndModDataChanged (juce::String cvInput, double sampleEndMod)
{
    LogDataAndUiChanges ("sampleEndModDataChanged");
    sampleEndModComboBox.setSelectedItemText (cvInput);
    sampleEndModTextEditor.setText (FormatHelpers::formatDouble (sampleEndMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::sampleEndModUiChanged (juce::String cvInput, double sampleEndMod)
{
    LogDataAndUiChanges ("sampleEndModUiChanged");
    channelProperties.setSampleEndMod (cvInput, sampleEndMod, false);
}

void ChannelEditor::sampleStartModDataChanged (juce::String cvInput, double sampleStartMod)
{
    LogDataAndUiChanges ("sampleStartModDataChanged");
    sampleStartModComboBox.setSelectedItemText (cvInput);
    sampleStartModTextEditor.setText (FormatHelpers::formatDouble (sampleStartMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::sampleStartModUiChanged (juce::String cvInput, double sampleStartMod)
{
    LogDataAndUiChanges ("sampleStartModUiChanged");
    channelProperties.setSampleStartMod (cvInput, sampleStartMod, false);
}

void ChannelEditor::spliceSmoothingDataChanged (bool spliceSmoothing)
{
    LogDataAndUiChanges ("spliceSmoothingDataChanged");
    spliceSmoothingButton.setToggleState (spliceSmoothing, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::spliceSmoothingUiChanged (bool spliceSmoothing)
{
    LogDataAndUiChanges ("spliceSmoothingUiChanged");
    channelProperties.setSpliceSmoothing (spliceSmoothing, false);
}

void ChannelEditor::xfadeGroupDataChanged (juce::String xfadeGroup)
{
    LogDataAndUiChanges ("xfadeGroupDataChanged");
    xfadeGroupComboBox.setText (xfadeGroup);
}

void ChannelEditor::xfadeGroupUiChanged (juce::String xfadeGroup)
{
    LogDataAndUiChanges ("xfadeGroupUiChanged");
    channelProperties.setXfadeGroup (xfadeGroup, false);
}

void ChannelEditor::zonesCVDataChanged (juce::String zonesCV)
{
    LogDataAndUiChanges ("zonesCVDataChanged");
    zonesCVComboBox.setSelectedItemText (zonesCV);
}

void ChannelEditor::zonesCVUiChanged (juce::String zonesCV)
{
    LogDataAndUiChanges ("zonesCVUiChanged");
    channelProperties.setZonesCV (zonesCV, false);
}

void ChannelEditor::zonesRTDataChanged (int zonesRT)
{
    LogDataAndUiChanges ("zonesRTDataChanged");
    zonesRTComboBox.setSelectedItemIndex (zonesRT, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::zonesRTUiChanged (int zonesRT)
{
    LogDataAndUiChanges ("zonesRTUiChanged");
    channelProperties.setZonesRT (zonesRT, false);
}
