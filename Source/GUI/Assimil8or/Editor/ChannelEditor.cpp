#include "ChannelEditor.h"
#include "FormatHelpers.h"
#include "ParameterToolTipData.h"
#include "../../../SystemServices.h"
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

    setupLabel (zonesLabel, "ZONES", kMediumLabelSize, juce::Justification::centredLeft);
    zonesLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
    setupLabel (zoneMaxVoltage, "+5.00", 10.0, juce::Justification::centredLeft);
    zoneMaxVoltage.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white.darker (0.1f));

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
        updateWaveformDisplay ();
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
    toolsButton.setTooltip ("Channel Tools");
    toolsButton.onClick = [this] ()
    {
        if (displayToolsMenu != nullptr)
            displayToolsMenu (channelIndex);
    };
    addAndMakeVisible (toolsButton);
    setupChannelComponents ();
    arEnvelopeProperties.wrap (arEnvelopeComponent.getPropertiesVT (), AREnvelopeProperties::WrapperType::client, AREnvelopeProperties::EnableCallbacks::yes);
    arEnvelopeProperties.onAttackPercentChanged = [this] (double attackPercent)
    {
        const auto rawAttackValue { (kMaxEnvelopeTime * 2) * attackPercent };
        const auto curAttackFractionalValue { channelProperties.getAttack () - static_cast<int> (channelProperties.getAttack ()) };
        const auto newAttackValue { snapEnvelopeValue (static_cast<int> (rawAttackValue) + curAttackFractionalValue) };
        attackDataChanged (newAttackValue);
        attackUiChanged (newAttackValue);
    };
    arEnvelopeProperties.onReleasePercentChanged = [this] (double releasePercent)
    {
        const auto rawReleaseValue { (kMaxEnvelopeTime *2) * releasePercent };
        const auto curReleaseFractionalValue { channelProperties.getRelease () - static_cast<int> (channelProperties.getRelease ()) };
        const auto newReleaseValue { snapEnvelopeValue (static_cast<int> (rawReleaseValue) + curReleaseFractionalValue) };
        releaseDataChanged (newReleaseValue);
        releaseUiChanged (newReleaseValue);
    };
    addAndMakeVisible (arEnvelopeComponent);

    // Waveform display
    addAndMakeVisible (sampleWaveformDisplay);

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

// TODO - move this to the EditManger
void ChannelEditor::clearAllZones ()
{
    const auto numZones { editManager->getNumUsedZones (channelIndex) };
    for (auto curZoneIndex { 0 }; curZoneIndex < numZones; ++curZoneIndex)
        zoneProperties [curZoneIndex].copyFrom (defaultZoneProperties.getValueTree (), false);
}

// TODO - move this to the EditManger
void ChannelEditor::copyZone (int zoneIndex, bool settingsOnly)
{
    copyBufferZoneProperties.copyFrom (zoneProperties [zoneIndex].getValueTree (), settingsOnly);
    if (settingsOnly)
        copyBufferZoneProperties.setSample ("", false);
    *zoneCopyBufferHasData = true;
}

// TODO - move this to the EditManger
void ChannelEditor::deleteZone (int zoneIndex)
{
    zoneProperties [zoneIndex].copyFrom (defaultZoneProperties.getValueTree (), false);
    // if this zone was the last in the list, but not also the first, then set the minVoltage for the new last in list to -5
    if (zoneIndex == editManager->getNumUsedZones (channelIndex) && zoneIndex != 0)
        zoneProperties [zoneIndex - 1].setMinVoltage (-5.0, false);
    removeEmptyZones ();
}

