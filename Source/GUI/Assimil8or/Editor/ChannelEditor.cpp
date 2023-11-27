#include "ChannelEditor.h"
#include "FormatHelpers.h"
#include "ParameterToolTipData.h"
#include "../../../Assimil8or/Preset/PresetHelpers.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

EditManager editManager;

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
    // TODO - these are copies of what is in ChannelEditor::setupChannelComponents, need to DRY
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
    zoneMaxVoltage.setColour (juce::Label::ColourIds::textColourId, juce::Colours::lightgrey.darker (0.2f));

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
    arEnvelopeProperties.onAttackPercentChanged= [this] (double attackPercent)
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

void ChannelEditor::duplicateZone (int zoneIndex)
{
    // if the list is full, we can't copy the end anywhere, so we'll start with the one before the end, otherwise start at the end
    auto startingZoneIndex { getNumUsedZones () - (getNumUsedZones () == zoneProperties.size () ? 2 : 1) };
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
    auto scalerValue = [rawValue] ()
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
    return static_cast<double> (static_cast<uint32_t> (rawValue * scalerValue)) / scalerValue;
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
    return static_cast<double> (static_cast<uint32_t> (rawValue * scalerValue)) / scalerValue;
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
                                                          juce::String parameterName, std::function<void ()> validateCallback, std::function<void (juce::String)> doneEditingCallback)
    {
        jassert (doneEditingCallback != nullptr);
        textEditor.setJustification (justification);
        textEditor.setIndents (1, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.setTooltip (parameterToolTipData.getToolTip ("Channel", parameterName));
        textEditor.onFocusLost = [this, &textEditor, doneEditingCallback] () { doneEditingCallback (textEditor.getText ()); };
        textEditor.onReturnKey = [this, &textEditor, doneEditingCallback] () { doneEditingCallback (textEditor.getText ()); };
        if (validateCallback != nullptr)
            textEditor.onTextChange = [this, validateCallback] () { validateCallback (); };
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
    // PITCH
    setupLabel (pitchLabel, "PITCH", kLargeLabelSize, juce::Justification::centred);
    setupTextEditor (pitchTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Pitch", [this] ()
    {
        FormatHelpers::setColorIfError (pitchTextEditor, minChannelProperties.getPitch (), maxChannelProperties.getPitch ());
    },
    [this] (juce::String text)
    {
        const auto pitch { std::clamp (text.getDoubleValue (), minChannelProperties.getPitch (), maxChannelProperties.getPitch ()) };
        pitchUiChanged (pitch);
        pitchTextEditor.setText (FormatHelpers::formatDouble (pitch, 2, true));
    });
    pitchInputControl.onDrag = [this] (int dragSpeed)
    {
        const auto incAmount { 0.1 * dragSpeed };
        const auto newAmount { channelProperties.getPitch () + incAmount };
        auto pitch { std::clamp (newAmount, minChannelProperties.getPitch (), maxChannelProperties.getPitch ()) };
        channelProperties.setPitch (pitch, true);
    };
    pitchInputControl.onPopupMenu = [this] ()
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
        pm.addSubMenu ("Special", special, true);
        pm.showMenuAsync ({}, [this] (int) {});
    };

    //
    setupLabel (pitchSemiLabel, "SEMI", kSmallLabelSize, juce::Justification::centredLeft);
    setupCvInputComboBox (pitchCVComboBox, "PitchCV", [this] () { pitchCVUiChanged (pitchCVComboBox.getSelectedItemText (), pitchCVTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (pitchCVTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PitchCV", [this] ()
    {
        FormatHelpers::setColorIfError (pitchCVTextEditor, FormatHelpers::getAmount (minChannelProperties.getPitchCV ()), FormatHelpers::getAmount (maxChannelProperties.getPitchCV ()));
    },
    [this] (juce::String text)
    {
        const auto pitchCV { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getPitchCV ()),
                                                                 FormatHelpers::getAmount (maxChannelProperties.getPitchCV ())) };
        pitchCVUiChanged (pitchCVComboBox.getSelectedItemText (), pitchCV);
        pitchCVTextEditor.setText (FormatHelpers::formatDouble (pitchCV, 2, true));
    });

    // LINFM
    setupLabel (linFMLabel, "LIN FM", kLargeLabelSize, juce::Justification::centred);
    setupCvInputComboBox (linFMComboBox, "LinFM", [this] () { linFMUiChanged (linFMComboBox.getSelectedItemText (), linFMTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (linFMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LinFM", [this] ()
    {
        FormatHelpers::setColorIfError (linFMTextEditor, FormatHelpers::getAmount (minChannelProperties.getLinFM ()), FormatHelpers::getAmount (maxChannelProperties.getLinFM ()));
    },
    [this] (juce::String text)
    {
        const auto linFM { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getLinFM ()),
                                                               FormatHelpers::getAmount (maxChannelProperties.getLinFM ())) };
        linFMUiChanged (linFMComboBox.getSelectedItemText (), linFM);
        linFMTextEditor.setText (FormatHelpers::formatDouble (linFM, 2, true));
    });

    // EXPFM
    setupLabel (expFMLabel, "EXP FM", kLargeLabelSize, juce::Justification::centred);
    setupCvInputComboBox (expFMComboBox, "ExpFM", [this] () { expFMUiChanged (expFMComboBox.getSelectedItemText (), expFMTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (expFMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ExpFM", [this] ()
    {
        FormatHelpers::setColorIfError (expFMTextEditor, FormatHelpers::getAmount (minChannelProperties.getExpFM ()), FormatHelpers::getAmount (maxChannelProperties.getExpFM ()));
    },
    [this] (juce::String text)
    {
        const auto expFM { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getExpFM ()),
                                                               FormatHelpers::getAmount (maxChannelProperties.getExpFM ())) };
        expFMUiChanged (expFMComboBox.getSelectedItemText (), expFM);
        expFMTextEditor.setText (FormatHelpers::formatDouble (expFM, 2, true));
    });

    // LEVEL
    setupLabel (levelLabel, "LEVEL", kLargeLabelSize, juce::Justification::centred);
    setupTextEditor (levelTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Level", [this] ()
    {
        FormatHelpers::setColorIfError (levelTextEditor, minChannelProperties.getLevel (), maxChannelProperties.getLevel ());
    },
    [this] (juce::String text)
    {
        const auto level { std::clamp (text.getDoubleValue (), minChannelProperties.getLevel (), maxChannelProperties.getLevel ()) };
        levelUiChanged (level);
        levelTextEditor.setText (FormatHelpers::formatDouble (level, 1, false));
    });
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
    setupCvInputComboBox (linAMComboBox, "LinAM", [this] () { linAMUiChanged (linAMComboBox.getSelectedItemText (), linAMTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (linAMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LinAM", [this] ()
    {
        FormatHelpers::setColorIfError (linAMTextEditor, FormatHelpers::getAmount (minChannelProperties.getLinAM ()), FormatHelpers::getAmount (maxChannelProperties.getLinAM ()));
    },
    [this] (juce::String text)
    {
        const auto linAM { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getLinAM ()),
                                                               FormatHelpers::getAmount (maxChannelProperties.getLinAM ())) };
        linAMUiChanged (linAMComboBox.getSelectedItemText (), linAM);
        linAMTextEditor.setText (FormatHelpers::formatDouble (linAM, 2, true));
    });
    // Linear AM Bias

    // EXPAM
    setupLabel (expAMLabel, "EXP AM", kLargeLabelSize, juce::Justification::centred);
    setupCvInputComboBox (expAMComboBox, "ExpAM", [this] () { expAMUiChanged (expAMComboBox.getSelectedItemText (), expAMTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (expAMTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ExpAM", [this] ()
    {
            FormatHelpers::setColorIfError (expAMTextEditor, FormatHelpers::getAmount (minChannelProperties.getExpAM ()), FormatHelpers::getAmount (maxChannelProperties.getExpAM ()));
    },
    [this] (juce::String text)
    {
        const auto expAM { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getExpAM ()),
                                                               FormatHelpers::getAmount (maxChannelProperties.getExpAM ())) };
        expAMUiChanged (expAMComboBox.getSelectedItemText (), expAM);
        expAMTextEditor.setText (FormatHelpers::formatDouble (expAM, 2, true));
    });

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
    setupLabel (pMSourceLabel, "SRC", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (phaseCVComboBox, "PhaseCV", [this] () { phaseCVUiChanged (phaseCVComboBox.getSelectedItemText (), phaseCVTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (phaseCVTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PhaseCV", [this] ()
    {
        FormatHelpers::setColorIfError (phaseCVTextEditor, FormatHelpers::getAmount (minChannelProperties.getPhaseCV ()), FormatHelpers::getAmount (maxChannelProperties.getPhaseCV ()));
    },
    [this] (juce::String text)
    {
        const auto phaseCV { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getPhaseCV ()),
                                                                 FormatHelpers::getAmount (maxChannelProperties.getPhaseCV ())) };
        phaseCVUiChanged (phaseCVComboBox.getSelectedItemText (), phaseCV);
        phaseCVTextEditor.setText (FormatHelpers::formatDouble (phaseCV, 2, true));
    });

    // PHASE MOD INDEX
    setupLabel (phaseModIndexSectionLabel, "PHASE MOD", kLargeLabelSize, juce::Justification::centred);
    setupTextEditor (pMIndexTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PMIndex", [this] ()
    {
        FormatHelpers::setColorIfError (pMIndexTextEditor, minChannelProperties.getPMIndex (), maxChannelProperties.getPMIndex ());
    },
    [this] (juce::String text)
    {
        const auto pmIndex { std::clamp (text.getDoubleValue (), minChannelProperties.getPMIndex (), maxChannelProperties.getPMIndex ()) };
        pMIndexUiChanged (pmIndex);
        pMIndexTextEditor.setText (FormatHelpers::formatDouble (pmIndex, 2, true));
    });
    setupLabel (pMIndexLabel, "INDEX", kMediumLabelSize, juce::Justification::centredLeft);
    setupCvInputComboBox (pMIndexModComboBox, "PMIndexMod", [this] () { pMIndexModUiChanged (pMIndexModComboBox.getSelectedItemText (), pMIndexModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (pMIndexModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PMIndexMod", [this] ()
    {
        FormatHelpers::setColorIfError (pMIndexModTextEditor, FormatHelpers::getAmount (minChannelProperties.getPMIndexMod ()), FormatHelpers::getAmount (maxChannelProperties.getPMIndexMod ()));
    },
    [this] (juce::String text)
    {
        const auto pmIndexMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getPMIndexMod ()),
                                                                    FormatHelpers::getAmount (maxChannelProperties.getPMIndexMod ())) };
        pMIndexModUiChanged (pMIndexModComboBox.getSelectedItemText (), pmIndexMod);
        pMIndexModTextEditor.setText (FormatHelpers::formatDouble (pmIndexMod, 2, true));
    });

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
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centredRight);
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, ".0123456789", "Attack", [this] ()
    {
        FormatHelpers::setColorIfError (attackTextEditor, minChannelProperties.getAttack (), maxChannelProperties.getAttack ());
    },
    [this] (juce::String text)
    {
        const auto attackTime { snapEnvelopeValue (std::clamp (text.getDoubleValue (), minChannelProperties.getAttack (), maxChannelProperties.getAttack ())) };
        arEnvelopeProperties.setAttackPercent (attackTime / static_cast<double> (kMaxEnvelopeTime * 2), false);
        attackUiChanged (attackTime);
        attackTextEditor.setText (FormatHelpers::formatDouble (attackTime, getEnvelopeValueResolution (attackTime), false));
    });
    setupCvInputComboBox (attackModComboBox, "AttackMod", [this] () { attackModUiChanged (attackModComboBox.getSelectedItemText (), attackModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (attackModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "AttackMod", [this] ()
    {
        FormatHelpers::setColorIfError (attackModTextEditor, FormatHelpers::getAmount (minChannelProperties.getAttackMod ()), FormatHelpers::getAmount (maxChannelProperties.getAttackMod ()));
    },
    [this] (juce::String text)
    {
        const auto attackMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getAttackMod ()),
                                                                    FormatHelpers::getAmount (maxChannelProperties.getAttackMod ())) };
        attackModUiChanged (attackModComboBox.getSelectedItemText (), attackMod);
        attackModTextEditor.setText (FormatHelpers::formatDouble (attackMod, 2, true));
    });

    // RELEASE
    setupTextEditor (releaseTextEditor, juce::Justification::centred, 0, ".0123456789", "Release", [this] ()
    {
        FormatHelpers::setColorIfError (releaseTextEditor, minChannelProperties.getRelease (), maxChannelProperties.getRelease ());
    },
    [this] (juce::String text)
    {
        const auto releaseTime { snapEnvelopeValue (std::clamp (text.getDoubleValue (), minChannelProperties.getRelease (), maxChannelProperties.getRelease ())) };
        arEnvelopeProperties.setReleasePercent (releaseTime / static_cast<double> (kMaxEnvelopeTime * 2), false);
        releaseUiChanged (releaseTime);
        releaseTextEditor.setText (FormatHelpers::formatDouble (releaseTime, getEnvelopeValueResolution (releaseTime), false));
    });
    setupLabel (releaseLabel, "RELEASE", kMediumLabelSize, juce::Justification::centredLeft);
    setupCvInputComboBox (releaseModComboBox, "ReleaseMod", [this] () { releaseModUiChanged (releaseModComboBox.getSelectedItemText (), releaseModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (releaseModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "ReleaseMod", [this] ()
    {
        FormatHelpers::setColorIfError (releaseModTextEditor, FormatHelpers::getAmount (minChannelProperties.getReleaseMod ()), FormatHelpers::getAmount (maxChannelProperties.getReleaseMod ()));
    },
    [this] (juce::String text)
    {
        const auto releaseMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getReleaseMod ()),
                                                                    FormatHelpers::getAmount (maxChannelProperties.getReleaseMod ())) };
        releaseModUiChanged (releaseModComboBox.getSelectedItemText (), releaseMod);
        releaseModTextEditor.setText (FormatHelpers::formatDouble (releaseMod, 2, true));
    });

    /////////////////////////////////////////
    // column three
    setupLabel (mutateLabel, "MUTATE", kLargeLabelSize, juce::Justification::centred);

    // BITS
    setupTextEditor (bitsTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Bits", [this] ()
    {
        FormatHelpers::setColorIfError (bitsTextEditor, minChannelProperties.getBits (), maxChannelProperties.getBits ());
    },
    [this] (juce::String text)
    {
        const auto bits { snapBitsValue (std::clamp (text.getDoubleValue (), minChannelProperties.getBits (), maxChannelProperties.getBits ())) };
        bitsUiChanged (bits);
        bitsTextEditor.setText (FormatHelpers::formatDouble (bits, 1, false));
    });
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (bitsModComboBox, "BitsMod", [this] () { bitsModUiChanged (bitsModComboBox.getSelectedItemText (), bitsModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (bitsModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "BitsMod", [this] ()
    {
        FormatHelpers::setColorIfError (bitsModTextEditor, FormatHelpers::getAmount (minChannelProperties.getBitsMod ()), FormatHelpers::getAmount (maxChannelProperties.getBitsMod ()));
    },
    [this] (juce::String text)
    {
        const auto bitsMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getBitsMod ()),
                                                                 FormatHelpers::getAmount (maxChannelProperties.getBitsMod ())) };
        bitsModUiChanged (bitsModComboBox.getSelectedItemText (), bitsMod);
        bitsModTextEditor.setText (FormatHelpers::formatDouble (bitsMod, 2, true));
    });

    // ALIASING
    setupTextEditor (aliasingTextEditor, juce::Justification::centred, 0, "0123456789", "Aliasing", [this] ()
    {
        FormatHelpers::setColorIfError (aliasingTextEditor, minChannelProperties.getAliasing (), maxChannelProperties.getAliasing ());
    },
    [this] (juce::String text)
    {
        const auto aliasing { std::clamp (text.getIntValue (), minChannelProperties.getAliasing (), maxChannelProperties.getAliasing ()) };
        aliasingUiChanged (aliasing);
        aliasingTextEditor.setText (juce::String (aliasing));
    });
    setupLabel (aliasingLabel, "ALIAS", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (aliasingModComboBox, "AliasingMod", [this] () { aliasingModUiChanged (aliasingModComboBox.getSelectedItemText (), aliasingModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (aliasingModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "AliasingMod", [this] ()
    {
        FormatHelpers::setColorIfError (aliasingModTextEditor, FormatHelpers::getAmount (minChannelProperties.getAliasingMod ()), FormatHelpers::getAmount (maxChannelProperties.getAliasingMod ()));
    },
    [this] (juce::String text)
    {
        const auto aliasingMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getAliasingMod ()),
                                                                     FormatHelpers::getAmount (maxChannelProperties.getAliasingMod ())) };
        aliasingModUiChanged (aliasingModComboBox.getSelectedItemText (), aliasingMod);
        aliasingModTextEditor.setText (FormatHelpers::formatDouble (aliasingMod, 2, true));
    });

    // REVERSE/SMOOTH
    setupButton (reverseButton, "REV", "Reverse", [this] () { reverseUiChanged (reverseButton.getToggleState ()); });
    setupButton (spliceSmoothingButton, "SMOOTH", "SpliceSmoothing", [this] () { reverseUiChanged (spliceSmoothingButton.getToggleState ()); });

    // PAN/MIX
    setupLabel (panMixLabel, "PAN/MIX", kLargeLabelSize, juce::Justification::centred);
    setupTextEditor (panTextEditor, juce::Justification::centred, 0, "+-.0123456789", "Pan", [this] ()
    {
        FormatHelpers::setColorIfError (panTextEditor, minChannelProperties.getPan (), maxChannelProperties.getPan ());
    },
    [this] (juce::String text)
    {
        const auto pan { std::clamp (text.getDoubleValue (), minChannelProperties.getPan (), maxChannelProperties.getPan ()) };
        panUiChanged (pan);
        panTextEditor.setText (FormatHelpers::formatDouble (pan, 2, true));
    });
    setupLabel (panLabel, "PAN", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (panModComboBox, "PanMod", [this] () { panModUiChanged (panModComboBox.getSelectedItemText (), panModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (panModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PanMod", [this] ()
    {
        FormatHelpers::setColorIfError (panModTextEditor, FormatHelpers::getAmount (minChannelProperties.getPanMod ()), FormatHelpers::getAmount (maxChannelProperties.getPanMod ()));
    },
    [this] (juce::String text)
    {
        const auto panMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getPanMod ()),
                                                                FormatHelpers::getAmount (maxChannelProperties.getPanMod ())) };
        panModUiChanged (panModComboBox.getSelectedItemText (), panMod);
        panModTextEditor.setText (FormatHelpers::formatDouble (panMod, 2, true));
    });
    setupTextEditor (mixLevelTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MixLevel", [this] ()
    {
        FormatHelpers::setColorIfError (mixLevelTextEditor, minChannelProperties.getMixLevel (), maxChannelProperties.getMixLevel ());
    },
    [this] (juce::String text)
    {
        const auto mixLevel { std::clamp (text.getDoubleValue (), minChannelProperties.getMixLevel (), maxChannelProperties.getMixLevel ()) };
        mixLevelUiChanged (mixLevel);
        mixLevelTextEditor.setText (FormatHelpers::formatDouble (mixLevel, 1, false));
    });
    setupLabel (mixLevelLabel, "MIX", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (mixModComboBox, "MixMod", [this] () { mixModUiChanged (mixModComboBox.getSelectedItemText (), mixModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (mixModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MixMod", [this] ()
    {
        FormatHelpers::setColorIfError (mixModTextEditor, FormatHelpers::getAmount (minChannelProperties.getMixMod ()), FormatHelpers::getAmount (maxChannelProperties.getMixMod ()));
    },
    [this] (juce::String text)
    {
        const auto mixMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getMixMod ()),
                                                                FormatHelpers::getAmount (maxChannelProperties.getMixMod ())) };
        mixModUiChanged (mixModComboBox.getSelectedItemText (), mixMod);
        mixModTextEditor.setText (FormatHelpers::formatDouble (mixMod, 2, true));
    });
    setupLabel (mixModIsFaderLabel, "Mix Mod", kMediumLabelSize, juce::Justification::centredRight);
    mixModIsFaderComboBox.addItem ("Normal", 1);
    mixModIsFaderComboBox.addItem ("Fader", 2);
    setupComboBox (mixModIsFaderComboBox, "MixModIsFader", [this] ()
    {
        const auto mixModIsFader { mixModIsFaderComboBox.getSelectedId () == 2 };
        mixModIsFaderUiChanged (mixModIsFader);
    });

    // /AUTO TRIGGER
    setupLabel (autoTriggerLabel, "TRIGGER", kMediumLabelSize, juce::Justification::centredRight);
    autoTriggerComboBox.addItem ("Normal", 1);
    autoTriggerComboBox.addItem ("Auto", 2);
    setupComboBox (autoTriggerComboBox, "AutoTrigger", [this] ()
    {
        const auto autoTrigger { autoTriggerComboBox.getSelectedId () == 2 };
        attackFromCurrentUiChanged (autoTrigger);
    });

    // PLAY MODE
    setupLabel (playModeLabel, "PLAY", kMediumLabelSize, juce::Justification::centredRight);
    playModeComboBox.addItem ("Gated", 1); // 0 = Gated, 1 = One Shot
    playModeComboBox.addItem ("One Shot", 2);
    setupComboBox (playModeComboBox, "PlayMode", [this] () { playModeUiChanged (playModeComboBox.getSelectedId () - 1); });

    // /LOOP MODE
    setupLabel (loopModeLabel, "LOOP", kMediumLabelSize, juce::Justification::centredRight);
    loopModeComboBox.addItem ("No Loop", 1); // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    loopModeComboBox.addItem ("Loop", 2);
    loopModeComboBox.addItem ("Loop/Release", 3);
    setupComboBox (loopModeComboBox, "LoopMode", [this] () { loopModeUiChanged (loopModeComboBox.getSelectedId () - 1); });

    /////////////////////////////////////////
    // column four
    setupLabel (channelModeLabel, "MODE", kMediumLabelSize, juce::Justification::centred);
    channelModeComboBox.addItem ("Master", ChannelProperties::ChannelMode::master + 1); // 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    channelModeComboBox.addItem ("Link", ChannelProperties::ChannelMode::link + 1);
    channelModeComboBox.addItem ("Stereo/Right", ChannelProperties::ChannelMode::stereoRight + 1);
    channelModeComboBox.addItem ("Cycle", ChannelProperties::ChannelMode::cycle + 1);
    setupComboBox (channelModeComboBox, "ChannelMode", [this] ()
    {
        channelModeUiChanged (channelModeComboBox.getSelectedId () - 1);
    });
    setupLabel (sampleStartModLabel, "SAMPLE START", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (sampleStartModComboBox, "SampleStartMod", [this] () { sampleStartModUiChanged (sampleStartModComboBox.getSelectedItemText (), sampleStartModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (sampleStartModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "SampleStartMod", [this] ()
    {
        FormatHelpers::setColorIfError (sampleStartModTextEditor, FormatHelpers::getAmount (minChannelProperties.getSampleStartMod ()), FormatHelpers::getAmount (maxChannelProperties.getSampleStartMod ()));
    },
    [this] (juce::String text)
    {
        const auto sampleStartMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getSampleStartMod ()),
                                                                        FormatHelpers::getAmount (maxChannelProperties.getSampleStartMod ())) };
        sampleStartModUiChanged (sampleStartModComboBox.getSelectedItemText (), sampleStartMod);
        sampleStartModTextEditor.setText (FormatHelpers::formatDouble (sampleStartMod, 2, true));
    });
    setupLabel (sampleEndModLabel, "SAMPLE END", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (sampleEndModComboBox, "SampleEndMod", [this] () { sampleEndModUiChanged (sampleEndModComboBox.getSelectedItemText (), sampleEndModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (sampleEndModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "SampleEndMod", [this] ()
    {
        FormatHelpers::setColorIfError (sampleEndModTextEditor, FormatHelpers::getAmount (minChannelProperties.getSampleEndMod ()), FormatHelpers::getAmount (maxChannelProperties.getSampleEndMod ()));
    },
    [this] (juce::String text)
    {
        const auto sampleEndMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getSampleEndMod ()),
                                                                      FormatHelpers::getAmount (maxChannelProperties.getSampleEndMod ())) };
        sampleEndModUiChanged (sampleEndModComboBox.getSelectedItemText (), sampleEndMod);
        sampleEndModTextEditor.setText (FormatHelpers::formatDouble (sampleEndMod, 2, true));
    });
    setupLabel (loopStartModLabel, "LOOP START", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (loopStartModComboBox, "LoopStartMod", [this] () { loopStartModUiChanged (loopStartModComboBox.getSelectedItemText (), loopStartModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (loopStartModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LoopStartMod", [this] ()
    {
        FormatHelpers::setColorIfError (loopStartModTextEditor, FormatHelpers::getAmount (minChannelProperties.getLoopStartMod ()), FormatHelpers::getAmount (maxChannelProperties.getLoopStartMod ()));
    },
    [this] (juce::String text)
    {
        const auto loopStartMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getLoopStartMod ()),
                                                                      FormatHelpers::getAmount (maxChannelProperties.getLoopStartMod ())) };
        loopStartModUiChanged (loopStartModComboBox.getSelectedItemText (), loopStartMod);
        loopStartModTextEditor.setText (FormatHelpers::formatDouble (loopStartMod, 2, true));
    });
    setupLabel (loopLengthModLabel, "LOOP LENGTH", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (loopLengthModComboBox, "LoopLengthMod", [this] () { loopLengthModUiChanged (loopLengthModComboBox.getSelectedItemText (), loopLengthModTextEditor.getText ().getDoubleValue ()); });
    setupTextEditor (loopLengthModTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LoopLengthMod", [this] ()
    {
        FormatHelpers::setColorIfError (loopLengthModTextEditor, FormatHelpers::getAmount (minChannelProperties.getLoopLengthMod ()), FormatHelpers::getAmount (maxChannelProperties.getLoopLengthMod ()));
    },
    [this] (juce::String text)
    {
        const auto loopLengthMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getLoopLengthMod ()),
                                                                       FormatHelpers::getAmount (maxChannelProperties.getLoopLengthMod ())) };
        loopLengthModUiChanged (loopLengthModComboBox.getSelectedItemText (), loopLengthMod);
        loopLengthModTextEditor.setText (FormatHelpers::formatDouble (loopLengthMod, 2, true));
    });
    setupLabel (xfadeGroupLabel, "XFADE GRP", kSmallLabelSize, juce::Justification::centredRight);
    xfadeGroupComboBox.addItem ("None", 1); // Off, A, B, C, D
    xfadeGroupComboBox.addItem ("A", 2);
    xfadeGroupComboBox.addItem ("B", 3);
    xfadeGroupComboBox.addItem ("C", 4);
    xfadeGroupComboBox.addItem ("D", 5);
    setupComboBox (xfadeGroupComboBox, "XfadeGroup", [this] () { xfadeGroupUiChanged (xfadeGroupComboBox.getText ()); });

    setupLabel (zonesCVLabel, "CV", kMediumLabelSize, juce::Justification::centredRight);
    setupCvInputComboBox (zonesCVComboBox, "ZonesCV", [this] () { zonesCVUiChanged (zonesCVComboBox.getSelectedItemText ()); });
    setupLabel (zonesRTLabel, "SELECT", kSmallLabelSize, juce::Justification::centredRight);
    zonesRTComboBox.addItem ("Gate Rise", 1); // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
    zonesRTComboBox.addItem ("Continuous", 2);
    zonesRTComboBox.addItem ("Advance", 3);
    zonesRTComboBox.addItem ("Random", 4);
    setupComboBox (zonesRTComboBox, "ZonesRT", [this] () { zonesRTUiChanged (zonesRTComboBox.getSelectedId () - 1); });
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

void ChannelEditor::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT, EditManager* theEditManager, SamplePool* theSamplePool)
{
    //DebugLog ("ChannelEditor["+ juce::String (channelProperties.getId ()) + "]", "init");
    jassert (theSamplePool != nullptr);
    samplePool = theSamplePool;

    jassert (theEditManager != nullptr);
    editManager = theEditManager;

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
                copyBufferZoneProperties.copyFrom (zoneProperties [zoneIndex].getValueTree ());
                copyBufferHasData = true;
            });
            pm.addItem ("Paste", copyBufferHasData, false, [this, zoneIndex] ()
            {
                zoneProperties [zoneIndex].copyFrom (copyBufferZoneProperties.getValueTree ());
                // if this was on the end of the zone list, make sure it's minVoltage is set to -5
                if (zoneIndex == getNumUsedZones ())
                {
                    auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneIndex, 1) };
                    zoneProperties [zoneIndex - 1].setMinVoltage (bottomBoundary + ((topBoundary - bottomBoundary) / 2), false);
                    zoneProperties [zoneIndex].setMinVoltage (-5.0, false);
                }
                updateAllZoneTabNames ();
                ensureProperZoneIsSelected ();
            });
            pm.addItem ("Delete", zoneProperties [zoneIndex].getSample ().isNotEmpty (), false, [this, zoneIndex] ()
            {
                // TODO - this setSample call seems redundant with the follow copyFrom call
                zoneProperties [zoneIndex].setSample ("", true);
                zoneProperties [zoneIndex].copyFrom (defaultZoneProperties.getValueTree ());
                // if this zone was the last in the list, but not also the first, then set the minVoltage for the new last in list to -5
                if (zoneIndex == getNumUsedZones () && zoneIndex != 0)
                    zoneProperties [zoneIndex - 1].setMinVoltage (-5.0, false);
                removeEmptyZones ();
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
                const auto numZones { getNumUsedZones () };
                for (auto curZoneIndex { 0 }; curZoneIndex < numZones; ++curZoneIndex)
                    zoneProperties [curZoneIndex].copyFrom (defaultZoneProperties.getValueTree ());
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
                auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneEditorIndex, 0) };
                return voltage > bottomBoundary && voltage < topBoundary;
            }
        };
        zoneEditor.clampMinVoltage = [this, zoneEditorIndex] (double voltage)
        {
            auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneEditorIndex, 0) };
            return std::clamp (voltage, bottomBoundary + 0.01, topBoundary - 0.01);
        };
        zoneEditor.onSampleChange = [this, zoneEditorIndex] (juce::String sampleFileName)
        {
            if (sampleFileName.isNotEmpty ())
            {
                if (zoneEditorIndex > 0)
                {
                    auto [topBoundary, bottomBoundary] { getVoltageBoundaries (zoneEditorIndex, 1) };
                    zoneProperties [zoneEditorIndex - 1].setMinVoltage (bottomBoundary + ((topBoundary - bottomBoundary) / 2), false);
                }
                if (zoneEditorIndex == getNumUsedZones () - 1)
                    zoneProperties [zoneEditorIndex].setMinVoltage (-5.0, false);
            }
            else
            {
                if (zoneEditorIndex> 0 && zoneEditorIndex == getNumUsedZones ())
                    zoneProperties [zoneEditorIndex - 1].setMinVoltage (-5.0, false);
            }
            ensureProperZoneIsSelected ();
            updateAllZoneTabNames ();
        };
        zoneEditor.handleSamples = [this] (int zoneIndex, const juce::StringArray& files)
        {
            // TODO - should this have been checked prior to this call?
            for (auto fileName : files)
                if (! zoneEditors [zoneIndex].isSupportedAudioFile (fileName))
                    return false;

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

#if JUCE_DEBUG
            // verifying that all minVoltages are valid
            for (auto curZoneIndex { 0 }; curZoneIndex < getNumUsedZones () - 1; ++curZoneIndex)
                if (zoneProperties [curZoneIndex].getMinVoltage () <= zoneProperties [curZoneIndex + 1].getMinVoltage ())
                    jassertfalse;
#endif
            return true;
        };

        // Zone Properties setup
        auto& curZoneProperties { zoneProperties [zoneEditorIndex] };
        curZoneProperties.wrap (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
        curZoneProperties.onSampleChange = [this, zoneEditorIndex] ([[maybe_unused]] juce::String sampleFile)
        {
            ensureProperZoneIsSelected ();
            updateAllZoneTabNames ();
        };
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
    auto zoneIndex { zoneTabs.getCurrentTabIndex () };
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
    channelProperties.onZonesRTChange = [this] (int zonesRT) { loopModeDataChanged (zonesRT);  };
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
}

void ChannelEditor::positionColumnOne (int xOffset, int width)
{
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    auto curYOffset { kInitialYOffset };
    pitchLabel.setBounds (xOffset, curYOffset, width, kLargeLabelIntSize);
    curYOffset += kLargeLabelIntSize;
    curYOffset += kFirstControlSectionYOffset;
    pitchTextEditor.setBounds (xOffset, curYOffset, scaleWidth (0.5f), kParameterLineHeight);
    const auto ddcSize { pitchTextEditor.getHeight () / 4 };

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
    zoneMaxVoltage.setBounds (zoneColumn.getX (), zoneColumn.getY () - 12, 40, 11);
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
    aliasingTextEditor.setText (juce::String (aliasing));
}

void ChannelEditor::aliasingUiChanged (int aliasing)
{
    channelProperties.setAliasing (aliasing, false);
}

void ChannelEditor::aliasingModDataChanged (juce::String cvInput, double aliasingMod)
{
    aliasingModComboBox.setSelectedItemText (cvInput);
    aliasingModTextEditor.setText (FormatHelpers::formatDouble (aliasingMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::aliasingModUiChanged (juce::String cvInput, double aliasingMod)
{
    channelProperties.setAliasingMod (cvInput, aliasingMod, false);
}

void ChannelEditor::attackDataChanged (double attack)
{
    arEnvelopeProperties.setAttackPercent (attack / static_cast<double> (kMaxEnvelopeTime * 2), false);
    attackTextEditor.setText (FormatHelpers::formatDouble (attack, getEnvelopeValueResolution (attack), false));
}

void ChannelEditor::attackUiChanged (double attack)
{
    channelProperties.setAttack (attack, false);
}

void ChannelEditor::attackFromCurrentDataChanged (bool attackFromCurrent)
{
    attackFromCurrentComboBox.setSelectedId (attackFromCurrent ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::attackFromCurrentUiChanged (bool attackFromCurrent)
{
    channelProperties.setAttackFromCurrent (attackFromCurrent, false);
}

void ChannelEditor::attackModDataChanged (juce::String cvInput, double attackMod)
{
    attackModComboBox.setSelectedItemText (cvInput);
    attackModTextEditor.setText (FormatHelpers::formatDouble (attackMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::attackModUiChanged (juce::String cvInput, double attackMod)
{
    channelProperties.setAttackMod (cvInput, attackMod, false);
}

void ChannelEditor::autoTriggerDataChanged (bool autoTrigger)
{
    autoTriggerComboBox.setSelectedId (autoTrigger ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::autoTriggerUiChanged (bool autoTrigger)
{
    channelProperties.setAutoTrigger (autoTrigger, false);
}

void ChannelEditor::bitsDataChanged (double bits)
{
    bitsTextEditor.setText (FormatHelpers::formatDouble (bits, 1, false));
}

void ChannelEditor::bitsUiChanged (double bits)
{
    channelProperties.setBits (bits, false);
}

void ChannelEditor::bitsModDataChanged (juce::String cvInput, double bitsMod)
{
    bitsModComboBox.setSelectedItemText (cvInput);
    bitsModTextEditor.setText (FormatHelpers::formatDouble (bitsMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::bitsModUiChanged (juce::String cvInput, double bitsMod)
{
    channelProperties.setBitsMod (cvInput, bitsMod, false);
}

void ChannelEditor::channelModeDataChanged (int channelMode)
{
    channelModeComboBox.setSelectedItemIndex (channelMode, juce::NotificationType::dontSendNotification);
    checkStereoRightOverlay ();
}

void ChannelEditor::channelModeUiChanged (int channelMode)
{
    channelProperties.setChannelMode (channelMode, false);
    checkStereoRightOverlay ();
}

void ChannelEditor::expAMDataChanged (juce::String cvInput, double expAM)
{
    expAMComboBox.setSelectedItemText (cvInput);
    expAMTextEditor.setText (FormatHelpers::formatDouble (expAM, 2, true));
}

void ChannelEditor::expAMUiChanged (juce::String cvInput, double expAM)
{
    channelProperties.setExpAM (cvInput, expAM, false);
}

void ChannelEditor::expFMDataChanged (juce::String cvInput, double expFM)
{
    expFMComboBox.setSelectedItemText (cvInput);
    expFMTextEditor.setText (FormatHelpers::formatDouble (expFM, 2, true));
}

void ChannelEditor::expFMUiChanged (juce::String cvInput, double expFM)
{
    channelProperties.setExpFM (cvInput, expFM, false);
}

void ChannelEditor::levelDataChanged (double level)
{
    levelTextEditor.setText (FormatHelpers::formatDouble (level, 1, false));
}

void ChannelEditor::levelUiChanged (double level)
{
    channelProperties.setLevel (level, false);
}

void ChannelEditor::linAMDataChanged (juce::String cvInput, double linAM)
{
    linAMComboBox.setSelectedItemText (cvInput);
    linAMTextEditor.setText (FormatHelpers::formatDouble (linAM, 2, true));
}

void ChannelEditor::linAMUiChanged (juce::String cvInput, double linAM)
{
    channelProperties.setLinAM (cvInput, linAM, false);
}

void ChannelEditor::linAMisExtEnvDataChanged (bool linAMisExtEnv)
{
    linAMisExtEnvComboBox.setSelectedId (linAMisExtEnv ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::linAMisExtEnvUiChanged (bool linAMisExtEnv)
{
    channelProperties.setLinAMisExtEnv (linAMisExtEnv, false);
}

void ChannelEditor::linFMDataChanged (juce::String cvInput, double linFM)
{
    linFMComboBox.setSelectedItemText (cvInput);
    linFMTextEditor.setText (FormatHelpers::formatDouble (linFM, 2, true));
}

void ChannelEditor::linFMUiChanged (juce::String cvInput, double linFM)
{
    channelProperties.setLinFM (cvInput, linFM, false);
}

void ChannelEditor::loopLengthIsEndDataChanged (bool loopLengthIsEnd)
{
    // inform all the zones of the change
    for (auto& zoneEditor : zoneEditors)
        zoneEditor.setLoopLengthIsEnd (loopLengthIsEnd);

    loopLengthIsEndComboBox.setSelectedId (loopLengthIsEnd ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopLengthIsEndUiChanged (bool loopLengthIsEnd)
{
    channelProperties.setLoopLengthIsEnd (loopLengthIsEnd, false);
}

void ChannelEditor::loopLengthModDataChanged (juce::String cvInput, double loopLengthMod)
{
    loopLengthModComboBox.setSelectedItemText (cvInput);
    loopLengthModTextEditor.setText (FormatHelpers::formatDouble (loopLengthMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopLengthModUiChanged (juce::String cvInput, double loopLengthMod)
{
    channelProperties.setLoopLengthMod (cvInput, loopLengthMod, false);
}

void ChannelEditor::loopModeDataChanged (int loopMode)
{
    loopModeComboBox.setSelectedItemIndex (loopMode, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopModeUiChanged (int loopMode)
{
    channelProperties.setLoopMode (loopMode, false);
}

void ChannelEditor::loopStartModDataChanged (juce::String cvInput, double loopStartMod)
{
    loopStartModComboBox.setSelectedItemText (cvInput);
    loopStartModTextEditor.setText (FormatHelpers::formatDouble (loopStartMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopStartModUiChanged (juce::String cvInput, double loopStartMod)
{
    channelProperties.setLoopStartMod (cvInput, loopStartMod, false);
}

void ChannelEditor::mixLevelDataChanged (double mixLevel)
{
    mixLevelTextEditor.setText (FormatHelpers::formatDouble (mixLevel, 1, false));
}

void ChannelEditor::mixLevelUiChanged (double mixLevel)
{
    channelProperties.setMixLevel (mixLevel, false);
}

void ChannelEditor::mixModDataChanged (juce::String cvInput, double mixMod)
{
    mixModComboBox.setSelectedItemText (cvInput);
    mixModTextEditor.setText (FormatHelpers::formatDouble (mixMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::mixModUiChanged (juce::String cvInput, double mixMod)
{
    channelProperties.setMixMod (cvInput, mixMod, false);
}

void ChannelEditor::mixModIsFaderDataChanged (bool mixModIsFader)
{
    mixModIsFaderComboBox.setSelectedId (mixModIsFader ? 2 : 1, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::mixModIsFaderUiChanged (bool mixModIsFader)
{
    channelProperties.setMixModIsFader (mixModIsFader, false);
}

void ChannelEditor::panDataChanged (double pan)
{
    panTextEditor.setText (FormatHelpers::formatDouble (pan, 2, true));
}

void ChannelEditor::panUiChanged (double pan)
{
    channelProperties.setPan (pan, false);
}

void ChannelEditor::panModDataChanged (juce::String cvInput, double panMod)
{
    panModComboBox.setSelectedItemText (cvInput);
    panModTextEditor.setText (FormatHelpers::formatDouble (panMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::panModUiChanged (juce::String cvInput, double panMod)
{
    channelProperties.setPanMod (cvInput, panMod, false);
}

void ChannelEditor::phaseCVDataChanged (juce::String cvInput, double phaseCV)
{
    phaseCVComboBox.setSelectedItemText (cvInput);
    phaseCVTextEditor.setText (FormatHelpers::formatDouble (phaseCV, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::phaseCVUiChanged (juce::String cvInput, double phaseCV)
{
    channelProperties.setPhaseCV (cvInput, phaseCV, false);
}

void ChannelEditor::pitchDataChanged (double pitch)
{
    pitchTextEditor.setText (FormatHelpers::formatDouble (pitch, 2, true));
}

void ChannelEditor::pitchUiChanged (double pitch)
{
    channelProperties.setPitch (pitch, false);
}

void ChannelEditor::pitchCVDataChanged (juce::String cvInput, double pitchCV)
{
    pitchCVComboBox.setSelectedItemText (cvInput);
    pitchCVTextEditor.setText (FormatHelpers::formatDouble (pitchCV, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::pitchCVUiChanged (juce::String cvInput, double pitchCV)
{
    channelProperties.setPitchCV (cvInput, pitchCV, false);
}

void ChannelEditor::playModeDataChanged (int playMode)
{
    playModeComboBox.setSelectedItemIndex (playMode, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::playModeUiChanged (int playMode)
{
    channelProperties.setPlayMode (playMode, false);
}

void ChannelEditor::pMIndexDataChanged (double pMIndex)
{
    pMIndexTextEditor.setText (FormatHelpers::formatDouble (pMIndex, 2, true));
}

void ChannelEditor::pMIndexUiChanged (double pMIndex)
{
    channelProperties.setPMIndex (pMIndex, false);
}

void ChannelEditor::pMIndexModDataChanged (juce::String cvInput, double pMIndexMod)
{
    pMIndexModComboBox.setSelectedItemText (cvInput);
    pMIndexModTextEditor.setText (FormatHelpers::formatDouble (pMIndexMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::pMIndexModUiChanged (juce::String cvInput, double pMIndexMod)
{
    channelProperties.setPMIndexMod (cvInput, pMIndexMod, false);
}

void ChannelEditor::pMSourceDataChanged (int pMSource)
{
    pMSourceComboBox.setSelectedItemIndex (pMSource, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::pMSourceUiChanged (int pMSource)
{
    channelProperties.setPMSource (pMSource, false);
}

void ChannelEditor::releaseDataChanged (double release)
{
    arEnvelopeProperties.setReleasePercent (release / static_cast<double> (kMaxEnvelopeTime * 2), false);
    releaseTextEditor.setText (FormatHelpers::formatDouble (release, getEnvelopeValueResolution (release), false));
}

void ChannelEditor::releaseUiChanged (double release)
{
    channelProperties.setRelease (release, false);
}

void ChannelEditor::releaseModDataChanged (juce::String cvInput, double releaseMod)
{
    releaseModComboBox.setSelectedItemText (cvInput);
    releaseModTextEditor.setText (FormatHelpers::formatDouble (releaseMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::releaseModUiChanged (juce::String cvInput, double releaseMod)
{
    channelProperties.setReleaseMod (cvInput, releaseMod, false);
}

void ChannelEditor::reverseDataChanged (bool reverse)
{
    reverseButton.setToggleState (reverse, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::reverseUiChanged (bool reverse)
{
    channelProperties.setReverse (reverse, false);
}

void ChannelEditor::sampleEndModDataChanged (juce::String cvInput, double sampleEndMod)
{
    sampleEndModComboBox.setSelectedItemText (cvInput);
    sampleEndModTextEditor.setText (FormatHelpers::formatDouble (sampleEndMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::sampleEndModUiChanged (juce::String cvInput, double sampleEndMod)
{
    channelProperties.setSampleEndMod (cvInput, sampleEndMod, false);
}

void ChannelEditor::sampleStartModDataChanged (juce::String cvInput, double sampleStartMod)
{
    sampleStartModComboBox.setSelectedItemText (cvInput);
    sampleStartModTextEditor.setText (FormatHelpers::formatDouble (sampleStartMod, 2, true), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::sampleStartModUiChanged (juce::String cvInput, double sampleStartMod)
{
    channelProperties.setSampleStartMod (cvInput, sampleStartMod, false);
}

void ChannelEditor::spliceSmoothingDataChanged (bool spliceSmoothing)
{
    spliceSmoothingButton.setToggleState (spliceSmoothing, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::spliceSmoothingUiChanged (bool spliceSmoothing)
{
    channelProperties.setSpliceSmoothing (spliceSmoothing, false);
}

void ChannelEditor::xfadeGroupDataChanged (juce::String xfadeGroup)
{
    xfadeGroupComboBox.setText (xfadeGroup);
}

void ChannelEditor::xfadeGroupUiChanged (juce::String xfadeGroup)
{
    channelProperties.setXfadeGroup (xfadeGroup, false);
}

void ChannelEditor::zonesCVDataChanged (juce::String zonesCV)
{
    zonesCVComboBox.setSelectedItemText (zonesCV);
}

void ChannelEditor::zonesCVUiChanged (juce::String zonesCV)
{
    channelProperties.setZonesCV (zonesCV, false);
}

void ChannelEditor::zonesRTDataChanged (int zonesRT)
{
    zonesRTComboBox.setSelectedItemIndex (zonesRT, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::zonesRTUiChanged (int zonesRT)
{
    channelProperties.setZonesRT (zonesRT, false);
}
