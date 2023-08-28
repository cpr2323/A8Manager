#include "ChannelEditor.h"
#include "FormatHelpers.h"
#include "ParameterToolTipData.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"

const auto kLargeLabelSize { 20.0f };
const auto kMediumLabelSize { 15.0f };
const auto kLargeLabelIntSize { static_cast<int>(kLargeLabelSize) };
const auto kMediumLabelIntSize { static_cast<int>(kMediumLabelSize) };

ChannelEditor::ChannelEditor ()
{
    zonesLabel.setText ("Zones", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (zonesLabel);

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
    setupLabel (loopLengthIsEndLabel, "Loop Length:", 12.0, juce::Justification::centredLeft);
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
        zoneTabs.addTab (juce::String::charToString ('1' + curZoneIndex), juce::Colours::darkgrey, &zoneEditors [curZoneIndex], false);
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

    setupChannelComponents ();
    addChildComponent (stereoRightTransparantOverly);
}

ChannelEditor::~ChannelEditor ()
{
    attackFromCurrentComboBox.setLookAndFeel (&noArrowComboBoxLnF);
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

void ChannelEditor::setupChannelComponents ()
{
    juce::XmlDocument xmlDoc { BinaryData::Assimil8orToolTips_xml };
    auto xmlElement { xmlDoc.getDocumentElement (false) };
    if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
        juce::Logger::outputDebugString ("XML Parsing Error for Assimil8orToolTips_xml: " + parseError);
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
    //
    setupLabel (pitchSemiLabel, "SEMI", kMediumLabelSize, juce::Justification::centredLeft);
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
    setupLabel (linFMLabel, "LINFM", kLargeLabelSize, juce::Justification::centred);
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
    setupLabel (expFMLabel, "EXPFM", kLargeLabelSize, juce::Justification::centred);
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
    setupLabel (levelDbLabel, "dB", kMediumLabelSize, juce::Justification::centredLeft);

    // LINAM
    setupLabel (linAMLabel, "LINAM", kLargeLabelSize, juce::Justification::centred);
    setupLabel (linAMisExtEnvLabel, "BIAS", kMediumLabelSize, juce::NotificationType::dontSendNotification);
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
    setupLabel (expAMLabel, "EXPAM", kLargeLabelSize, juce::Justification::centred);
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
    setupLabel (pMSourceLabel, "SOURCE", kMediumLabelSize, juce::Justification::centredLeft);
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
    setupTextEditor (attackTextEditor, juce::Justification::centred, 0, ".0123456789", "Attack", [this] ()
    {
        FormatHelpers::setColorIfError (attackTextEditor, minChannelProperties.getAttack (), maxChannelProperties.getAttack ());
    },
    [this] (juce::String text)
    {
        const auto attack { std::clamp (text.getDoubleValue (), minChannelProperties.getAttack (), maxChannelProperties.getAttack ()) };
        attackUiChanged (attack);
        attackTextEditor.setText (FormatHelpers::formatDouble (attack, 4, false));
    });
    setupLabel (attackLabel, "ATTACK", kMediumLabelSize, juce::Justification::centredLeft);
    setupLabel (attackFromCurrentLabel, "FROM", kMediumLabelSize, juce::Justification::centredLeft);
    attackFromCurrentComboBox.addItem ("Zero", 1);
    attackFromCurrentComboBox.addItem ("Current", 2);
    setupComboBox (attackFromCurrentComboBox, "AttackFromCurrent", [this] ()
    {
        const auto attackFromCurrent { attackFromCurrentComboBox.getSelectedId () == 2 };
        attackFromCurrentUiChanged (attackFromCurrent);
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
        const auto release { std::clamp (text.getDoubleValue (), minChannelProperties.getRelease (), maxChannelProperties.getRelease ()) };
        releaseUiChanged (release);
        releaseTextEditor.setText (FormatHelpers::formatDouble (release, 4, false));
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
        const auto bits { std::clamp (text.getDoubleValue (), minChannelProperties.getBits (), maxChannelProperties.getBits ()) };
        bitsUiChanged (bits);
        bitsTextEditor.setText (FormatHelpers::formatDouble (bits, 1, false));
    });
    setupLabel (bitsLabel, "BITS", kMediumLabelSize, juce::Justification::centredLeft);
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
        aliasingTextEditor.setText (juce::String(aliasing));
    });
    setupLabel (aliasingLabel, "ALIAS", kMediumLabelSize, juce::Justification::centredLeft);
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
    setupLabel (panLabel, "PAN", kMediumLabelSize, juce::Justification::centredLeft);
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
    setupLabel (mixLevelLabel, "MIX", kMediumLabelSize, juce::Justification::centredLeft);
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
    setupLabel (mixModIsFaderLabel, "Mix Mod", kMediumLabelSize, juce::Justification::centredLeft);
    mixModIsFaderComboBox.addItem ("Normal", 1);
    mixModIsFaderComboBox.addItem ("Fader", 2);
    setupComboBox (mixModIsFaderComboBox, "MixModIsFader", [this] ()
    {
        const auto mixModIsFader { mixModIsFaderComboBox.getSelectedId () == 2 };
        mixModIsFaderUiChanged (mixModIsFader);
    });

    // PLAY MODE
    setupLabel (playModeLabel, "PLAY", kMediumLabelSize, juce::Justification::centred);
    playModeComboBox.addItem ("Gated", 1); // 0 = Gated, 1 = One Shot
    playModeComboBox.addItem ("One Shot", 2);
    setupComboBox (playModeComboBox, "PlayMode", [this] () { playModeUiChanged (playModeComboBox.getSelectedId () - 1); });

    // /LOOP MODE
    setupLabel (loopModeLabel, "LOOP", kMediumLabelSize, juce::Justification::centred);
    loopModeComboBox.addItem ("No Loop", 1); // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    loopModeComboBox.addItem ("Loop", 2);
    loopModeComboBox.addItem ("Loop/Release", 3);
    setupComboBox (loopModeComboBox, "LoopMode", [this] () { loopModeUiChanged (loopModeComboBox.getSelectedId () - 1); });

    // /AUTO TRIGGER
    setupLabel (autoTriggerLabel, "TRIGGER", kMediumLabelSize, juce::Justification::centred);

    // Trigger on Select
    autoTriggerComboBox.addItem ("Normal", 1);
    autoTriggerComboBox.addItem ("Auto", 2);
    setupComboBox (autoTriggerComboBox, "AutoTrigger", [this] ()
    {
        const auto autoTrigger { autoTriggerComboBox.getSelectedId () == 2 };
        attackFromCurrentUiChanged (autoTrigger);
    });

    /////////////////////////////////////////
    // column four
    setupLabel (channelModeLabel, "CHANNEL", kMediumLabelSize, juce::Justification::centred);
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
        // TODO - after implementing LoopLength/LoopEnd switch, update code here to use it
        const auto loopLengthMod { std::clamp (text.getDoubleValue (), FormatHelpers::getAmount (minChannelProperties.getLoopLengthMod ()),
                                                                       FormatHelpers::getAmount (maxChannelProperties.getLoopLengthMod ())) };
        loopLengthModUiChanged (loopLengthModComboBox.getSelectedItemText (), loopLengthMod);
        loopLengthModTextEditor.setText (FormatHelpers::formatDouble (loopLengthMod, 2, true));
    });
    setupLabel (xfadeGroupLabel, "CROSSFADE", kMediumLabelSize, juce::Justification::centred);
    xfadeGroupComboBox.addItem ("None", 1); // Off, A, B, C, D
    xfadeGroupComboBox.addItem ("A", 2);
    xfadeGroupComboBox.addItem ("B", 3);
    xfadeGroupComboBox.addItem ("C", 4);
    xfadeGroupComboBox.addItem ("D", 5);
    setupComboBox (xfadeGroupComboBox, "XfadeGroup", [this] () { xfadeGroupUiChanged (xfadeGroupComboBox.getText ()); });
    setupLabel (zonesCVLabel, "ZONE CV", kMediumLabelSize, juce::Justification::centred);
    setupCvInputComboBox (zonesCVComboBox, "ZonesCV", [this] () { zonesCVUiChanged (zonesCVComboBox.getSelectedItemText ()); });
    setupLabel (zonesRTLabel, "ZONE SELECTION", kMediumLabelSize, juce::Justification::centred);
    zonesRTComboBox.addItem ("Gate Rise", 1); // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
    zonesRTComboBox.addItem ("Continuous", 2);
    zonesRTComboBox.addItem ("Advance", 3);
    zonesRTComboBox.addItem ("Random", 4);
    setupComboBox (zonesRTComboBox, "ZonesRT", [this] () { zonesRTUiChanged (zonesRTComboBox.getSelectedId () - 1); });
}