// TODO - move this to the EditManger
void ChannelEditor::duplicateZone (int zoneIndex)
{
    jassert (zoneIndex > 0 && zoneIndex < 7);
    const auto topBoundary { zoneProperties[zoneIndex - 1].getMinVoltage () };
    const auto bottomBoundary { zoneProperties [zoneIndex].getMinVoltage () };
    const auto newZoneVoltage { bottomBoundary + ((topBoundary - bottomBoundary) / 2) };
    for (auto curZoneIndex { 6 }; curZoneIndex >= zoneIndex; --curZoneIndex)
    {
        ZoneProperties destZoneProperties (channelProperties.getZoneVT (curZoneIndex + 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        destZoneProperties.copyFrom (channelProperties.getZoneVT (curZoneIndex), false);
    }
    zoneProperties [zoneIndex].setMinVoltage (newZoneVoltage, false);
    if (editManager->getNumUsedZones (channelIndex) == 8)
        zoneProperties [7].setMinVoltage (-5.0, false);
}

// TODO - move this to the EditManger
void ChannelEditor::pasteZone (int zoneIndex)
{
    zoneProperties [zoneIndex].copyFrom (copyBufferZoneProperties.getValueTree (), copyBufferZoneProperties.getSample ().isEmpty ());
    // if this is not on the end
    if (zoneIndex < editManager->getNumUsedZones (channelIndex) - 1)
    {
        // ensure pasted minVoltage is valid
        const auto [topBoundary, bottomBoundary] { editManager->getVoltageBoundaries (channelIndex, zoneIndex, 0) };
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
            const auto [topBoundary, _] { editManager->getVoltageBoundaries (channelIndex, zoneIndex, 1) };
            const auto prevMinVolatage { zoneProperties [zoneIndex - 1].getMinVoltage () };
            if (prevMinVolatage >= topBoundary || prevMinVolatage <= minValue)
                zoneProperties [zoneIndex - 1].setMinVoltage (minValue + ((topBoundary - minValue) / 2), false);
        }
    }
}

// TODO - move this to the EditManger
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
                    curZoneProperties.copyFrom (nextZoneProperties.getValueTree (), false);
                    nextZoneProperties.copyFrom (defaultZoneProperties.getValueTree (), false);
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

void ChannelEditor::explodeZone (int zoneIndex, int explodeCount)
{
    SampleProperties sampleProperties (sampleManagerProperties.getSamplePropertiesVT (channelIndex, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
    juce::int64 sampleSize { sampleProperties.getLengthInSamples () };
    const auto sliceSize { sampleSize / explodeCount };
    auto& sourceZoneProperties { zoneProperties [zoneIndex] };
    auto setSamplePoints = [this, sliceSize] (ZoneProperties& zpToUpdate, int index)
    {
        const auto sampleStart { index * sliceSize };
        const auto sampleEnd { sampleStart + sliceSize };
        zpToUpdate.setSampleStart (sampleStart, true);
        zpToUpdate.setSampleEnd (sampleEnd, true);
        zpToUpdate.setLoopStart (sampleStart, true);
        zpToUpdate.setLoopLength (static_cast<double> (sliceSize), true);
    };
    setSamplePoints (sourceZoneProperties, 0);

    for (auto destinationZoneIndex { zoneIndex + 1 }; destinationZoneIndex < zoneIndex + explodeCount; ++destinationZoneIndex)
    {
        auto& destZoneProperties { zoneProperties [destinationZoneIndex] };
        destZoneProperties.copyFrom (sourceZoneProperties.getValueTree (), false);
        setSamplePoints (destZoneProperties, destinationZoneIndex - zoneIndex);
    }
    zoneProperties [editManager->getNumUsedZones (channelIndex) - 1].setMinVoltage (-5.0, false);
    balanceVoltages (VoltageBalanceType::distributeAcross10V);
    ensureProperZoneIsSelected ();
    updateAllZoneTabNames ();
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

juce::PopupMenu ChannelEditor::createChannelCloneMenu (std::function <void (ChannelProperties&)> setter, std::function<bool (ChannelProperties&)> canCloneCallback,
                                                       std::function<bool (ChannelProperties&)> canCloneToAllCallback)
{
    jassert (setter != nullptr);
    jassert (canCloneCallback != nullptr);
    jassert (canCloneToAllCallback != nullptr);
    juce::PopupMenu cloneMenu;
    for (auto destChannelIndex { 0 }; destChannelIndex < 8; ++destChannelIndex)
    {
        if (destChannelIndex != channelIndex)
        {
            auto canCloneToDestChannel { true };
            editManager->forChannels ({ destChannelIndex }, [this, canCloneCallback, &canCloneToDestChannel] (juce::ValueTree channelPropertiesVT)
            {
                ChannelProperties destChannelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                canCloneToDestChannel = canCloneCallback (destChannelProperties);
            });
            if (canCloneToDestChannel)
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
    }
    std::vector<int> channelIndexList;
    // build list of other channels
    for (auto destChannelIndex { 0 }; destChannelIndex < 8; ++destChannelIndex)
        if (destChannelIndex != channelIndex)
            channelIndexList.emplace_back (destChannelIndex);
    auto canCloneDestChannelCount { 0 };
    editManager->forChannels ({ channelIndexList }, [this, canCloneToAllCallback, &canCloneDestChannelCount] (juce::ValueTree channelPropertiesVT)
    {
        ChannelProperties destChannelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        if (canCloneToAllCallback (destChannelProperties))
            ++canCloneDestChannelCount;
    });
    if (canCloneDestChannelCount > 0)
    {
        cloneMenu.addItem ("To All", true, false, [this, setter, channelIndexList] ()
        {
            // clone to other channels
            editManager->forChannels (channelIndexList, [this, setter] (juce::ValueTree channelPropertiesVT)
            {
                ChannelProperties destChannelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                setter (destChannelProperties);
            });
        });
    }
    return cloneMenu;
}

juce::PopupMenu ChannelEditor::createChannelEditMenu (std::function <void (ChannelProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter)
{
    juce::PopupMenu editMenu;
    editMenu.addSubMenu ("Clone", createChannelCloneMenu (setter, [this] (ChannelProperties&) { return true; }, [this] (ChannelProperties&) { return true; }), true);
    if (resetter != nullptr)
        editMenu.addItem ("Default", true, false, [this, resetter] () { resetter (); });
    if (reverter != nullptr)
        editMenu.addItem ("Revert", true, false, [this, reverter] () { reverter (); });

    return editMenu;
};

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

    /////////////////////////////////////////
    // column one
    //
    // PITCH SECTION LABEL
    setupLabel (pitchLabel, "PITCH", kLargeLabelSize, juce::Justification::centred);

    // PITCH EDITOR
    pitchTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getPitch (); };
    pitchTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getPitch (); };
    pitchTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    pitchTextEditor.updateDataCallback = [this] (double value) { pitchUiChanged (value); };
    pitchTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 0.01;
            else if (dragSpeed == DragSpeed::medium)
                return 0.1;
            else
                return (maxChannelProperties.getPitch () - minChannelProperties.getPitch ()) / 10.0;
        } ();
        const auto newValue { channelProperties.getPitch () + (multiplier * static_cast<double> (direction)) };
        pitchTextEditor.setValue (newValue);
    };
    pitchTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setPitch (channelProperties.getPitch (), false); },
                                               [this] () { channelProperties.setPitch (defaultChannelProperties.getPitch (), true); },
                                               [this] () { channelProperties.setPitch (uneditedChannelProperties.getPitch (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (pitchTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Pitch");

    // PITCH UNITS LABEL
    setupLabel (pitchSemiLabel, "SEMI", kSmallLabelSize, juce::Justification::centredLeft);

    // PITCH CV INPUT COMBOMBOX
    pitchCVComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        pitchCVComboBox.setSelectedItemIndex (std::clamp (pitchCVComboBox.getSelectedItemIndex () + scrollAmount, 0, pitchCVComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPitchCV () };
        channelProperties.setPitchCV (pitchCVComboBox.getSelectedItemText (), amount, false);
    };
    pitchCVComboBox.onPopupMenuCallback = [this] ()
    {
        auto pitchCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPitchCV (FormatHelpers::getCvInput (channelProperties.getPitchCV ()), FormatHelpers::getAmount (destChannelProperties.getPitchCV ()), false);
        };
        auto pitchCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getPitchCV ()};
            channelProperties.setPitchCV (defaultCvInput, FormatHelpers::getAmount (channelProperties.getPitchCV ()), true);
        };
        auto pitchCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getPitchCV ()};
            channelProperties.setPitchCV (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getPitchCV ()), true);
        };
        auto editMenu { createChannelEditMenu (pitchCVInputSetter, pitchCVInputResetter, pitchCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (pitchCVComboBox, "PitchCV", [this] () { pitchCVUiChanged (pitchCVComboBox.getSelectedItemText (), pitchCVTextEditor.getText ().getDoubleValue ()); });

    // PITCH CV OFFSET EDITOR
    pitchCVTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPitchCV ()); };
    pitchCVTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPitchCV ()); };
    pitchCVTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPitchCV ();  };
    pitchCVTextEditor.updateDataCallback = [this] (double amount) { pitchCVUiChanged (FormatHelpers::getCvInput (channelProperties.getPitchCV ()), amount); };
    pitchCVTextEditor.onPopupMenuCallback = [this] ()
    {
        auto pitchCVOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPitchCV (FormatHelpers::getCvInput (destChannelProperties.getPitchCV ()), FormatHelpers::getAmount (channelProperties.getPitchCV ()), false);
        };
        auto pitchCVOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getPitchCV ()};
            channelProperties.setPitchCV (FormatHelpers::getCvInput (channelProperties.getPitchCV ()), defaultCvAmount, true);
        };
        auto pitchCVOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getPitchCV ()};
            channelProperties.setPitchCV (FormatHelpers::getCvInput (channelProperties.getPitchCV ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (pitchCVOffsetSetter, pitchCVOffsetResetter, pitchCVOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (pitchCVTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PitchCV");

    // LINFM SECTION LABEL
    setupLabel (linFMLabel, "LIN FM", kLargeLabelSize, juce::Justification::centred);

    // LINFM CV INPUT COMBOBOX
    linFMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        linFMComboBox.setSelectedItemIndex (std::clamp (linFMComboBox.getSelectedItemIndex () + scrollAmount, 0, linFMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLinFM () };
        channelProperties.setLinFM (linFMComboBox.getSelectedItemText (), amount, false);
    };
    linFMComboBox.onPopupMenuCallback = [this] ()
    {
        auto linFMCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLinFM (FormatHelpers::getCvInput (channelProperties.getLinFM ()), FormatHelpers::getAmount (destChannelProperties.getLinFM ()), false);
        };
        auto linFMCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getLinFM ()};
            channelProperties.setLinFM (defaultCvInput, FormatHelpers::getAmount (channelProperties.getLinFM ()), true);
        };
        auto linFMCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getLinFM ()};
            channelProperties.setLinFM (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getLinFM ()), true);
        };
        auto editMenu { createChannelEditMenu (linFMCVInputSetter, linFMCVInputResetter, linFMCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (linFMComboBox, "LinFM", [this] () { linFMUiChanged (linFMComboBox.getSelectedItemText (), linFMTextEditor.getText ().getDoubleValue ()); });

    // LINFM CV OFFSET EDITOR
    linFMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLinFM ()); };
    linFMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLinFM ()); };
    linFMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLinFM ();  };
    linFMTextEditor.updateDataCallback = [this] (double amount) { linFMUiChanged (FormatHelpers::getCvInput (channelProperties.getLinFM ()), amount); };
    linFMTextEditor.onPopupMenuCallback = [this] ()
    {
        auto linFMCVOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLinFM (FormatHelpers::getCvInput (destChannelProperties.getLinFM ()), FormatHelpers::getAmount (channelProperties.getLinFM ()), false);
        };
        auto linFMCVOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getLinFM ()};
            channelProperties.setLinFM (FormatHelpers::getCvInput (channelProperties.getLinFM ()), defaultCvAmount, true);
        };
        auto linFMCVOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getLinFM ()};
            channelProperties.setLinFM (FormatHelpers::getCvInput (channelProperties.getLinFM ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (linFMCVOffsetSetter, linFMCVOffsetResetter, linFMCVOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (linFMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LinFM");

    // EXPFM SECTION LABEL
    setupLabel (expFMLabel, "EXP FM", kLargeLabelSize, juce::Justification::centred);

    // EXPFM CV INPUT COMBOBOX
    expFMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        expFMComboBox.setSelectedItemIndex (std::clamp (expFMComboBox.getSelectedItemIndex () + scrollAmount, 0, expFMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getExpFM () };
        channelProperties.setExpFM (expFMComboBox.getSelectedItemText (), amount, false);
    };
    expFMComboBox.onPopupMenuCallback = [this] ()
    {
        auto expFMCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setExpFM (FormatHelpers::getCvInput (channelProperties.getExpFM ()), FormatHelpers::getAmount (destChannelProperties.getExpFM ()), false);
        };
        auto expFMCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getExpFM ()};
            channelProperties.setPitchCV (defaultCvInput, FormatHelpers::getAmount (channelProperties.getExpFM ()), true);
        };
        auto expFMCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getExpFM ()};
            channelProperties.setPitchCV (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getExpFM ()), true);
        };
        auto editMenu { createChannelEditMenu (expFMCVInputSetter, expFMCVInputResetter, expFMCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (expFMComboBox, "ExpFM", [this] () { expFMUiChanged (expFMComboBox.getSelectedItemText (), expFMTextEditor.getText ().getDoubleValue ()); });

    // EXPFM CV OFFSET EDITOR
    expFMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getExpFM ()); };
    expFMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getExpFM ()); };
    expFMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getExpFM ();  };
    expFMTextEditor.updateDataCallback = [this] (double amount) { expFMUiChanged (FormatHelpers::getCvInput (channelProperties.getExpFM ()), amount); };
    expFMTextEditor.onPopupMenuCallback = [this] ()
    {
        auto expFMCVOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setExpFM (FormatHelpers::getCvInput (destChannelProperties.getExpFM ()), FormatHelpers::getAmount (channelProperties.getExpFM ()), false);
        };
        auto expFMCVOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getExpFM ()};
            channelProperties.setExpFM (FormatHelpers::getCvInput (channelProperties.getExpFM ()), defaultCvAmount, true);
        };
        auto expFMCVOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getExpFM ()};
            channelProperties.setExpFM (FormatHelpers::getCvInput (channelProperties.getExpFM ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (expFMCVOffsetSetter, expFMCVOffsetResetter, expFMCVOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (expFMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ExpFM");

    // LEVEL SECTION LABEL
    setupLabel (levelLabel, "LEVEL", kLargeLabelSize, juce::Justification::centred);

    // LEVEL EDITOR
    levelTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getLevel (); };
    levelTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getLevel (); };
    levelTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, false); };
    levelTextEditor.updateDataCallback = [this] (double value) { levelUiChanged (value); };
    levelTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
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
    levelTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setLevel (channelProperties.getLevel (), false); },
                                               [this] () { channelProperties.setLevel (defaultChannelProperties.getLevel (), true); },
                                               [this] () { channelProperties.setLevel (uneditedChannelProperties.getLevel (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (levelTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Level");

    // LEVEL UNITS LABEL
    setupLabel (levelDbLabel, "dB", kSmallLabelSize, juce::Justification::centredLeft);

    // LINAM SECTION LABEL
    setupLabel (linAMLabel, "LIN AM", kLargeLabelSize, juce::Justification::centred);

    // LINAM EXT ENVELOPE LABEL
    setupLabel (linAMisExtEnvLabel, "BIAS", kMediumLabelSize, juce::Justification::centredRight);

    // LINAM EXT ENVELOPE COMBOBOX
    linAMisExtEnvComboBox.addItem ("Normal", 1); // 0 = Normal, 1 = External Envelope
    linAMisExtEnvComboBox.addItem ("External", 2);
    linAMisExtEnvComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setLinAMisExtEnv (std::clamp (linAMisExtEnvComboBox.getSelectedItemIndex () + scrollAmount, 0, linAMisExtEnvComboBox.getNumItems () - 1) == 1, true);
    };
    linAMisExtEnvComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setLinAMisExtEnv (channelProperties.getLinAMisExtEnv (), false); },
                                               [this] () { channelProperties.setLinAMisExtEnv (defaultChannelProperties.getLinAMisExtEnv (), true); },
                                               [this] () { channelProperties.setLinAMisExtEnv (uneditedChannelProperties.getLinAMisExtEnv (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (linAMisExtEnvComboBox, "LinAMisExtEnv", [this] ()
    {
        const auto linAMisExtEnv { linAMisExtEnvComboBox.getSelectedId () == 2 };
        linAMisExtEnvUiChanged (linAMisExtEnv);
    });

    // LINAM CV INPUT COMBOBOX
    linAMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        linAMComboBox.setSelectedItemIndex (std::clamp (linAMComboBox.getSelectedItemIndex () + scrollAmount, 0, linAMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLinAM () };
        channelProperties.setLinAM (linAMComboBox.getSelectedItemText (), amount, false);
    };
    linAMComboBox.onPopupMenuCallback = [this] ()
    {
        auto linAMCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLinAM (FormatHelpers::getCvInput (channelProperties.getLinAM ()), FormatHelpers::getAmount (destChannelProperties.getLinAM ()), false);
        };
        auto linAMCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getLinAM ()};
            channelProperties.setLinAM (defaultCvInput, FormatHelpers::getAmount (channelProperties.getLinAM ()), true);
        };
        auto linAMCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getLinAM ()};
            channelProperties.setLinAM (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getLinAM ()), true);
        };
        auto editMenu { createChannelEditMenu (linAMCVInputSetter, linAMCVInputResetter, linAMCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (linAMComboBox, "LinAM", [this] () { linAMUiChanged (linAMComboBox.getSelectedItemText (), linAMTextEditor.getText ().getDoubleValue ()); });

    // LINAM CV OFFSET EDITOR
    linAMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLinAM ()); };
    linAMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLinAM ()); };
    linAMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLinAM ();  };
    linAMTextEditor.updateDataCallback = [this] (double amount) { linAMUiChanged (FormatHelpers::getCvInput (channelProperties.getLinAM ()), amount); };
    linAMTextEditor.onPopupMenuCallback = [this] ()
    {
        auto linAMCVOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLinAM (FormatHelpers::getCvInput (destChannelProperties.getLinAM ()), FormatHelpers::getAmount (channelProperties.getLinAM ()), false);
        };
        auto linAMCVOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getLinAM ()};
            channelProperties.setLinAM (FormatHelpers::getCvInput (channelProperties.getLinAM ()), defaultCvAmount, true);
        };
        auto linAMCVOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getLinAM ()};
            channelProperties.setLinAM (FormatHelpers::getCvInput (channelProperties.getLinAM ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (linAMCVOffsetSetter, linAMCVOffsetResetter, linAMCVOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (linAMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LinAM");

    // EXPAM SECTION LABEL
    setupLabel (expAMLabel, "EXP AM", kLargeLabelSize, juce::Justification::centred);

    // EXPAM CV INPUT COMBOBOX
    expAMComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        expAMComboBox.setSelectedItemIndex (std::clamp (expAMComboBox.getSelectedItemIndex () + scrollAmount, 0, expAMComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getExpAM () };
        channelProperties.setExpAM (expAMComboBox.getSelectedItemText (), amount, false);
    };
    expAMComboBox.onPopupMenuCallback = [this] ()
    {
        auto expAMCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setExpAM (FormatHelpers::getCvInput (channelProperties.getExpAM ()), FormatHelpers::getAmount (destChannelProperties.getExpAM ()), false);
        };
        auto expAMCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getExpAM ()};
            channelProperties.setExpAM (defaultCvInput, FormatHelpers::getAmount (channelProperties.getExpAM ()), true);
        };
        auto expAMCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getExpAM ()};
            channelProperties.setExpAM (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getExpAM ()), true);
        };
        auto editMenu { createChannelEditMenu (expAMCVInputSetter, expAMCVInputResetter, expAMCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (expAMComboBox, "ExpAM", [this] () { expAMUiChanged (expAMComboBox.getSelectedItemText (), expAMTextEditor.getText ().getDoubleValue ()); });

    // EXPAM CV OFFSET EDITOR
    expAMTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getExpAM ()); };
    expAMTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getExpAM ()); };
    expAMTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getExpAM ();  };
    expAMTextEditor.updateDataCallback = [this] (double amount) { expAMUiChanged (FormatHelpers::getCvInput (channelProperties.getExpAM ()), amount); };
    expAMTextEditor.onPopupMenuCallback = [this] ()
    {
        auto expAMCVOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setExpAM (FormatHelpers::getCvInput (destChannelProperties.getExpAM ()), FormatHelpers::getAmount (channelProperties.getExpAM ()), false);
        };
        auto expAMCVOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getExpAM ()};
            channelProperties.setExpAM (FormatHelpers::getCvInput (channelProperties.getExpAM ()), defaultCvAmount, true);
        };
        auto expAMCVOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getExpAM ()};
            channelProperties.setExpAM (FormatHelpers::getCvInput (channelProperties.getExpAM ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (expAMCVOffsetSetter, expAMCVOffsetResetter, expAMCVOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (expAMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ExpAM");

    /////////////////////////////////////////
    // column two
    // PHASE MOD SOURCE SECTION LABEL
    setupLabel (phaseSourceSectionLabel, "PHASE MOD", kLargeLabelSize, juce::Justification::centred);

    // PHASE MOD SOURCE LABEL
    setupLabel (pMSourceLabel, "SRC", kMediumLabelSize, juce::Justification::centredRight);

    // PHASE MOD SOURCE COMBOBOX
    // PM Source Index - Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
    for (auto pmSourceIndex { 0 }; pmSourceIndex < 8; ++pmSourceIndex)
    {
        pMSourceComboBox.addItem ("Channel " + juce::String::charToString ('1' + pmSourceIndex), pmSourceIndex + 1);
    }
    pMSourceComboBox.addItem ("Left Input", 9);
    pMSourceComboBox.addItem ("Right Input", 10);
    pMSourceComboBox.addItem ("Phase CV", 11);
    pMSourceComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setPMSource (std::clamp (pMSourceComboBox.getSelectedItemIndex () + scrollAmount, 0, pMSourceComboBox.getNumItems () - 1), true);
    };
    pMSourceComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setPMSource (channelProperties.getPMSource (), false); },
                                               [this] () { channelProperties.setPMSource (defaultChannelProperties.getPMSource (), true); },
                                               [this] () { channelProperties.setPMSource (uneditedChannelProperties.getPMSource (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (pMSourceComboBox, "PMSource", [this] () { pMSourceUiChanged (pMSourceComboBox.getSelectedId () - 1); });

    // PHASE MODE SOURCE CV INPUT COMBOBOX
    phaseCVComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        phaseCVComboBox.setSelectedItemIndex (std::clamp (phaseCVComboBox.getSelectedItemIndex () + scrollAmount, 0, phaseCVComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPhaseCV () };
        channelProperties.setPhaseCV (phaseCVComboBox.getSelectedItemText (), amount, false);
    };
    phaseCVComboBox.onPopupMenuCallback = [this] ()
    {
        auto phaseCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPhaseCV (FormatHelpers::getCvInput (channelProperties.getPhaseCV ()), FormatHelpers::getAmount (destChannelProperties.getPhaseCV ()), false);
        };
        auto phaseCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getPhaseCV ()};
            channelProperties.setPhaseCV (defaultCvInput, FormatHelpers::getAmount (channelProperties.getPhaseCV ()), true);
        };
        auto phaseCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getPhaseCV ()};
            channelProperties.setPhaseCV (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getPhaseCV ()), true);
        };
        auto editMenu { createChannelEditMenu (phaseCVInputSetter, phaseCVInputResetter, phaseCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (phaseCVComboBox, "PhaseCV", [this] () { phaseCVUiChanged (phaseCVComboBox.getSelectedItemText (), phaseCVTextEditor.getText ().getDoubleValue ()); });

    // PHASE MODE SOURCE CV OFFSET EDITOR
    phaseCVTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPhaseCV ()); };
    phaseCVTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPhaseCV ()); };
    phaseCVTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPhaseCV ();  };
    phaseCVTextEditor.updateDataCallback = [this] (double amount) { phaseCVUiChanged (FormatHelpers::getCvInput (channelProperties.getPhaseCV ()), amount); };
    phaseCVTextEditor.onPopupMenuCallback = [this] ()
    {
        auto phaseCVOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPhaseCV (FormatHelpers::getCvInput (destChannelProperties.getPhaseCV ()), FormatHelpers::getAmount (channelProperties.getPhaseCV ()), false);
        };
        auto phaseCVOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getPhaseCV ()};
            channelProperties.setPhaseCV (FormatHelpers::getCvInput (channelProperties.getPhaseCV ()), defaultCvAmount, true);
        };
        auto phaseCVOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getPhaseCV ()};
            channelProperties.setPhaseCV (FormatHelpers::getCvInput (channelProperties.getPhaseCV ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (phaseCVOffsetSetter, phaseCVOffsetResetter, phaseCVOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (phaseCVTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PhaseCV");

    // PHASE MOD INDEX SECTION LABEL
    setupLabel (phaseModIndexSectionLabel, "PHASE MOD", kLargeLabelSize, juce::Justification::centred);

    // PHASE MOD INDEX LABEL
    setupLabel (pMIndexLabel, "INDEX", kMediumLabelSize, juce::Justification::centredLeft);

    // PHASE MOD INDEX EDITOR
    pMIndexTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getPMIndex (); };
    pMIndexTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getPMIndex (); };
    pMIndexTextEditor.updateDataCallback = [this] (double value) { pMIndexUiChanged (value); };
    pMIndexTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    pMIndexTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 0.01;
            else if (dragSpeed == DragSpeed::medium)
                return 0.1;
            else
                return 1.0;
        } ();
        const auto newValue { channelProperties.getPMIndex () + (multiplier * static_cast<double> (direction)) };
        pMIndexTextEditor.setValue (newValue);
    };
    pMIndexTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setPMIndex (channelProperties.getPMIndex (), false); },
                                               [this] () { channelProperties.setPMIndex (defaultChannelProperties.getPMIndex (), true); },
                                               [this] () { channelProperties.setPMIndex (uneditedChannelProperties.getPMIndex (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (pMIndexTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PMIndex");

    // PHASE MOD INDEX CV INPUT COMBOBOX
    pMIndexModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        pMIndexModComboBox.setSelectedItemIndex (std::clamp (pMIndexModComboBox.getSelectedItemIndex () + scrollAmount, 0, pMIndexModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPMIndexMod () };
        channelProperties.setPMIndexMod (pMIndexModComboBox.getSelectedItemText (), amount, false);
    };
    pMIndexModComboBox.onPopupMenuCallback = [this] ()
    {
        auto pMIndexModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPMIndexMod (FormatHelpers::getCvInput (channelProperties.getPMIndexMod ()), FormatHelpers::getAmount (destChannelProperties.getPMIndexMod ()), false);
        };
        auto pMIndexModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getPMIndexMod ()};
            channelProperties.setPMIndexMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getPMIndexMod ()), true);
        };
        auto pMIndexModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getPMIndexMod ()};
            channelProperties.setPMIndexMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getPMIndexMod ()), true);
        };
        auto editMenu { createChannelEditMenu (pMIndexModCVInputSetter, pMIndexModCVInputResetter, pMIndexModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (pMIndexModComboBox, "PMIndexMod", [this] () { pMIndexModUiChanged (pMIndexModComboBox.getSelectedItemText (), pMIndexModTextEditor.getText ().getDoubleValue ()); });

    // PHASE MOD INDEX CV OFFSET EDITOR
    pMIndexModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPMIndexMod ()); };
    pMIndexModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPMIndexMod ()); };
    pMIndexModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPMIndexMod ();  };
    pMIndexModTextEditor.updateDataCallback = [this] (double amount) { pMIndexModUiChanged (FormatHelpers::getCvInput (channelProperties.getPMIndexMod ()), amount); };
    pMIndexModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto pMIndexModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPMIndexMod (FormatHelpers::getCvInput (destChannelProperties.getPMIndexMod ()), FormatHelpers::getAmount (channelProperties.getPMIndexMod ()), false);
        };
        auto pMIndexModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getPMIndexMod ()};
            channelProperties.setPMIndexMod (FormatHelpers::getCvInput (channelProperties.getPMIndexMod ()), defaultCvAmount, true);
        };
        auto pMIndexModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getPMIndexMod ()};
            channelProperties.setPMIndexMod (FormatHelpers::getCvInput (channelProperties.getPMIndexMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (pMIndexModOffsetSetter, pMIndexModOffsetResetter, pMIndexModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (pMIndexModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PMIndexMod");

    // ENVELOPE SECTION LABEL
    setupLabel (envelopeLabel, "ENVELOPE", kLargeLabelSize, juce::Justification::centred);

    // ATTACK START FROM LABEL
    setupLabel (attackFromCurrentLabel, "FROM", kMediumLabelSize, juce::Justification::centredRight);

    // ATTACK START FROM COMBOBOX
    attackFromCurrentComboBox.addItem ("Zero", 1);
    attackFromCurrentComboBox.addItem ("Current", 2);
    attackFromCurrentComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setAttackFromCurrent (std::clamp (attackFromCurrentComboBox.getSelectedItemIndex () + scrollAmount, 0, attackFromCurrentComboBox.getNumItems () - 1) == 1, true);
    };
    attackFromCurrentComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setAttackFromCurrent (channelProperties.getAttackFromCurrent (), false); },
                                               [this] () { channelProperties.setAttackFromCurrent (defaultChannelProperties.getAttackFromCurrent (), true); },
                                               [this] () { channelProperties.setAttackFromCurrent (uneditedChannelProperties.getAttackFromCurrent (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (attackFromCurrentComboBox, "AttackFromCurrent", [this] ()
    {
        const auto attackFromCurrent { attackFromCurrentComboBox.getSelectedId () == 2 };
        attackFromCurrentUiChanged (attackFromCurrent);
    });

    // ATTACK LABEL
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centredRight);

    // ATTACK EDITOR
    attackTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getAttack (); };
    attackTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getAttack (); };
    attackTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, getEnvelopeValueResolution (value), false); };
    attackTextEditor.updateDataCallback = [this] (double value)
    {
        arEnvelopeProperties.setAttackPercent (value / static_cast<double> (kMaxEnvelopeTime * 2), false);
        attackUiChanged (value);
    };
    attackTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            const auto scalerValue = [rawValue = channelProperties.getAttack ()] ()
            {
                if (rawValue < 0.01)
                    return 0.0001;
                else if (rawValue < 0.1)
                    return 0.001;
                else if (rawValue < 1.0)
                    return 0.1;
                else
                    return 1.0;
            } ();

            if (dragSpeed == DragSpeed::slow)
                return scalerValue;
            else if (dragSpeed == DragSpeed::medium)
                return scalerValue * 5.0;
            else
                return (maxChannelProperties.getAttack () - minChannelProperties.getAttack ()) / 10.0;
        } ();
        const auto newValue { channelProperties.getAttack () + (multiplier * static_cast<double> (direction)) };
        attackTextEditor.setValue (newValue);
    };
    attackTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setAttack (channelProperties.getAttack (), false); },
                                               [this] () { channelProperties.setAttack (defaultChannelProperties.getAttack (), true); },
                                               [this] () { channelProperties.setAttack (uneditedChannelProperties.getAttack (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, ".0123456789", "Attack");

    // ATTACK CV INPUT COMBOBOX
    attackModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        attackModComboBox.setSelectedItemIndex (std::clamp (attackModComboBox.getSelectedItemIndex () + scrollAmount, 0, attackModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getAttackMod () };
        channelProperties.setAttackMod (attackModComboBox.getSelectedItemText (), amount, false);
    };
    attackModComboBox.onPopupMenuCallback = [this] ()
    {
        auto attackModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setAttackMod (FormatHelpers::getCvInput (channelProperties.getAttackMod ()), FormatHelpers::getAmount (destChannelProperties.getAttackMod ()), false);
        };
        auto attackModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getAttackMod ()};
            channelProperties.setAttackMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getAttackMod ()), true);
        };
        auto attackModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getAttackMod ()};
            channelProperties.setAttackMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getAttackMod ()), true);
        };
        auto editMenu { createChannelEditMenu (attackModCVInputSetter, attackModCVInputResetter, attackModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (attackModComboBox, "AttackMod", [this] () { attackModUiChanged (attackModComboBox.getSelectedItemText (), attackModTextEditor.getText ().getDoubleValue ()); });

    // ATTACK CV OFFSET EDITOR
    attackModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getAttackMod ()); };
    attackModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getAttackMod ()); };
    attackModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getAttackMod ();  };
    attackModTextEditor.updateDataCallback = [this] (double amount) { attackModUiChanged (FormatHelpers::getCvInput (channelProperties.getAttackMod ()), amount); };
    attackModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto attackModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setAttackMod (FormatHelpers::getCvInput (destChannelProperties.getAttackMod ()), FormatHelpers::getAmount (channelProperties.getAttackMod ()), false);
        };
        auto attackModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getAttackMod ()};
            channelProperties.setAttackMod (FormatHelpers::getCvInput (channelProperties.getAttackMod ()), defaultCvAmount, true);
        };
        auto attackModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getAttackMod ()};
            channelProperties.setAttackMod (FormatHelpers::getCvInput (channelProperties.getAttackMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (attackModOffsetSetter, attackModOffsetResetter, attackModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (attackModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "AttackMod");

    // RELEASE LABEL
    setupLabel (releaseLabel, "RELEASE", kMediumLabelSize, juce::Justification::centredLeft);

    // RELEASE EDITOR
    releaseTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getRelease (); };
    releaseTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getRelease (); };
    releaseTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, getEnvelopeValueResolution (value), false); };
    releaseTextEditor.updateDataCallback = [this] (double value)
    {
        arEnvelopeProperties.setReleasePercent (value / static_cast<double> (kMaxEnvelopeTime * 2), false);
        releaseUiChanged (value);
    };
    releaseTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            const auto scalerValue = [rawValue = channelProperties.getRelease ()] ()
            {
                if (rawValue < 0.01)
                    return 0.0001;
                else if (rawValue < 0.1)
                    return 0.001;
                else if (rawValue < 1.0)
                    return 0.1;
                else
                    return 1.0;
            } ();

                if (dragSpeed == DragSpeed::slow)
                    return scalerValue;
                else if (dragSpeed == DragSpeed::medium)
                    return scalerValue * 5.0;
                else
                    return (maxChannelProperties.getRelease () - minChannelProperties.getRelease ()) / 10.0;
        } ();
        const auto newValue { channelProperties.getRelease () + (multiplier * static_cast<double> (direction)) };
        releaseTextEditor.setValue (newValue);
    };
    releaseTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setRelease (channelProperties.getRelease (), false); },
                                               [this] () { channelProperties.setRelease (defaultChannelProperties.getRelease (), true); },
                                               [this] () { channelProperties.setRelease (uneditedChannelProperties.getRelease (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (releaseTextEditor, juce::Justification::centred, 0, ".0123456789", "Release");

    // RELEASE CV INPUT COMBOBOX
    releaseModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        releaseModComboBox.setSelectedItemIndex (std::clamp (releaseModComboBox.getSelectedItemIndex () + scrollAmount, 0, releaseModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getReleaseMod () };
        channelProperties.setReleaseMod (releaseModComboBox.getSelectedItemText (), amount, false);
    };
    releaseModComboBox.onPopupMenuCallback = [this] ()
    {
        auto releaseModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setReleaseMod (FormatHelpers::getCvInput (channelProperties.getReleaseMod ()), FormatHelpers::getAmount (destChannelProperties.getReleaseMod ()), false);
        };
        auto releaseModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getReleaseMod ()};
            channelProperties.setReleaseMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getReleaseMod ()), true);
        };
        auto releaseModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getReleaseMod ()};
            channelProperties.setReleaseMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getReleaseMod ()), true);
        };
        auto editMenu { createChannelEditMenu (releaseModCVInputSetter, releaseModCVInputResetter, releaseModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (releaseModComboBox, "ReleaseMod", [this] () { releaseModUiChanged (releaseModComboBox.getSelectedItemText (), releaseModTextEditor.getText ().getDoubleValue ()); });

    // RELEASE CV OFFSET EDITOR
    releaseModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getReleaseMod ()); };
    releaseModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getReleaseMod ()); };
    releaseModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getReleaseMod ();  };
    releaseModTextEditor.updateDataCallback = [this] (double amount) { releaseModUiChanged (FormatHelpers::getCvInput (channelProperties.getReleaseMod ()), amount); };
    releaseModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto releaseModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setReleaseMod (FormatHelpers::getCvInput (destChannelProperties.getReleaseMod ()), FormatHelpers::getAmount (channelProperties.getReleaseMod ()), false);
        };
        auto releaseModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getReleaseMod ()};
            channelProperties.setReleaseMod (FormatHelpers::getCvInput (channelProperties.getReleaseMod ()), defaultCvAmount, true);
        };
        auto releaseModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getReleaseMod ()};
            channelProperties.setReleaseMod (FormatHelpers::getCvInput (channelProperties.getReleaseMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (releaseModOffsetSetter, releaseModOffsetResetter, releaseModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (releaseModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ReleaseMod");

    /////////////////////////////////////////
    // column three

    // MUTATE SECTION LABEL
    setupLabel (mutateLabel, "MUTATE", kLargeLabelSize, juce::Justification::centred);

    // BITS LABEL
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centredRight);

    // BITS EDITOR
    bitsTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getBits (); };
    bitsTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getBits (); };
    bitsTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, false); };
    bitsTextEditor.updateDataCallback = [this] (double value) { bitsUiChanged (value); };
    bitsTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            const auto scalerValue = [rawValue = channelProperties.getBits ()] ()
            {
                if (rawValue < 10.0)
                    return 0.1;
                else
                    return 1.0;
            } ();

            if (dragSpeed == DragSpeed::slow)
                return scalerValue;
            else if (dragSpeed == DragSpeed::medium)
                return scalerValue * 5.0;
            else
                return (maxChannelProperties.getBits () - minChannelProperties.getBits ()) / 10.0;
        } ();
        const auto newValue { channelProperties.getBits () + (multiplier * static_cast<double> (direction)) };
        bitsTextEditor.setValue (newValue);
    };
    bitsTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setBits (channelProperties.getBits (), false); },
                                               [this] () { channelProperties.setBits (defaultChannelProperties.getBits (), true); },
                                               [this] () { channelProperties.setBits (uneditedChannelProperties.getBits (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (bitsTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Bits");

    // BITS CV INPUT COMBOBOX
    bitsModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        bitsModComboBox.setSelectedItemIndex (std::clamp (bitsModComboBox.getSelectedItemIndex () + scrollAmount, 0, bitsModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getBitsMod () };
        channelProperties.setBitsMod (bitsModComboBox.getSelectedItemText (), amount, false);
    };
    bitsModComboBox.onPopupMenuCallback = [this] ()
    {
        auto bitsModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setBitsMod (FormatHelpers::getCvInput (channelProperties.getBitsMod ()), FormatHelpers::getAmount (destChannelProperties.getBitsMod ()), false);
        };
        auto bitsModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getBitsMod ()};
            channelProperties.setBitsMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getBitsMod ()), true);
        };
        auto bitsModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getBitsMod ()};
            channelProperties.setBitsMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getBitsMod ()), true);
        };
        auto editMenu { createChannelEditMenu (bitsModCVInputSetter, bitsModCVInputResetter, bitsModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (bitsModComboBox, "BitsMod", [this] () { bitsModUiChanged (bitsModComboBox.getSelectedItemText (), bitsModTextEditor.getText ().getDoubleValue ()); });

    // BITS CV OFFSET EDITOR
    bitsModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getBitsMod ()); };
    bitsModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getBitsMod ()); };
    bitsModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getBitsMod ();  };
    bitsModTextEditor.updateDataCallback = [this] (double amount) { bitsModUiChanged (FormatHelpers::getCvInput (channelProperties.getBitsMod ()), amount); };
    bitsModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto bitsModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setBitsMod (FormatHelpers::getCvInput (destChannelProperties.getBitsMod ()), FormatHelpers::getAmount (channelProperties.getBitsMod ()), false);
        };
        auto bitsModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getBitsMod ()};
            channelProperties.setBitsMod (FormatHelpers::getCvInput (channelProperties.getBitsMod ()), defaultCvAmount, true);
        };
        auto bitsModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getBitsMod ()};
            channelProperties.setBitsMod (FormatHelpers::getCvInput (channelProperties.getBitsMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (bitsModOffsetSetter, bitsModOffsetResetter, bitsModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (bitsModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "BitsMod");

    // ALIAS LABEL
    setupLabel (aliasingLabel, "ALIAS", kMediumLabelSize, juce::Justification::centredRight);

    // ALIAS EDITOR
    aliasingTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getAliasing (); };
    aliasingTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getAliasing (); };
    aliasingTextEditor.toStringCallback = [this] (int value) { return juce::String (value); };
    aliasingTextEditor.updateDataCallback = [this] (int value) { aliasingUiChanged (value); };
    aliasingTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 1;
            else if (dragSpeed == DragSpeed::medium)
                return 5;
            else
                return 10;
        } ();
        const auto newValue { channelProperties.getAliasing () + (multiplier * direction) };
        aliasingTextEditor.setValue (newValue);
    };
    aliasingTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setAliasing (channelProperties.getAliasing (), false); },
                                               [this] () { channelProperties.setAliasing (defaultChannelProperties.getAliasing (), true); },
                                               [this] () { channelProperties.setAliasing (uneditedChannelProperties.getAliasing (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (aliasingTextEditor, juce::Justification::centred, 0, "0123456789", "Aliasing");

    // ALIAS CV INPUT COMBOBOX
    aliasingModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        aliasingModComboBox.setSelectedItemIndex (std::clamp (aliasingModComboBox.getSelectedItemIndex () + scrollAmount, 0, aliasingModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getAliasingMod () };
        channelProperties.setAliasingMod (aliasingModComboBox.getSelectedItemText (), amount, false);
    };
    aliasingModComboBox.onPopupMenuCallback = [this] ()
    {
        auto aliasingModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setAliasingMod (FormatHelpers::getCvInput (channelProperties.getAliasingMod ()), FormatHelpers::getAmount (destChannelProperties.getAliasingMod ()), false);
        };
        auto aliasingModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getAliasingMod ()};
            channelProperties.setAliasingMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getAliasingMod ()), true);
        };
        auto aliasingModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getAliasingMod ()};
            channelProperties.setAliasingMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getAliasingMod ()), true);
        };
        auto editMenu { createChannelEditMenu (aliasingModCVInputSetter, aliasingModCVInputResetter, aliasingModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (aliasingModComboBox, "AliasingMod", [this] () { aliasingModUiChanged (aliasingModComboBox.getSelectedItemText (), aliasingModTextEditor.getText ().getDoubleValue ()); });

    // ALIAS CV OFFSET EDITOR
    aliasingModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getAliasingMod ()); };
    aliasingModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getAliasingMod ()); };
    aliasingModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getAliasingMod ();  };
    aliasingModTextEditor.updateDataCallback = [this] (double amount) { aliasingModUiChanged (FormatHelpers::getCvInput (channelProperties.getAliasingMod ()), amount); };
    aliasingModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto aliasingModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setAliasingMod (FormatHelpers::getCvInput (destChannelProperties.getAliasingMod ()), FormatHelpers::getAmount (channelProperties.getAliasingMod ()), false);
        };
        auto aliasingModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getAliasingMod ()};
            channelProperties.setAliasingMod (FormatHelpers::getCvInput (channelProperties.getAliasingMod ()), defaultCvAmount, true);
        };
        auto aliasingModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getAliasingMod ()};
            channelProperties.setAliasingMod (FormatHelpers::getCvInput (channelProperties.getAliasingMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (aliasingModOffsetSetter, aliasingModOffsetResetter, aliasingModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (aliasingModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "AliasingMod");

    // REVERSE BUTTON
    reverseButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setReverse (channelProperties.getReverse (), false); },
                                               [this] () { channelProperties.setReverse (defaultChannelProperties.getReverse (), true); },
                                               [this] () { channelProperties.setReverse (uneditedChannelProperties.getReverse (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupButton (reverseButton, "REV", "Reverse", [this] () { reverseUiChanged (reverseButton.getToggleState ()); });

    // SMOOTH BUTTON
    spliceSmoothingButton.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setSpliceSmoothing (channelProperties.getSpliceSmoothing (), false); },
                                               [this] () { channelProperties.setSpliceSmoothing (defaultChannelProperties.getSpliceSmoothing (), true); },
                                               [this] () { channelProperties.setSpliceSmoothing (uneditedChannelProperties.getSpliceSmoothing (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupButton (spliceSmoothingButton, "SMOOTH", "SpliceSmoothing", [this] () { spliceSmoothingUiChanged (spliceSmoothingButton.getToggleState ()); });

    // PAN/MIX SECTION LABEL
    setupLabel (panMixLabel, "PAN/MIX", kLargeLabelSize, juce::Justification::centred);

    // PAN LABEL
    setupLabel (panLabel, "PAN", kMediumLabelSize, juce::Justification::centredRight);

    // PAN EDITOR
    panTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getPan (); };
    panTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getPan (); };
    panTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    panTextEditor.updateDataCallback = [this] (double value) { panUiChanged (value); };
    panTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 0.01;
            else if (dragSpeed == DragSpeed::medium)
                return 0.1;
            else
                return 0.5;
        } ();
        const auto newValue { channelProperties.getPan () + (multiplier * static_cast<double> (direction)) };
        panTextEditor.setValue (newValue);
    };
    panTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setPan (channelProperties.getPan (), false); },
                                               [this] () { channelProperties.setPan (defaultChannelProperties.getPan (), true); },
                                               [this] () { channelProperties.setPan (uneditedChannelProperties.getPan (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (panTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Pan");

    // PAN CV INPUT COMBOBOX
    panModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        panModComboBox.setSelectedItemIndex (std::clamp (panModComboBox.getSelectedItemIndex () + scrollAmount, 0, panModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getPanMod () };
        channelProperties.setPanMod (panModComboBox.getSelectedItemText (), amount, false);
    };
    panModComboBox.onPopupMenuCallback = [this] ()
    {
        auto panModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPanMod (FormatHelpers::getCvInput (channelProperties.getPanMod ()), FormatHelpers::getAmount (destChannelProperties.getPanMod ()), false);
        };
        auto panModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getPanMod ()};
            channelProperties.setPanMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getPanMod ()), true);
        };
        auto panModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getPanMod ()};
            channelProperties.setPanMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getPanMod ()), true);
        };
        auto editMenu { createChannelEditMenu (panModCVInputSetter, panModCVInputResetter, panModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (panModComboBox, "PanMod", [this] () { panModUiChanged (panModComboBox.getSelectedItemText (), panModTextEditor.getText ().getDoubleValue ()); });

    // PAN CV OFFSET EDITOR
    panModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getPanMod ()); };
    panModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getPanMod ()); };
    panModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getPanMod ();  };
    panModTextEditor.updateDataCallback = [this] (double amount) { panModUiChanged (FormatHelpers::getCvInput (channelProperties.getPanMod ()), amount); };
    panModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto panModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setPanMod (FormatHelpers::getCvInput (destChannelProperties.getPanMod ()), FormatHelpers::getAmount (channelProperties.getPanMod ()), false);
        };
        auto panModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getPanMod ()};
            channelProperties.setPanMod (FormatHelpers::getCvInput (channelProperties.getPanMod ()), defaultCvAmount, true);
        };
        auto panModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getPanMod ()};
            channelProperties.setPanMod (FormatHelpers::getCvInput (channelProperties.getPanMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (panModOffsetSetter, panModOffsetResetter, panModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (panModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PanMod");

    // MIX LABEL
    setupLabel (mixLevelLabel, "MIX", kMediumLabelSize, juce::Justification::centredRight);

    // MIX EDITOR
    mixLevelTextEditor.getMinValueCallback = [this] () { return minChannelProperties.getMixLevel (); };
    mixLevelTextEditor.getMaxValueCallback = [this] () { return maxChannelProperties.getMixLevel (); };
    mixLevelTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, false); };
    mixLevelTextEditor.updateDataCallback = [this] (double value) { mixLevelUiChanged (value); };
    mixLevelTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto multiplier = [this, dragSpeed] ()
        {
            if (dragSpeed == DragSpeed::slow)
                return 0.1;
            else if (dragSpeed == DragSpeed::medium)
                return 1.0;
            else
                return (maxChannelProperties.getMixLevel () - minChannelProperties.getMixLevel ()) / 10.0;
        } ();
        const auto newValue { channelProperties.getMixLevel () + (multiplier * static_cast<double> (direction)) };
        mixLevelTextEditor.setValue (newValue);
    };
    mixLevelTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setMixLevel (channelProperties.getMixLevel (), false); },
                                               [this] () { channelProperties.setMixLevel (defaultChannelProperties.getMixLevel (), true); },
                                               [this] () { channelProperties.setMixLevel (uneditedChannelProperties.getMixLevel (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (mixLevelTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MixLevel");

    // MIX MOD LABEL
    setupLabel (mixModIsFaderLabel, "Mix Mod", kMediumLabelSize, juce::Justification::centredRight);

    // MIX MOD COMBOBOX
    mixModIsFaderComboBox.addItem ("Normal", 1);
    mixModIsFaderComboBox.addItem ("Fader", 2);
    mixModIsFaderComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setMixModIsFader (std::clamp (mixModIsFaderComboBox.getSelectedItemIndex () + scrollAmount, 0, mixModIsFaderComboBox.getNumItems () - 1) == 1, true);
    };
    mixModIsFaderComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setMixModIsFader (channelProperties.getMixModIsFader (), false); },
                                               [this] () { channelProperties.setMixModIsFader (defaultChannelProperties.getMixModIsFader (), true); },
                                               [this] () { channelProperties.setMixModIsFader (uneditedChannelProperties.getMixModIsFader (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (mixModIsFaderComboBox, "MixModIsFader", [this] () { mixModIsFaderUiChanged (mixModIsFaderComboBox.getSelectedId () == 2); });

    // MIX CV INPUTCOMBOBOX
    mixModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        mixModComboBox.setSelectedItemIndex (std::clamp (mixModComboBox.getSelectedItemIndex () + scrollAmount, 0, mixModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getMixMod () };
        channelProperties.setMixMod (mixModComboBox.getSelectedItemText (), amount, false);
    };
    mixModComboBox.onPopupMenuCallback = [this] ()
    {
        auto mixModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setMixMod (FormatHelpers::getCvInput (channelProperties.getMixMod ()), FormatHelpers::getAmount (destChannelProperties.getMixMod ()), false);
        };
        auto mixModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getMixMod ()};
            channelProperties.setMixMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getMixMod ()), true);
        };
        auto mixModCVInputReverter= [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getMixMod ()};
            channelProperties.setMixMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getMixMod ()), true);
        };
        auto editMenu { createChannelEditMenu (mixModCVInputSetter, mixModCVInputResetter, mixModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (mixModComboBox, "MixMod", [this] () { mixModUiChanged (mixModComboBox.getSelectedItemText (), mixModTextEditor.getText ().getDoubleValue ()); });

    // MIX MOD CV OFFSET EDITOR
    mixModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getMixMod ()); };
    mixModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getMixMod ()); };
    mixModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getMixMod ();  };
    mixModTextEditor.updateDataCallback = [this] (double amount) { mixModUiChanged (FormatHelpers::getCvInput (channelProperties.getMixMod ()), amount); };
    mixModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto mixModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setMixMod (FormatHelpers::getCvInput (destChannelProperties.getMixMod ()), FormatHelpers::getAmount (channelProperties.getMixMod ()), false);
        };
        auto mixModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getMixMod ()};
            channelProperties.setMixMod (FormatHelpers::getCvInput (channelProperties.getMixMod ()), defaultCvAmount, true);
        };
        auto mixModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getMixMod ()};
            channelProperties.setMixMod (FormatHelpers::getCvInput (channelProperties.getMixMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (mixModOffsetSetter, mixModOffsetResetter, mixModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (mixModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MixMod");

    /////////////////////////////////////////
    // column four

    // CHANNEL MODE LABEL
    setupLabel (channelModeLabel, "MODE", kMediumLabelSize, juce::Justification::centred);

    // CHANNEL MODE COMBOBOX
    channelModeComboBox.addItem ("Master", ChannelProperties::ChannelMode::master + 1); // 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    channelModeComboBox.addItem ("Link", ChannelProperties::ChannelMode::link + 1);
    channelModeComboBox.addItem ("Stereo/Right", ChannelProperties::ChannelMode::stereoRight + 1);
    channelModeComboBox.addItem ("Cycle", ChannelProperties::ChannelMode::cycle + 1);
    channelModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setChannelMode (std::clamp (channelModeComboBox.getSelectedItemIndex () + scrollAmount, 0, channelModeComboBox.getNumItems () - 1), true);
    };
    channelModeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setChannelMode (channelProperties.getChannelMode (), false); },
                                               [this] () { channelProperties.setChannelMode (defaultChannelProperties.getChannelMode (), true); },
                                               [this] () { channelProperties.setChannelMode (uneditedChannelProperties.getChannelMode (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (channelModeComboBox, "ChannelMode", [this] ()
    {
        channelModeUiChanged (channelModeComboBox.getSelectedId () - 1);
    });

    // TRIGGER LABEL
    setupLabel (autoTriggerLabel, "TRIGGER", kMediumLabelSize, juce::Justification::centredRight);

    // TRIGGER COMBOBOX
    autoTriggerComboBox.addItem ("Normal", 1);
    autoTriggerComboBox.addItem ("Auto", 2);
    autoTriggerComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setAutoTrigger (std::clamp (autoTriggerComboBox.getSelectedItemIndex () + scrollAmount, 0, autoTriggerComboBox.getNumItems () - 1) == 1, true);
    };
    autoTriggerComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setAutoTrigger (channelProperties.getAutoTrigger (), true); },
                                               [this] () { channelProperties.setAutoTrigger (defaultChannelProperties.getAutoTrigger (), true); },
                                               [this] () { channelProperties.setAutoTrigger (uneditedChannelProperties.getAutoTrigger (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (autoTriggerComboBox, "AutoTrigger", [this] ()
    {
        const auto autoTrigger { autoTriggerComboBox.getSelectedId () == 2 };
        autoTriggerUiChanged (autoTrigger);
    });

    // PLAY MODE LABEL
    setupLabel (playModeLabel, "PLAY", kMediumLabelSize, juce::Justification::centredRight);

    // PLAY MODE COMBOBOX
    playModeComboBox.addItem ("Gated", 1); // 0 = Gated, 1 = One Shot
    playModeComboBox.addItem ("One Shot", 2);
    playModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setPlayMode (std::clamp (playModeComboBox.getSelectedItemIndex () + scrollAmount, 0, playModeComboBox.getNumItems () - 1), true);
    };
    playModeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setPlayMode (channelProperties.getPlayMode (), false); },
                                               [this] () { channelProperties.setPlayMode (defaultChannelProperties.getPlayMode (), true); },
                                               [this] () { channelProperties.setPlayMode (uneditedChannelProperties.getPlayMode (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (playModeComboBox, "PlayMode", [this] () { playModeUiChanged (playModeComboBox.getSelectedId () - 1); });

    // SAMPLE START MOD LABEL
    setupLabel (sampleStartModLabel, "SAMPLE START", kMediumLabelSize, juce::Justification::centred);

    // SAMPLE START CV INPUT COMBOBOX
    sampleStartModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        sampleStartModComboBox.setSelectedItemIndex (std::clamp (sampleStartModComboBox.getSelectedItemIndex () + scrollAmount, 0, sampleStartModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getSampleStartMod () };
        channelProperties.setSampleStartMod (sampleStartModComboBox.getSelectedItemText (), amount, false);
    };
    sampleStartModComboBox.onPopupMenuCallback = [this] ()
    {
        auto sampleStartModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setSampleStartMod (FormatHelpers::getCvInput (channelProperties.getSampleStartMod ()), FormatHelpers::getAmount (destChannelProperties.getSampleStartMod ()), false);
        };
        auto sampleStartModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getSampleStartMod ()};
            channelProperties.setSampleStartMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getSampleStartMod ()), true);
        };
        auto sampleStartModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getSampleStartMod ()};
            channelProperties.setSampleStartMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getSampleStartMod ()), true);
        };
        auto editMenu { createChannelEditMenu (sampleStartModCVInputSetter, sampleStartModCVInputResetter, sampleStartModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (sampleStartModComboBox, "SampleStartMod", [this] () { sampleStartModUiChanged (sampleStartModComboBox.getSelectedItemText (), sampleStartModTextEditor.getText ().getDoubleValue ()); });

    // SAMPLE START CV OFFSET EDITOR
    sampleStartModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getSampleStartMod ()); };
    sampleStartModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getSampleStartMod ()); };
    sampleStartModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getSampleStartMod ();  };
    sampleStartModTextEditor.updateDataCallback = [this] (double amount) { sampleStartModUiChanged (FormatHelpers::getCvInput (channelProperties.getSampleStartMod ()), amount); };
    sampleStartModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto sampleStartModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setSampleStartMod (FormatHelpers::getCvInput (destChannelProperties.getSampleStartMod ()), FormatHelpers::getAmount (channelProperties.getSampleStartMod ()), false);
        };
        auto sampleStartModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getSampleStartMod ()};
            channelProperties.setSampleStartMod (FormatHelpers::getCvInput (channelProperties.getSampleStartMod ()), defaultCvAmount, true);
        };
        auto sampleStartModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getSampleStartMod ()};
            channelProperties.setSampleStartMod (FormatHelpers::getCvInput (channelProperties.getSampleStartMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (sampleStartModOffsetSetter, sampleStartModOffsetResetter, sampleStartModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (sampleStartModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "SampleStartMod");

    // SAMPLE END MOD LABEL
    setupLabel (sampleEndModLabel, "SAMPLE END", kMediumLabelSize, juce::Justification::centred);

    // SAMPLE END CV INPUT COMBOBOX
    sampleEndModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        sampleEndModComboBox.setSelectedItemIndex (std::clamp (sampleEndModComboBox.getSelectedItemIndex () + scrollAmount, 0, sampleEndModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getSampleEndMod () };
        channelProperties.setSampleEndMod (sampleEndModComboBox.getSelectedItemText (), amount, false);
    };
    sampleEndModComboBox.onPopupMenuCallback = [this] ()
    {
        auto sampleEndModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setSampleEndMod (FormatHelpers::getCvInput (channelProperties.getSampleEndMod ()), FormatHelpers::getAmount (destChannelProperties.getSampleEndMod ()), false);
        };
        auto sampleEndModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getSampleEndMod ()};
            channelProperties.setSampleEndMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getSampleEndMod ()), true);
        };
        auto sampleEndModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getSampleEndMod ()};
            channelProperties.setSampleEndMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getSampleEndMod ()), true);
        };
        auto editMenu { createChannelEditMenu (sampleEndModCVInputSetter, sampleEndModCVInputResetter, sampleEndModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (sampleEndModComboBox, "SampleEndMod", [this] () { sampleEndModUiChanged (sampleEndModComboBox.getSelectedItemText (), sampleEndModTextEditor.getText ().getDoubleValue ()); });

    // SAMPLE END CV OFFSET EDITOR
    sampleEndModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getSampleEndMod ()); };
    sampleEndModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getSampleEndMod ()); };
    sampleEndModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getSampleEndMod ();  };
    sampleEndModTextEditor.updateDataCallback = [this] (double amount) { sampleEndModUiChanged (FormatHelpers::getCvInput (channelProperties.getSampleEndMod ()), amount); };
    sampleEndModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto sampleEndModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setSampleEndMod (FormatHelpers::getCvInput (destChannelProperties.getSampleEndMod ()), FormatHelpers::getAmount (channelProperties.getSampleEndMod ()), false);
        };
        auto sampleEndModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getSampleEndMod ()};
            channelProperties.setSampleEndMod (FormatHelpers::getCvInput (channelProperties.getSampleEndMod ()), defaultCvAmount, true);
        };
        auto sampleEndModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getSampleEndMod ()};
            channelProperties.setSampleEndMod (FormatHelpers::getCvInput (channelProperties.getSampleEndMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (sampleEndModOffsetSetter, sampleEndModOffsetResetter, sampleEndModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (sampleEndModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "SampleEndMod");

    // LOOP MODE LABEL
    setupLabel (loopModeLabel, "LOOP", kMediumLabelSize, juce::Justification::centredRight);

    // LOOP MODE COMBOBOX
    loopModeComboBox.addItem ("No Loop", 1); // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    loopModeComboBox.addItem ("Loop", 2);
    loopModeComboBox.addItem ("Loop/Release", 3);
    loopModeComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setLoopMode (std::clamp (loopModeComboBox.getSelectedItemIndex () + scrollAmount, 0, loopModeComboBox.getNumItems () - 1), true);
    };
    loopModeComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setLoopMode (channelProperties.getLoopMode (), false); },
                                               [this] () { channelProperties.setLoopMode (defaultChannelProperties.getLoopMode (), true); },
                                               [this] () { channelProperties.setLoopMode (uneditedChannelProperties.getLoopMode (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (loopModeComboBox, "LoopMode", [this] () { loopModeUiChanged (loopModeComboBox.getSelectedId () - 1); });

    // LOOP START MOD LABEL
    setupLabel (loopStartModLabel, "LOOP START", kMediumLabelSize, juce::Justification::centred);

    // LOOP START CV INPUT COMBOBOX
    loopStartModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        loopStartModComboBox.setSelectedItemIndex (std::clamp (loopStartModComboBox.getSelectedItemIndex () + scrollAmount, 0, loopStartModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLoopStartMod () };
        channelProperties.setLoopStartMod (loopStartModComboBox.getSelectedItemText (), amount, false);
    };
    loopStartModComboBox.onPopupMenuCallback = [this] ()
    {
        auto loopStartModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLoopStartMod (FormatHelpers::getCvInput (channelProperties.getLoopStartMod ()), FormatHelpers::getAmount (destChannelProperties.getLoopStartMod ()), false);
        };
        auto loopStartModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getLoopStartMod ()};
            channelProperties.setLoopStartMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getLoopStartMod ()), true);
        };
        auto loopStartModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getLoopStartMod ()};
            channelProperties.setLoopStartMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getLoopStartMod ()), true);
        };
        auto editMenu { createChannelEditMenu (loopStartModCVInputSetter, loopStartModCVInputResetter, loopStartModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (loopStartModComboBox, "LoopStartMod", [this] () { loopStartModUiChanged (loopStartModComboBox.getSelectedItemText (), loopStartModTextEditor.getText ().getDoubleValue ()); });

    // LOOP START CV OFFSET EDITOR
    loopStartModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLoopStartMod ()); };
    loopStartModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLoopStartMod ()); };
    loopStartModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLoopStartMod ();  };
    loopStartModTextEditor.updateDataCallback = [this] (double amount) { loopStartModUiChanged (FormatHelpers::getCvInput (channelProperties.getLoopStartMod ()), amount); };
    loopStartModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto loopStartModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLoopStartMod (FormatHelpers::getCvInput (destChannelProperties.getLoopStartMod ()), FormatHelpers::getAmount (channelProperties.getLoopStartMod ()), false);
        };
        auto loopStartModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getLoopStartMod ()};
            channelProperties.setLoopStartMod (FormatHelpers::getCvInput (channelProperties.getLoopStartMod ()), defaultCvAmount, true);
        };
        auto loopStartModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getLoopStartMod ()};
            channelProperties.setLoopStartMod (FormatHelpers::getCvInput (channelProperties.getLoopStartMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (loopStartModOffsetSetter, loopStartModOffsetResetter, loopStartModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (loopStartModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LoopStartMod");

    // LOOP END MOD LABEL
    setupLabel (loopLengthModLabel, "LOOP LENGTH", kMediumLabelSize, juce::Justification::centred);

    // LOOP END CV INPUT COMBOBOX
    loopLengthModComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        loopLengthModComboBox.setSelectedItemIndex (std::clamp (loopLengthModComboBox.getSelectedItemIndex () + scrollAmount, 0, loopLengthModComboBox.getNumItems () - 1));
        auto [_, amount] { channelProperties.getLoopLengthMod () };
        channelProperties.setLoopLengthMod (loopLengthModComboBox.getSelectedItemText (), amount, false);
    };
    loopLengthModComboBox.onPopupMenuCallback = [this] ()
    {
        auto loopLengthModCVInputSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLoopLengthMod (FormatHelpers::getCvInput (channelProperties.getLoopLengthMod ()), FormatHelpers::getAmount (destChannelProperties.getLoopLengthMod ()), false);
        };
        auto loopLengthModCVInputResetter = [this] ()
        {
            const auto [defaultCvInput, _] { defaultChannelProperties.getLoopLengthMod ()};
            channelProperties.setLoopLengthMod (defaultCvInput, FormatHelpers::getAmount (channelProperties.getLoopLengthMod ()), true);
        };
        auto loopLengthModCVInputReverter = [this] ()
        {
            const auto [uneditedCvInput, _] { uneditedChannelProperties.getLoopLengthMod ()};
            channelProperties.setLoopLengthMod (uneditedCvInput, FormatHelpers::getAmount (channelProperties.getLoopLengthMod ()), true);
        };
        auto editMenu { createChannelEditMenu (loopLengthModCVInputSetter, loopLengthModCVInputResetter, loopLengthModCVInputReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (loopLengthModComboBox, "LoopLengthMod", [this] () { loopLengthModUiChanged (loopLengthModComboBox.getSelectedItemText (), loopLengthModTextEditor.getText ().getDoubleValue ()); });

    // LOOP END CV OFFSET EDITOR
    loopLengthModTextEditor.getMinValueCallback = [this] () { return FormatHelpers::getAmount (minChannelProperties.getLoopLengthMod ()); };
    loopLengthModTextEditor.getMaxValueCallback = [this] () { return FormatHelpers::getAmount (maxChannelProperties.getLoopLengthMod ()); };
    loopLengthModTextEditor.getCvInputAndAmount = [this] () { return channelProperties.getLoopLengthMod ();  };
    loopLengthModTextEditor.updateDataCallback = [this] (double amount) { loopLengthModUiChanged (FormatHelpers::getCvInput (channelProperties.getLoopLengthMod ()), amount); };
    loopLengthModTextEditor.onPopupMenuCallback = [this] ()
    {
        auto loopLengthModOffsetSetter = [this] (ChannelProperties& destChannelProperties)
        {
            destChannelProperties.setLoopLengthMod (FormatHelpers::getCvInput (destChannelProperties.getLoopLengthMod ()), FormatHelpers::getAmount (channelProperties.getLoopLengthMod ()), false);
        };
        auto loopLengthModOffsetResetter = [this] ()
        {
            const auto [_, defaultCvAmount] { defaultChannelProperties.getLoopLengthMod ()};
            channelProperties.setLoopLengthMod (FormatHelpers::getCvInput (channelProperties.getLoopLengthMod ()), defaultCvAmount, true);
        };
        auto loopLengthModOffsetReverter = [this] ()
        {
            const auto [_, uneditedCvAmount] { uneditedChannelProperties.getLoopLengthMod ()};
            channelProperties.setLoopLengthMod (FormatHelpers::getCvInput (channelProperties.getLoopLengthMod ()), uneditedCvAmount, true);
        };
        auto editMenu { createChannelEditMenu (loopLengthModOffsetSetter, loopLengthModOffsetResetter, loopLengthModOffsetReverter) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (loopLengthModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LoopLengthMod");

    // CROSSFADE GROUP LABEL
    setupLabel (xfadeGroupLabel, "XFADE GRP", kSmallLabelSize, juce::Justification::centredRight);

    // CROSSFADE GROUP COMBOBOX
    xfadeGroupComboBox.addItem ("None", 1); // Off, A, B, C, D
    xfadeGroupComboBox.addItem ("A", 2);
    xfadeGroupComboBox.addItem ("B", 3);
    xfadeGroupComboBox.addItem ("C", 4);
    xfadeGroupComboBox.addItem ("D", 5);
    xfadeGroupComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        xfadeGroupComboBox.setSelectedItemIndex (std::clamp (xfadeGroupComboBox.getSelectedItemIndex () + scrollAmount, 0, xfadeGroupComboBox.getNumItems () - 1));
        channelProperties.setXfadeGroup (xfadeGroupComboBox.getText () ,false);
    };
    xfadeGroupComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setXfadeGroup (channelProperties.getXfadeGroup (), false); },
                                               [this] () { channelProperties.setXfadeGroup (defaultChannelProperties.getXfadeGroup (), true); },
                                               [this] () { channelProperties.setXfadeGroup (uneditedChannelProperties.getXfadeGroup (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (xfadeGroupComboBox, "XfadeGroup", [this] () { xfadeGroupUiChanged (xfadeGroupComboBox.getText ()); });

    /////////////////////////////////////////
    // column five above the Zones tab component

    // CV ZONE SELECT LABEL
    setupLabel (zonesCVLabel, "CV", kMediumLabelSize, juce::Justification::centredRight);

    // CV ZONE SELECT CV INPUT COMBOBOX
    zonesCVComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        const auto newCvInputComboBoxIndex { zonesCVComboBox.getSelectedItemIndex () + scrollAmount};
        zonesCVComboBox.setSelectedItemIndex (std::clamp (newCvInputComboBoxIndex, 0, zonesCVComboBox.getNumItems () - 1));
        channelProperties.setZonesCV (zonesCVComboBox.getSelectedItemText (), false);
    };
    zonesCVComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setZonesCV (channelProperties.getZonesCV (), false); },
                                               [this] () { channelProperties.setZonesCV (defaultChannelProperties.getZonesCV (), true); },
                                               [this] () { channelProperties.setZonesCV (uneditedChannelProperties.getZonesCV (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupCvInputComboBox (zonesCVComboBox, "ZonesCV", [this] () { zonesCVUiChanged (zonesCVComboBox.getSelectedItemText ()); });

    // ZONE SELECT MODE LABEL
    setupLabel (zonesRTLabel, "SELECT", kSmallLabelSize, juce::Justification::centredRight);

    // ZONE SELECT MODE COMBOBOX
    zonesRTComboBox.addItem ("Gate Rise", 1); // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
    zonesRTComboBox.addItem ("Continuous", 2);
    zonesRTComboBox.addItem ("Advance", 3);
    zonesRTComboBox.addItem ("Random", 4);
    zonesRTComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setZonesRT (std::clamp (zonesRTComboBox.getSelectedItemIndex () + scrollAmount, 0, zonesRTComboBox.getNumItems () - 1), true);
    };
    zonesRTComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setZonesRT (channelProperties.getZonesRT (), false); },
                                               [this] () { channelProperties.setZonesRT (defaultChannelProperties.getZonesRT (), true); },
                                               [this] () { channelProperties.setZonesRT (uneditedChannelProperties.getZonesRT (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (zonesRTComboBox, "ZonesRT", [this] () { zonesRTUiChanged (zonesRTComboBox.getSelectedId () - 1); });

    // LOOP LENGTH/END TOGGLE LABEL
    setupLabel (loopLengthIsEndLabel, "LENGTH/END", kSmallLabelIntSize, juce::Justification::centredRight);

    // LOOP LENGTH/END TOGGLE COMBOBOX
    loopLengthIsEndComboBox.addItem ("Length", 1); // 0 = Length, 1 = End
    loopLengthIsEndComboBox.addItem ("End", 2);
    loopLengthIsEndComboBox.onDragCallback = [this] (DragSpeed dragSpeed, int direction)
    {
        const auto scrollAmount { (dragSpeed == DragSpeed::fast ? 2 : 1) * direction };
        channelProperties.setLoopLengthIsEnd (std::clamp (loopLengthIsEndComboBox.getSelectedItemIndex () + scrollAmount, 0, loopLengthIsEndComboBox.getNumItems () - 1), true);
    };
    loopLengthIsEndComboBox.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createChannelEditMenu ([this] (ChannelProperties& destChannelProperties) { destChannelProperties.setLoopLengthIsEnd (channelProperties.getLoopLengthIsEnd (), false); },
                                               [this] () { channelProperties.setLoopLengthIsEnd (defaultChannelProperties.getLoopLengthIsEnd (), true); },
                                               [this] () { channelProperties.setLoopLengthIsEnd (uneditedChannelProperties.getLoopLengthIsEnd (), true); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupComboBox (loopLengthIsEndComboBox, "LoopLengthIsEnd", [this] ()
    {
        const auto loopLengthIsEnd { loopLengthIsEndComboBox.getSelectedId () == 2 };
        loopLengthIsEndUiChanged (loopLengthIsEnd);
        // inform all the zones of the change
        for (auto& zoneEditor : zoneEditors)
            zoneEditor.setLoopLengthIsEnd (loopLengthIsEnd);
    });
}

// TODO - move this to the EditManger
void ChannelEditor::balanceVoltages (VoltageBalanceType balanceType)
{
    const auto numUsedZones { editManager->getNumUsedZones (channelIndex) };
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

void ChannelEditor::init (juce::ValueTree channelPropertiesVT, juce::ValueTree uneditedChannelPropertiesVT, juce::ValueTree rootPropertiesVT,
                          juce::ValueTree copyBufferZonePropertiesVT, bool* theZoneCopyBufferHasData)
{
    //DebugLog ("ChannelEditor["+ juce::String (channelProperties.getId ()) + "]", "init");
    jassert (theZoneCopyBufferHasData != nullptr);
    zoneCopyBufferHasData = theZoneCopyBufferHasData;

    copyBufferZoneProperties.wrap (copyBufferZonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::no);

    SystemServices systemServices {runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes};
    editManager = systemServices.getEditManager ();

    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    uneditedChannelProperties.wrap (uneditedChannelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    channelIndex = channelProperties.getId () - 1;
    setupChannelPropertiesCallbacks ();
    sampleWaveformDisplay.init (channelPropertiesVT, rootPropertiesVT);

    channelProperties.forEachZone ([this, rootPropertiesVT] (juce::ValueTree zonePropertiesVT, int zoneIndex)
    {
        // Zone Editor setup
        auto& zoneEditor { zoneEditors [zoneIndex] };
        zoneEditor.init (zonePropertiesVT, uneditedChannelProperties.getZoneVT (zoneIndex), rootPropertiesVT);
        zoneEditor.displayToolsMenu = [this] (int zoneIndex)
        {
            auto* popupMenuLnF { new juce::LookAndFeel_V4 };
            popupMenuLnF->setColour (juce::PopupMenu::ColourIds::headerTextColourId, juce::Colours::white.withAlpha (0.3f));
            juce::PopupMenu toolsMenu;
            toolsMenu.setLookAndFeel (popupMenuLnF);
            toolsMenu.addSectionHeader ("Zone " + juce::String (zoneProperties [zoneIndex].getId ()));
            toolsMenu.addSeparator ();

            {
                juce::PopupMenu balanceMenu;
                balanceMenu.addItem ("5V", true, false, [this] () { balanceVoltages (VoltageBalanceType::distributeAcross5V); });
                balanceMenu.addItem ("10V", true, false, [this] () { balanceVoltages (VoltageBalanceType::distributeAcross10V); });
                balanceMenu.addItem ("Kbd", true, false, [this] () { balanceVoltages (VoltageBalanceType::distribute1vPerOct); });
                balanceMenu.addItem ("Maj", true, false, [this] () { balanceVoltages (VoltageBalanceType::distribute1vPerOctMajor); });
                toolsMenu.addSubMenu ("Balance", balanceMenu, zoneProperties [zoneIndex].getSample ().isNotEmpty ());
            }
            {
                juce::PopupMenu editMenu;
                {
                    juce::PopupMenu copyMenu;
                    copyMenu.addItem ("Settings", zoneProperties [zoneIndex].getSample ().isNotEmpty (), false, [this, zoneIndex] () { copyZone (zoneIndex, true); });
                    copyMenu.addItem ("Sample and Settings", zoneProperties [zoneIndex].getSample ().isNotEmpty (), false, [this, zoneIndex] () { copyZone (zoneIndex, false); });
                    editMenu.addSubMenu ("Copy", copyMenu, zoneProperties [zoneIndex].getSample ().isNotEmpty ());
                }
                editMenu.addItem ("Paste", *zoneCopyBufferHasData && (zoneProperties [zoneIndex].getSample ().isNotEmpty () || copyBufferZoneProperties.getSample ().isNotEmpty ()), false, [this, zoneIndex] ()
                {
                    pasteZone (zoneIndex);
                    updateAllZoneTabNames ();
                    ensureProperZoneIsSelected ();
                });
                editMenu.addItem ("Insert", zoneProperties [zoneIndex].getSample ().isNotEmpty () && zoneIndex > 0 && zoneIndex < zoneProperties.size () - 1, false, [this, zoneIndex] ()
                {
                    duplicateZone (zoneIndex);
                    ensureProperZoneIsSelected ();
                    updateAllZoneTabNames ();
                });
                {
                    juce::PopupMenu deleteMenu;
                    deleteMenu.addItem ("Zone " + juce::String (zoneProperties [zoneIndex].getId ()), zoneProperties [zoneIndex].getSample ().isNotEmpty (), false, [this, zoneIndex] ()
                    {
                        deleteZone (zoneIndex);
                        ensureProperZoneIsSelected ();
                        updateAllZoneTabNames ();
                    });
                    deleteMenu.addItem ("All", editManager->getNumUsedZones (channelIndex) > 0, false, [this] ()
                    {
                        clearAllZones ();
                        ensureProperZoneIsSelected ();
                        updateAllZoneTabNames ();
                    });
                    editMenu.addSubMenu ("Delete", deleteMenu, zoneProperties [zoneIndex].getSample ().isNotEmpty ());
                }
                toolsMenu.addSubMenu ("Edit", editMenu, true);
            }
            {
                juce::PopupMenu explodeMenu;
                for (auto explodeCount { 2 }; explodeCount < 9 - zoneIndex; ++explodeCount)
                    explodeMenu.addItem (juce::String (explodeCount) + " zones", true, false, [this, zoneIndex, explodeCount] ()
                    {
                        explodeZone (zoneIndex, explodeCount);
                    });
                toolsMenu.addSubMenu ("Explode", explodeMenu, zoneIndex < 7);
            }
            {
                juce::PopupMenu flipMenu;
                const auto maxFlipCount { editManager->getNumUsedZones (channelIndex) + 1 - zoneIndex };
                for (auto flipCount { 2 }; flipCount < maxFlipCount; ++flipCount)
                    flipMenu.addItem (juce::String (flipCount) + " zones", true, false, [this, zoneIndex, flipCount] ()
                    {
                        flipZones (zoneIndex, flipCount);
                    });
                toolsMenu.addSubMenu ("Flip", flipMenu, zoneIndex < 7);
            }

            toolsMenu.showMenuAsync ({}, [this, popupMenuLnF] (int) { delete popupMenuLnF; });
        };

        // Zone Properties setup
        auto& curZoneProperties { zoneProperties [zoneIndex] };
        curZoneProperties.wrap (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
        curZoneProperties.onSampleChange = [this] (juce::String sample)
        {
            // TODO - optimize so we only update the zone that is changing
            ensureProperZoneIsSelected ();
            updateAllZoneTabNames ();
        };
        curZoneProperties.onMinVoltageChange = [this] ([[maybe_unused]] double minVoltage)
        {
            updateAllZoneTabNames ();
        };
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
    checkStereoRightOverlay ();
    if (isVisible ())
        configAudioPlayer ();
}

// TODO - can we move this to the EditManager, as it eventually just calls editManager->assignSamples (parentChannelIndex, startingZoneIndex, files); in the ZoneEditor
void ChannelEditor::receiveSampleLoadRequest (juce::File sampleFile)
{
    const auto zoneIndex { zoneTabs.getCurrentTabIndex () };
    auto curZoneEditor { dynamic_cast<ZoneEditor*> (zoneTabs.getTabContentComponent (zoneIndex)) };
    curZoneEditor->receiveSampleLoadRequest (sampleFile);
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
    const bool isStereoRightMode { channelProperties.getChannelMode () == ChannelProperties::ChannelMode::stereoRight };
    stereoRightTransparantOverly.setVisible (isStereoRightMode);

    aliasingTextEditor.setEnabled (! isStereoRightMode);
    aliasingModComboBox.setEnabled (! isStereoRightMode);
    aliasingModTextEditor.setEnabled (! isStereoRightMode);
    attackTextEditor.setEnabled (! isStereoRightMode);
    attackFromCurrentComboBox.setEnabled (! isStereoRightMode);
    attackModComboBox.setEnabled (! isStereoRightMode);
    attackModTextEditor.setEnabled (! isStereoRightMode);
    autoTriggerComboBox.setEnabled (! isStereoRightMode);
    bitsTextEditor.setEnabled (! isStereoRightMode);
    bitsModComboBox.setEnabled (! isStereoRightMode);
    bitsModTextEditor.setEnabled (! isStereoRightMode);
    //channelModeComboBox.setEnabled (! isStereoRightMode); // this is still editable in stereo/right mode
    expAMComboBox.setEnabled (! isStereoRightMode);
    expAMTextEditor.setEnabled (! isStereoRightMode);
    expFMComboBox.setEnabled (! isStereoRightMode);
    expFMTextEditor.setEnabled (! isStereoRightMode);
    levelTextEditor.setEnabled (! isStereoRightMode);
    linAMComboBox.setEnabled (! isStereoRightMode);
    linAMTextEditor.setEnabled (! isStereoRightMode);
    linAMisExtEnvComboBox.setEnabled (! isStereoRightMode);
    linFMComboBox.setEnabled (! isStereoRightMode);
    linFMTextEditor.setEnabled (! isStereoRightMode);
    loopLengthIsEndComboBox.setEnabled (! isStereoRightMode);
    loopLengthModComboBox.setEnabled (! isStereoRightMode);
    loopLengthModTextEditor.setEnabled (! isStereoRightMode);
    loopModeComboBox.setEnabled (! isStereoRightMode);
    loopStartModComboBox.setEnabled (! isStereoRightMode);
    loopStartModTextEditor.setEnabled (! isStereoRightMode);
    mixLevelTextEditor.setEnabled (! isStereoRightMode);
    mixModComboBox.setEnabled (! isStereoRightMode);
    mixModTextEditor.setEnabled (! isStereoRightMode);
    mixModIsFaderComboBox.setEnabled (! isStereoRightMode);
    panTextEditor.setEnabled (! isStereoRightMode);
    panModComboBox.setEnabled (! isStereoRightMode);
    panModTextEditor.setEnabled (! isStereoRightMode);
    phaseCVComboBox.setEnabled (! isStereoRightMode);
    phaseCVTextEditor.setEnabled (! isStereoRightMode);
    pitchTextEditor.setEnabled (! isStereoRightMode);
    pitchCVComboBox.setEnabled (! isStereoRightMode);
    pitchCVTextEditor.setEnabled (! isStereoRightMode);
    playModeComboBox.setEnabled (! isStereoRightMode);
    pMIndexTextEditor.setEnabled (! isStereoRightMode);
    pMIndexModComboBox.setEnabled (! isStereoRightMode);
    pMIndexModTextEditor.setEnabled (! isStereoRightMode);
    pMSourceComboBox.setEnabled (! isStereoRightMode);
    releaseTextEditor.setEnabled (! isStereoRightMode);
    releaseModComboBox.setEnabled (! isStereoRightMode);
    releaseModTextEditor.setEnabled (! isStereoRightMode);
    reverseButton.setEnabled (! isStereoRightMode);
    sampleEndModComboBox.setEnabled (! isStereoRightMode);
    sampleEndModTextEditor.setEnabled (! isStereoRightMode);
    sampleStartModComboBox.setEnabled (! isStereoRightMode);
    sampleStartModTextEditor.setEnabled (! isStereoRightMode);
    spliceSmoothingButton.setEnabled (! isStereoRightMode);
    xfadeGroupComboBox.setEnabled (! isStereoRightMode);
    zonesCVComboBox.setEnabled (! isStereoRightMode);
    zonesRTComboBox.setEnabled (! isStereoRightMode);
    arEnvelopeComponent.setEnabled (! isStereoRightMode);
    sampleWaveformDisplay.setEnabled (! isStereoRightMode);
    toolsButton.setEnabled (! isStereoRightMode);
    for (auto zoneIndex { 0 }; zoneIndex < 8; ++zoneIndex)
        dynamic_cast<ZoneEditor*> (zoneTabs.getTabContentComponent (zoneIndex))->setStereoRightChannelMode (isStereoRightMode);
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

    // Envelope Graphic Editor
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

    jassert (displayToolsMenu != nullptr);
    toolsButton.setBounds (5, getHeight () - 5 - 20, 40, 20);

    // layout the Zones section. ie. the tabs and the channel level controls
    auto zoneColumn {getLocalBounds ().removeFromRight (213)};
    zoneColumn.removeFromTop (3);
    auto zoneTopSection { zoneColumn.removeFromTop (75).withTrimmedBottom (5).withTrimmedRight (3)};
    zonesLabel.setBounds (zoneTopSection.getX () + 15, zoneTopSection.getHeight () / 2 - kMediumLabelIntSize / 2, 80, kMediumLabelIntSize);
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

    // TODO - improve size calculation
    // Waveform Display
    sampleWaveformDisplay.setBounds (mixModComboBox.getX (), xfadeGroupComboBox.getBounds ().getBottom () + kInterControlYOffset + 5,
                                     zoneTabs.getX () - mixModComboBox.getX () - 15, getHeight () - xfadeGroupComboBox.getBounds ().getBottom () - kInterControlYOffset - 15);
}

void ChannelEditor::updateWaveformDisplay ()
{
    const auto currentZoneIndex { zoneTabs.getCurrentTabIndex () };
    sampleWaveformDisplay.setZone (currentZoneIndex);
}

void ChannelEditor::flipZones (int zoneIndex, int flipCount)
{
    ZoneProperties tempZoneProperties;
    for (auto zoneCount { 0 }; zoneCount< flipCount / 2; ++zoneCount)
    {
        auto& firstZone { zoneProperties [zoneIndex + zoneCount] };
        const auto firstZoneMinVoltage { firstZone.getMinVoltage() };
        auto& secondZone { zoneProperties [zoneIndex + (flipCount - zoneCount - 1)] };
        const auto secondZoneMinVoltage { secondZone.getMinVoltage () };
        tempZoneProperties.copyFrom (secondZone.getValueTree (), false);
        secondZone.copyFrom (firstZone.getValueTree (), false);
        secondZone.setMinVoltage (secondZoneMinVoltage, true);
        firstZone.copyFrom (tempZoneProperties.getValueTree (), false);
        firstZone.setMinVoltage (firstZoneMinVoltage, true);
    }
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
        const auto minVoltage { zoneProperties [zoneIndex].getMinVoltage () };
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
    LogDataAndUiChanges (juce::String (channelProperties.getId ()));
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