void ChannelEditor::init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT)
{
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    setupChannelPropertiesCallbacks ();
    auto zoneEditorIndex { 0 };
    channelProperties.forEachZone([this, &zoneEditorIndex, rootPropertiesVT] (juce::ValueTree zonePropertiesVT)
    {
        zoneEditors [zoneEditorIndex].init (zonePropertiesVT, rootPropertiesVT);
        ++zoneEditorIndex;
        return true;
    });

    auto splitAndPassAsParams = [] (CvInputAndAmount amountAndCvInput, std::function<void (juce::String,double)> setter)
    {
        jassert (setter != nullptr);
        const auto& [cvInput, value] { amountAndCvInput };
        setter (cvInput, value);
    };
    aliasingDataChanged (channelProperties.getAliasing ());
    splitAndPassAsParams (channelProperties.getAliasingMod (), [this] (juce::String cvInput, double value) { aliasingModDataChanged (cvInput, value); });
    attackDataChanged (channelProperties.getAttack ());
    attackFromCurrentDataChanged (channelProperties.getAttackFromCurrent ());
    splitAndPassAsParams (channelProperties.getAttackMod (), [this] (juce::String cvInput, double value) { attackModDataChanged (cvInput, value); });
    autoTriggerDataChanged (channelProperties.getAutoTrigger ());
    bitsDataChanged (channelProperties.getBits ());
    splitAndPassAsParams (channelProperties.getBitsMod (), [this] (juce::String cvInput, double value) { bitsModDataChanged (cvInput, value); });
    channelModeDataChanged (channelProperties.getChannelMode ());
    splitAndPassAsParams (channelProperties.getExpAM (), [this] (juce::String cvInput, double value) { expAMDataChanged (cvInput, value); });
    splitAndPassAsParams (channelProperties.getExpFM (), [this] (juce::String cvInput, double value) { expFMDataChanged (cvInput, value); });
    levelDataChanged (channelProperties.getLevel ());
    splitAndPassAsParams (channelProperties.getLinAM (), [this] (juce::String cvInput, double value) { linAMDataChanged (cvInput, value); });
    linAMisExtEnvDataChanged (channelProperties.getLinAMisExtEnv ());
    splitAndPassAsParams (channelProperties.getLinFM (), [this] (juce::String cvInput, double value) { linFMDataChanged (cvInput, value); });
    loopLengthIsEndDataChanged (channelProperties.getLoopLengthIsEnd ());
    splitAndPassAsParams (channelProperties.getLoopLengthMod (), [this] (juce::String cvInput, double value) { loopLengthModDataChanged (cvInput, value); });
    loopModeDataChanged (channelProperties.getLoopMode ());
    splitAndPassAsParams (channelProperties.getLoopStartMod (), [this] (juce::String cvInput, double value) { loopStartModDataChanged (cvInput, value); });
    mixLevelDataChanged (channelProperties.getMixLevel ());
    splitAndPassAsParams (channelProperties.getMixMod (), [this] (juce::String cvInput, double value) { mixModDataChanged (cvInput, value); });
    mixModIsFaderDataChanged (channelProperties.getMixModIsFader ());
    panDataChanged (channelProperties.getPan ());
    splitAndPassAsParams (channelProperties.getPanMod (), [this] (juce::String cvInput, double value) { panModDataChanged (cvInput, value); });
    splitAndPassAsParams (channelProperties.getPhaseCV (), [this] (juce::String cvInput, double value) { phaseCVDataChanged (cvInput, value); });
    pitchDataChanged (channelProperties.getPitch ());
    splitAndPassAsParams (channelProperties.getPitchCV (), [this] (juce::String cvInput, double value) { pitchCVDataChanged (cvInput, value); });
    playModeDataChanged (channelProperties.getPlayMode ());
    pMIndexDataChanged (channelProperties.getPMIndex ());
    splitAndPassAsParams (channelProperties.getPMIndexMod (), [this] (juce::String cvInput, double value) { pMIndexModDataChanged (cvInput, value); });
    pMSourceDataChanged (channelProperties.getPMSource ());
    releaseDataChanged (channelProperties.getRelease ());
    splitAndPassAsParams (channelProperties.getReleaseMod (), [this] (juce::String cvInput, double value) { releaseModDataChanged (cvInput, value); });
    reverseDataChanged (channelProperties.getReverse ());
    splitAndPassAsParams (channelProperties.getSampleStartMod (), [this] (juce::String cvInput, double value) { sampleStartModDataChanged (cvInput, value); });
    splitAndPassAsParams (channelProperties.getSampleEndMod (), [this] (juce::String cvInput, double value) { sampleEndModDataChanged (cvInput, value); });
    spliceSmoothingDataChanged (channelProperties.getSpliceSmoothing ());
    xfadeGroupDataChanged (channelProperties.getXfadeGroup ());
    zonesCVDataChanged (channelProperties.getZonesCV ());
    zonesRTDataChanged (channelProperties.getZonesRT ());
}

void ChannelEditor::setupChannelPropertiesCallbacks ()
{
    channelProperties.onIndexChange = [this] ([[maybe_unused]] int index) { jassertfalse; /* I don't think this should change while we are editing */};
    channelProperties.onAliasingChange = [this] (int aliasing) { aliasingDataChanged (aliasing);  };
    channelProperties.onAliasingModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; aliasingModDataChanged (cvInput, value); };
    channelProperties.onAttackChange = [this] (double attack) { attackDataChanged (attack);  };
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
    channelProperties.onReleaseChange = [this] (double release) { releaseDataChanged (release);  };
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

void ChannelEditor::paint ([[maybe_unused]] juce::Graphics& g)
{
}

void ChannelEditor::resized ()
{
    stereoRightTransparantOverly.setBounds (getLocalBounds ());
    auto zoneBounds {getLocalBounds ().removeFromRight(getWidth () / 4)};
    zoneBounds.removeFromTop (3);
    auto zoneTopRow { zoneBounds.removeFromTop (25).withTrimmedBottom (5).withTrimmedRight (3)};
    zonesLabel.setBounds (zoneTopRow.removeFromLeft(40));
    loopLengthIsEndComboBox.setBounds (zoneTopRow.removeFromRight (30));
    loopLengthIsEndLabel.setBounds (zoneTopRow.removeFromRight (70));
    zoneTabs.setBounds (zoneBounds);

    // column one
    const auto columnOneXOffset { 5 };
    const auto columnOneWidth { 100};
    pitchLabel.setBounds (columnOneXOffset, 5, columnOneWidth, kLargeLabelIntSize);
    pitchTextEditor.setBounds (pitchLabel.getX (), pitchLabel.getBottom (), columnOneWidth / 2, 20);
    pitchSemiLabel.setBounds (pitchTextEditor.getRight () + 3, pitchTextEditor.getY (), columnOneWidth / 2, kMediumLabelIntSize);
    pitchCVComboBox.setBounds (pitchLabel.getX (), pitchTextEditor.getBottom (), columnOneWidth / 2, 20);
    pitchCVTextEditor.setBounds (pitchCVComboBox.getRight () + 3, pitchCVComboBox.getY (), columnOneWidth / 2, 20);

    linFMLabel.setBounds (columnOneXOffset, pitchCVComboBox.getBottom() + 5, columnOneWidth, kLargeLabelIntSize);
    linFMComboBox.setBounds (linFMLabel.getX (), linFMLabel.getBottom (), columnOneWidth / 2, 20);
    linFMTextEditor.setBounds (linFMComboBox.getRight () + 3, linFMComboBox.getY (), columnOneWidth / 2, 20);

    expFMLabel.setBounds (columnOneXOffset, linFMComboBox.getBottom () + 5, columnOneWidth, kLargeLabelIntSize);
    expFMComboBox.setBounds (expFMLabel.getX (), expFMLabel.getBottom (), columnOneWidth / 2, 20);
    expFMTextEditor.setBounds (expFMComboBox.getRight () + 3, expFMComboBox.getY (), columnOneWidth / 2, 20);

    levelLabel.setBounds (columnOneXOffset, expFMComboBox.getBottom () + 5, columnOneWidth, kLargeLabelIntSize);
    levelTextEditor.setBounds (levelLabel.getX (), levelLabel.getBottom (), columnOneWidth / 2, 20);
    levelDbLabel.setBounds (levelTextEditor.getRight () + 3, levelTextEditor.getY (), columnOneWidth / 2, kMediumLabelIntSize);

    linAMLabel.setBounds (columnOneXOffset, levelTextEditor.getBottom () + 5, columnOneWidth, kLargeLabelIntSize);
    linAMisExtEnvLabel.setBounds (linAMLabel.getX (), linAMLabel.getBottom (), columnOneWidth / 3, kMediumLabelIntSize);
    linAMisExtEnvComboBox.setBounds (linAMisExtEnvLabel.getRight () + 3, linAMisExtEnvLabel.getY (), columnOneWidth - (columnOneWidth / 3), 20);
    linAMComboBox.setBounds (linAMisExtEnvLabel.getX (), linAMisExtEnvComboBox.getBottom (), columnOneWidth / 2, 20);
    linAMTextEditor.setBounds (linAMComboBox.getRight () + 3, linAMComboBox.getY (), columnOneWidth / 2, 20);

    expAMLabel.setBounds (columnOneXOffset, linAMComboBox.getBottom () + 5, columnOneWidth, kLargeLabelIntSize);
    expAMComboBox.setBounds (expAMLabel.getX (), expAMLabel.getBottom (), columnOneWidth / 2, 20);
    expAMTextEditor.setBounds (expAMComboBox.getRight () + 3, expAMComboBox.getY (), columnOneWidth / 2, 20);

    //column two
    const auto columnTwoXOffset { columnOneXOffset + 150 };
    const auto columnTwoWidth { 100 };
    phaseSourceSectionLabel.setBounds (columnTwoXOffset, 5, columnTwoWidth, kLargeLabelIntSize);
    pMSourceComboBox.setBounds (phaseSourceSectionLabel.getX (), phaseSourceSectionLabel.getBottom (), columnTwoWidth, 20);
    pMSourceLabel.setBounds (pMSourceComboBox.getRight () + 3, pMSourceComboBox.getY (), 40, kMediumLabelIntSize);
    phaseCVComboBox.setBounds (phaseSourceSectionLabel.getX (), pMSourceComboBox.getBottom (), columnTwoWidth / 2, 20);
    phaseCVTextEditor.setBounds (phaseCVComboBox.getRight () + 3, phaseCVComboBox.getY (), columnTwoWidth / 2, 20);

    phaseModIndexSectionLabel.setBounds (columnTwoXOffset, phaseCVTextEditor.getBottom () + 5, columnTwoWidth, kLargeLabelIntSize);
    pMIndexLabel.setBounds (phaseModIndexSectionLabel.getX (), phaseModIndexSectionLabel.getBottom (), columnTwoWidth / 2, kMediumLabelIntSize);
    pMIndexTextEditor.setBounds (pMIndexLabel.getRight () + 3, pMIndexLabel.getY (), columnTwoWidth / 2, 20);
    pMIndexModComboBox.setBounds (phaseModIndexSectionLabel.getX (), pMIndexTextEditor.getBottom (), columnTwoWidth / 2, 20);
    pMIndexModTextEditor.setBounds (pMIndexModComboBox.getRight () + 3, pMIndexModComboBox.getY (), columnTwoWidth / 2, 20);

    envelopeLabel.setBounds (columnTwoXOffset, pMIndexModComboBox.getBottom () + 5, columnTwoWidth, kLargeLabelIntSize);
    // ATTACK
    attackLabel.setBounds (envelopeLabel.getX (), envelopeLabel.getBottom (), columnTwoWidth / 2, kMediumLabelIntSize);
    attackTextEditor.setBounds (attackLabel.getRight () + 3, attackLabel.getY (), columnTwoWidth / 2, 20);
    attackFromCurrentLabel.setBounds (attackLabel.getX (), attackTextEditor.getBottom (), columnTwoWidth / 2, kMediumLabelIntSize);
    attackFromCurrentComboBox.setBounds (attackFromCurrentLabel.getRight () + 3, attackFromCurrentLabel.getY (), columnTwoWidth / 2, 20);
    attackModComboBox.setBounds (attackFromCurrentLabel.getX (), attackFromCurrentComboBox.getBottom (), columnTwoWidth / 2, 20);
    attackModTextEditor.setBounds (attackModComboBox.getRight () + 3, attackModComboBox.getY (), columnTwoWidth / 2, 20);

    // RELEASE
    releaseLabel.setBounds (attackModComboBox.getX (), attackModComboBox.getBottom (), columnTwoWidth / 2, kMediumLabelIntSize);
    releaseTextEditor.setBounds (releaseLabel.getRight () + 3, releaseLabel.getY (), columnTwoWidth / 2, 20);
    releaseModComboBox.setBounds (releaseLabel.getX (), releaseTextEditor.getBottom (), columnTwoWidth / 2, 20);
    releaseModTextEditor.setBounds (releaseModComboBox.getRight () + 3, releaseModComboBox.getY (), columnTwoWidth / 2, 20);

    // column three
    const auto columnThreeXOffset { columnTwoXOffset + 165 };
    const auto columnThreeWidth { 100 };

    // MUTATE
    mutateLabel.setBounds (columnThreeXOffset, 5, columnThreeWidth, kLargeLabelIntSize);
    // BITS
    bitsLabel.setBounds (mutateLabel.getX (), mutateLabel.getBottom (), columnThreeWidth / 2, kMediumLabelIntSize);
    bitsTextEditor.setBounds (bitsLabel.getRight () + 3, bitsLabel.getY (), columnThreeWidth / 2, 20);
    bitsModComboBox.setBounds (bitsLabel.getX (), bitsTextEditor.getBottom (), columnThreeWidth / 2, 20);
    bitsModTextEditor.setBounds (bitsModComboBox.getRight () + 3, bitsModComboBox.getY (), columnThreeWidth / 2, 20);
    // ALIASING
    aliasingLabel.setBounds (bitsModComboBox.getX (), bitsModComboBox.getBottom () + 5, columnThreeWidth / 2, kMediumLabelIntSize);
    aliasingTextEditor.setBounds (aliasingLabel.getRight () + 3, aliasingLabel.getY (), columnThreeWidth / 2, 20);
#if 1
    aliasingModComboBox.setBounds (mutateLabel.getX (), aliasingTextEditor.getBottom (), columnThreeWidth / 2, 20);
    aliasingModTextEditor.setBounds (aliasingModComboBox.getRight () + 3, aliasingModComboBox.getY (), columnThreeWidth / 2, 20);

    // REVERSE/SMOOTH
    reverseButton.setBounds (aliasingModComboBox.getX (), aliasingModTextEditor.getBottom () + 5, columnThreeWidth * 0.40, 20);
    spliceSmoothingButton.setBounds (reverseButton.getRight () + 3, aliasingModTextEditor.getBottom () + 5, columnThreeWidth * 0.60, 20);

    panMixLabel.setBounds (columnThreeXOffset, reverseButton.getBottom () + 5, columnThreeWidth, kLargeLabelIntSize);
    panLabel.setBounds (panMixLabel.getX (), panMixLabel.getBottom (), columnThreeWidth / 2, kMediumLabelIntSize);
    panTextEditor.setBounds (panLabel.getRight () + 3, panLabel.getY (), columnThreeWidth / 2, 20);
    panModComboBox.setBounds (panLabel.getX (), panTextEditor.getBottom (), columnThreeWidth / 2, 20);
    panModTextEditor.setBounds (panModComboBox.getRight () + 3, panModComboBox.getY (), columnThreeWidth / 2, 20);

    mixLevelLabel.setBounds (panModComboBox.getX(), panModComboBox.getBottom (), columnThreeWidth / 2, kMediumLabelIntSize);
    mixLevelTextEditor.setBounds (mixLevelLabel.getRight () + 3, mixLevelLabel.getY (), columnThreeWidth / 2, 20);
    mixModIsFaderLabel.setBounds (mixLevelLabel.getX (), mixLevelTextEditor.getBottom (), columnThreeWidth / 2, kMediumLabelIntSize);
    mixModIsFaderComboBox.setBounds (mixModIsFaderLabel.getRight () + 3, mixModIsFaderLabel.getY (), columnThreeWidth / 2, 20);
    mixModComboBox.setBounds (mixModIsFaderLabel.getX (), mixModIsFaderComboBox.getBottom (), columnThreeWidth / 2, 20);
    mixModTextEditor.setBounds (mixModComboBox.getRight () + 3, mixModComboBox.getY (), columnThreeWidth / 2, 20);

    // AUTO TRIGGER
    autoTriggerLabel.setBounds (mixModComboBox.getX (), mixModComboBox.getBottom () + 5, ((columnThreeWidth / 3) * 2) - 15, kMediumLabelIntSize);
    autoTriggerComboBox.setBounds (autoTriggerLabel.getRight () + 3, autoTriggerLabel.getY (), (columnThreeWidth / 3 + 15), 20);

    // PLAY MODE
    playModeLabel.setBounds (autoTriggerLabel.getX (), autoTriggerComboBox.getBottom (), columnThreeWidth / 3, kMediumLabelIntSize);
    playModeComboBox.setBounds (playModeLabel.getRight () + 3, playModeLabel.getY (), (columnThreeWidth / 3) * 2, 20);

    // LOOP MODE
    loopModeLabel.setBounds (playModeLabel.getX (), playModeComboBox.getBottom (), columnThreeWidth / 3, kMediumLabelIntSize);
    loopModeComboBox.setBounds (loopModeLabel.getRight () + 3, loopModeLabel.getY (), (columnThreeWidth / 3) * 2, 20);
#endif

    // column four
    const auto columnFourXOffset { columnThreeXOffset + 160 };
    const auto columnFourWidth { 100 };

    channelModeLabel.setBounds (columnFourXOffset, 5, columnFourWidth, kMediumLabelIntSize);
    channelModeComboBox.setBounds (channelModeLabel.getX(), channelModeLabel.getBottom (), columnFourWidth, 20);
    loopStartModLabel.setBounds (columnFourXOffset, channelModeComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    loopStartModComboBox.setBounds (loopStartModLabel.getX (), loopStartModLabel.getBottom (), columnFourWidth / 2, 20);
    loopStartModTextEditor.setBounds (loopStartModComboBox.getRight() + 3, loopStartModComboBox.getY(), columnThreeWidth / 2, 20);
    loopLengthModLabel.setBounds (columnFourXOffset, loopStartModComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    loopLengthModComboBox.setBounds (loopLengthModLabel.getX (), loopLengthModLabel.getBottom (), columnFourWidth / 2, 20);
    loopLengthModTextEditor.setBounds (loopLengthModComboBox.getRight () + 3, loopLengthModComboBox.getY (), columnThreeWidth / 2, 20);
    sampleStartModLabel.setBounds (columnFourXOffset, loopLengthModComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    sampleStartModComboBox.setBounds (sampleStartModLabel.getX (), sampleStartModLabel.getBottom (), columnFourWidth / 2, 20);
    sampleStartModTextEditor.setBounds (sampleStartModComboBox.getRight () + 3, sampleStartModComboBox.getY (), columnThreeWidth /2, 20);
    sampleEndModLabel.setBounds (columnFourXOffset, sampleStartModComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    sampleEndModComboBox.setBounds (sampleEndModLabel.getX (), sampleEndModLabel.getBottom (), columnFourWidth / 2, 20);
    sampleEndModTextEditor.setBounds (sampleEndModComboBox.getRight () + 3, sampleEndModComboBox.getY (), columnFourWidth / 2, 20);
    xfadeGroupLabel.setBounds (columnFourXOffset, sampleEndModComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    xfadeGroupComboBox.setBounds (xfadeGroupLabel.getX (), xfadeGroupLabel.getBottom (), columnFourWidth, 20);
    zonesCVLabel.setBounds (columnFourXOffset, xfadeGroupComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    zonesCVComboBox.setBounds (zonesCVLabel.getX (), zonesCVLabel.getBottom (), columnFourWidth, 20);
    zonesRTLabel.setBounds (columnFourXOffset, zonesCVComboBox.getBottom () + 5, columnFourWidth, kMediumLabelIntSize);
    zonesRTComboBox.setBounds (zonesRTLabel.getX (), zonesRTLabel.getBottom (), columnFourWidth, 20);
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
    attackTextEditor.setText (FormatHelpers::formatDouble (attack, 4, false));
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
    autoTriggerComboBox.setSelectedId(autoTrigger ? 2 : 1, juce::NotificationType::dontSendNotification);
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
    releaseTextEditor.setText (FormatHelpers::formatDouble (release, 4, false));
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

