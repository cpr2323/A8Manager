#include "ChannelEditor.h"

ChannelEditor::ChannelEditor ()
{
    zonesLabel.setText ("Zones", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (zonesLabel);

    for (auto curZoneIndex { 0 }; curZoneIndex < 8; ++curZoneIndex)
        zoneTabs.addTab (juce::String::charToString ('1' + curZoneIndex), juce::Colours::darkgrey, &zoneEditors [curZoneIndex], false);
    addAndMakeVisible (zoneTabs);

    channelModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    loopModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    playModeComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    pMSourceComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    xfadeGroupComboBox.setLookAndFeel (&noArrowComboBoxLnF);
    zoneRTComboBox.setLookAndFeel (&noArrowComboBoxLnF);

    setupChannelControls ();
}

ChannelEditor::~ChannelEditor ()
{
    channelModeComboBox.setLookAndFeel (nullptr);
    loopModeComboBox.setLookAndFeel (nullptr);
    playModeComboBox.setLookAndFeel (nullptr);
    pMSourceComboBox.setLookAndFeel (nullptr);
    xfadeGroupComboBox.setLookAndFeel (nullptr);
    zoneRTComboBox.setLookAndFeel (nullptr);
}

void ChannelEditor::setupChannelControls ()
{
    const auto textColor { juce::Colours::black };
    auto setupLabel = [this, textColor] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setColour (juce::Label::ColourIds::textColourId, textColor);
        label.setFont (label.getFont ().withHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this, textColor] (juce::TextEditor& textEditor, juce::Justification justification, std::function<void (juce::String)> textEditedCallback)
    {
        jassert (textEditedCallback != nullptr);
        textEditor.setJustification (justification);
        textEditor.setIndents (0, 0);
        textEditor.onFocusLost = [this, &textEditor, textEditedCallback] () { textEditedCallback (textEditor.getText ()); };
        textEditor.onReturnKey = [this, &textEditor, textEditedCallback] () { textEditedCallback (textEditor.getText ()); };
        addAndMakeVisible (textEditor);
    };

    // column one
    // PITCH
    setupLabel (pitchLabel, "PITCH", 25.0f, juce::Justification::centredTop);
    setupTextEditor (pitchTextEditor, juce::Justification::centred, [this] (juce::String text) { pitchUiChanged (text.getDoubleValue ()); });
    setupLabel (pitchSemiLabel, "SEMI", 15.0f, juce::Justification::centredLeft);
    pitchCVComboBox.onChange = [this] () { pitchCVUiChanged (pitchCVComboBox.getSelectedItemText (), pitchCVTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (pitchCVComboBox);
    setupTextEditor (pitchCVTextEditor, juce::Justification::centred, [this] (juce::String text) { pitchCVUiChanged (pitchCVComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // LINFM
    setupLabel (linFMLabel, "LINFM", 25.0f, juce::Justification::centredTop);
    linFMComboBox.onChange = [this] () { linFMUiChanged (linFMComboBox.getSelectedItemText (), linFMTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (linFMComboBox);
    setupTextEditor (linFMTextEditor, juce::Justification::centred, [this] (juce::String text) { linFMUiChanged (linFMComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // EXPFM
    setupLabel (expFMLabel, "EXPFM", 25.0f, juce::Justification::centredTop);
    expFMComboBox.onChange = [this] () { expFMUiChanged (expFMComboBox.getSelectedItemText (), expFMTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (expFMComboBox);
    setupTextEditor (expFMTextEditor, juce::Justification::centred, [this] (juce::String text) { expFMUiChanged (expFMComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // LEVEL
    setupLabel (levelLabel, "LEVEL", 25.0f, juce::Justification::centredTop);
    setupTextEditor (levelTextEditor, juce::Justification::centred, [this] (juce::String text) { levelUiChanged (text.getDoubleValue ()); });
    setupLabel (levelDbLabel, "dB", 15.0f, juce::Justification::centredLeft);

    // LINAM
    setupLabel (linAMLabel, "LINAM", 25.0f, juce::Justification::centredTop);
    linAMComboBox.onChange = [this] () { linAMUiChanged (linAMComboBox.getSelectedItemText (), linAMTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (linAMComboBox);
    setupTextEditor (linAMTextEditor, juce::Justification::centred, [this] (juce::String text) { linAMUiChanged (linAMComboBox.getSelectedItemText (), text.getDoubleValue ()); });
    linAMisExtEnvButton.setButtonText ("EXT");
    linAMisExtEnvButton.setClickingTogglesState (true);
    linAMisExtEnvButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, linAMisExtEnvButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
    linAMisExtEnvButton.onClick = [this] () { linAMisExtEnvUiChanged (linAMisExtEnvButton.getToggleState ()); };
    addAndMakeVisible (linAMisExtEnvButton);

    // EXPAM
    setupLabel (expAMLabel, "EXPAM", 25.0f, juce::Justification::centredTop);
    expAMComboBox.onChange = [this] () { expAMUiChanged (expAMComboBox.getSelectedItemText (), expAMTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (expAMComboBox);
    setupTextEditor (expAMTextEditor, juce::Justification::centred, [this] (juce::String text) { expAMUiChanged (expAMComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // column two
    // PHASE MOD SOURCE
    setupLabel (phaseCVLabel, "PHASE MOD", 25.0f, juce::Justification::centredTop);
    // PM Source Index - Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
    for (auto pmSourceIndex { 0 }; pmSourceIndex < 8; ++pmSourceIndex)
    {
        pMSourceComboBox.addItem ("Channel " + juce::String::charToString ('1' + pmSourceIndex), pmSourceIndex + 1);
    }
    pMSourceComboBox.addItem ("Right Input", 9);
    pMSourceComboBox.addItem ("Left Input", 10);
    pMSourceComboBox.addItem ("Phase CV", 11);
    pMSourceComboBox.onChange = [this] () { pMSourceUiChanged (pMSourceComboBox.getSelectedId () - 1); };
    addAndMakeVisible (pMSourceComboBox);
    setupLabel (pMSourceLabel, "SOURCE", 15.0f, juce::Justification::centredLeft);
    phaseCVComboBox.onChange = [this] () { phaseCVUiChanged (phaseCVComboBox.getSelectedItemText (), phaseCVTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (phaseCVComboBox);

    // PHASE MOD INDEX
    setupTextEditor (phaseCVTextEditor, juce::Justification::centred, [this] (juce::String text) { phaseCVUiChanged (phaseCVComboBox.getSelectedItemText (), text.getDoubleValue ()); });
    setupLabel (pMIndexLabel, "PHASE MOD", 25.0f, juce::Justification::centredTop);
    setupTextEditor (pMIndexTextEditor, juce::Justification::centred, [this] (juce::String text) { pMIndexUiChanged (text.getDoubleValue ()); });
    setupLabel (pMIndexModLabel, "INDEX", 15.0f, juce::Justification::centredLeft);
    pMIndexModComboBox.onChange = [this] () { pMIndexModUiChanged (pMIndexModComboBox.getSelectedItemText (), pMIndexModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (pMIndexModComboBox);
    setupTextEditor (pMIndexModTextEditor, juce::Justification::centred, [this] (juce::String text) { pMIndexModUiChanged (pMIndexModComboBox.getSelectedItemText (), text.getDoubleValue ()); });

   // PAN/MIX
    setupLabel (panMixLabel, "PAN/MIX", 25.0f, juce::Justification::centredTop);
    setupTextEditor (panTextEditor, juce::Justification::centred, [this] (juce::String text) { panUiChanged (text.getDoubleValue ()); });
    setupLabel (panLabel, "PAN", 15.0f, juce::Justification::centredLeft);
    panModComboBox.onChange = [this] () { panModUiChanged (panModComboBox.getSelectedItemText (), panModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (panModComboBox);
    setupTextEditor (panModTextEditor, juce::Justification::centred, [this] (juce::String text) { panModUiChanged (panModComboBox.getSelectedItemText (), text.getDoubleValue ()); });
    setupTextEditor (mixLevelTextEditor, juce::Justification::centred, [this] (juce::String text) { mixLevelUiChanged (text.getDoubleValue ()); });
    setupLabel (mixLevelLabel, "MIX", 15.0f, juce::Justification::centredLeft);
    mixModComboBox.onChange = [this] () { mixModUiChanged (mixModComboBox.getSelectedItemText (), mixModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (mixModComboBox);
    setupTextEditor (mixModTextEditor, juce::Justification::centred, [this] (juce::String text) { mixModUiChanged (panModComboBox.getSelectedItemText (), text.getDoubleValue ()); });
    mixModIsFaderButton.setButtonText ("FADER");
    mixModIsFaderButton.setClickingTogglesState (true);
    mixModIsFaderButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, mixModIsFaderButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
    mixModIsFaderButton.onClick = [this] () { mixModIsFaderUiChanged (mixModIsFaderButton.getToggleState ()); };
    addAndMakeVisible (mixModIsFaderButton);

    // column three
    setupLabel (mutateLabel, "MUTATE", 25.0f, juce::Justification::centredTop);

    // BITS
    setupTextEditor (bitsTextEditor, juce::Justification::centred, [this] (juce::String text) { bitsUiChanged (text.getDoubleValue ()); });
    setupLabel (bitsLabel, "BITS", 15.0f, juce::Justification::centredLeft);
    bitsModComboBox.onChange = [this] () { bitsModUiChanged (bitsModComboBox.getSelectedItemText (), bitsModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (bitsModComboBox);
    setupTextEditor (bitsModTextEditor, juce::Justification::centred, [this] (juce::String text) { bitsModUiChanged (bitsModComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // ALIASING
    setupTextEditor (aliasingTextEditor, juce::Justification::centred, [this] (juce::String text) { aliasingUiChanged (text.getIntValue ()); });
    setupLabel (aliasingLabel, "ALIAS", 15.0f, juce::Justification::centredLeft);
    aliasingModComboBox.onChange = [this] () { aliasingModUiChanged (aliasingModComboBox.getSelectedItemText (), aliasingModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (aliasingModComboBox);
    setupTextEditor (aliasingModTextEditor, juce::Justification::centred, [this] (juce::String text) { aliasingModUiChanged (aliasingModComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // REVERSE/SMOOTH
    reverseButton.setButtonText ("REV");
    reverseButton.setClickingTogglesState (true);
    reverseButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, reverseButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
    reverseButton.onClick = [this] () { reverseUiChanged (reverseButton.getToggleState ()); };
    addAndMakeVisible (reverseButton);
    spliceSmoothingButton.setButtonText ("SMOOTH");
    spliceSmoothingButton.setClickingTogglesState (true);
    spliceSmoothingButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, spliceSmoothingButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
    spliceSmoothingButton.onClick = [this] () { reverseUiChanged (spliceSmoothingButton.getToggleState ()); };
    addAndMakeVisible (spliceSmoothingButton);

    // ENVELOPE
    setupLabel (envelopeLabel, "ENVELOPE", 25.0f, juce::Justification::centredTop);
    // ATTACK
    setupTextEditor (attackTextEditor, juce::Justification::centred, [this] (juce::String text) { attackUiChanged (text.getDoubleValue ()); });
    setupLabel (attackLabel, "ATTACK", 15.0f, juce::Justification::centredLeft);
    attackModComboBox.onChange = [this] () { attackModUiChanged (attackModComboBox.getSelectedItemText (), attackModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (attackModComboBox);
    setupTextEditor (attackModTextEditor, juce::Justification::centred, [this] (juce::String text) { attackModUiChanged (attackModComboBox.getSelectedItemText (), text.getDoubleValue ()); });
    attackFromCurrentButton.setButtonText ("CURRENT");
    attackFromCurrentButton.setClickingTogglesState (true);
    attackFromCurrentButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, attackFromCurrentButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
    attackFromCurrentButton.onClick = [this] () { attackFromCurrentUiChanged (attackFromCurrentButton.getToggleState ()); };
    addAndMakeVisible (attackFromCurrentButton);
    // RELEASE
    setupTextEditor (releaseTextEditor, juce::Justification::centred, [this] (juce::String text) { releaseUiChanged (text.getDoubleValue ()); });
    setupLabel (releaseLabel, "RELEASE", 15.0f, juce::Justification::centredLeft);
    releaseModComboBox.onChange = [this] () { releaseModUiChanged (releaseModComboBox.getSelectedItemText (), releaseModTextEditor.getText ().getDoubleValue ()); };
    addAndMakeVisible (releaseModComboBox);
    setupTextEditor (releaseModTextEditor, juce::Justification::centred, [this] (juce::String text) { releaseModUiChanged (releaseModComboBox.getSelectedItemText (), text.getDoubleValue ()); });

    // PLAY/LOOP/AUTO TRIGGER
    setupLabel (playModeLabel, "PLAY", 15.0f, juce::Justification::centred);
    playModeComboBox.addItem ("Gated", 1); // 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
    playModeComboBox.addItem ("One Shot", 2);
    playModeComboBox.onChange = [this] () { playModeUiChanged (playModeComboBox.getSelectedId () - 1); };
    addAndMakeVisible (playModeComboBox);

    setupLabel (loopModeLabel, "LOOP", 15.0f, juce::Justification::centred);
    loopModeComboBox.addItem ("No Loop", 1); // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    loopModeComboBox.addItem ("Loop", 2);
    loopModeComboBox.addItem ("Loop/Release", 3);
    loopModeComboBox.onChange = [this] () { loopModeUiChanged (loopModeComboBox.getSelectedId () - 1); };
    addAndMakeVisible (loopModeComboBox);

    setupLabel (autoTriggerLabel, "TRIGGER", 15.0f, juce::Justification::centred);

    autoTriggerButton.setButtonText ("AUTO");
    autoTriggerButton.setClickingTogglesState (true);
    autoTriggerButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, autoTriggerButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
    autoTriggerButton.onClick = [this] () { attackFromCurrentUiChanged (autoTriggerButton.getToggleState ()); };
    addAndMakeVisible (autoTriggerButton);

    // column four
    // juce::Label channelModeLabel;
    // juce::ComboBox channelModeComboBox; // 4 Channel Modes: 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    // juce::Label loopLengthModLabel;
    // CvInputChannelComboBox loopLengthModComboBox;
    // juce::TextEditor loopLengthModTextEditor;
    // juce::Label loopStartModLabel;
    // CvInputChannelComboBox loopStartModComboBox;
    // juce::TextEditor loopStartModTextEditor;
    // juce::Label sampleEndModLabel;
    // CvInputChannelComboBox sampleEndModComboBox;
    // juce::TextEditor sampleEndModTextEditor;
    // juce::Label sampleStartModLabel;
    // CvInputChannelComboBox sampleStartModComboBox;
    // juce::TextEditor sampleStartModTextEditor;
    // juce::Label xfadeGroupLabel;
    // juce::ComboBox xfadeGroupComboBox; // Off, A, B, C, D
    // juce::Label zonesCVLabel;
    // CvInputChannelComboBox zonesCVComboBox; // 0A - 8C
    // juce::Label zoneRTLabel;
    // juce::ComboBox zoneRTComboBox; // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
}


void ChannelEditor::init (juce::ValueTree channelPropertiesVT)
{
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    setupChannelPropertiesCallbacks ();
    auto zoneEditorIndex { 0 };
    channelProperties.forEachZone([this, &zoneEditorIndex] (juce::ValueTree zonePropertiesVT)
    {
        zoneEditors [zoneEditorIndex].init (zonePropertiesVT);
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

void ChannelEditor::paint ([[maybe_unused]] juce::Graphics& g)
{
//     g.setColour (juce::Colours::teal);
//     g.drawRect (getLocalBounds ());
}

void ChannelEditor::resized ()
{
    auto zoneBounds {getLocalBounds ().removeFromRight(getWidth () / 4)};
    zonesLabel.setBounds (zoneBounds.removeFromTop (20));
    zoneBounds.removeFromTop (3);
    zoneTabs.setBounds (zoneBounds);

    // column one
    const auto columnOneXOffset { 5 };
    const auto columnOneWidth { 80 };
    pitchLabel.setBounds (columnOneXOffset, 5, columnOneWidth, 25);
    pitchTextEditor.setBounds (pitchLabel.getX () + 3, pitchLabel.getBottom () + 3, pitchLabel.getWidth () - 6, 20);
    pitchSemiLabel.setBounds (pitchTextEditor.getRight () + 3, pitchTextEditor.getY (), 40, 20);
    pitchCVComboBox.setBounds (pitchLabel.getX (), pitchTextEditor.getBottom () + 3, (pitchLabel.getWidth () / 2) - 2, 20);
    pitchCVTextEditor.setBounds (pitchCVComboBox.getRight () + 3, pitchCVComboBox.getY (), pitchLabel.getWidth () - (pitchLabel.getWidth () / 2) - 1, 20);

    linFMLabel.setBounds (columnOneXOffset, pitchCVComboBox.getBottom() + 5, columnOneWidth, 25);
    linFMComboBox.setBounds (linFMLabel.getX (), linFMLabel.getBottom () + 3, (linFMLabel.getWidth () / 2) - 2, 20);
    linFMTextEditor.setBounds (linFMComboBox.getRight () + 3, linFMComboBox.getY (), linFMLabel.getWidth () - (linFMLabel.getWidth () / 2) - 1, 20);

    expFMLabel.setBounds (columnOneXOffset, linFMComboBox.getBottom () + 5, columnOneWidth, 25);
    expFMComboBox.setBounds (expFMLabel.getX (), expFMLabel.getBottom () + 3, (expFMLabel.getWidth () / 2) - 2, 20);
    expFMTextEditor.setBounds (expFMComboBox.getRight () + 3, expFMComboBox.getY (), expFMLabel.getWidth () - (expFMLabel.getWidth () / 2) - 1, 20);

    levelLabel.setBounds (columnOneXOffset, expFMComboBox.getBottom () + 5, columnOneWidth, 25);
    levelTextEditor.setBounds (levelLabel.getX () + 3, levelLabel.getBottom () + 3, levelLabel.getWidth () - 6, 20);
    levelDbLabel.setBounds (levelTextEditor.getRight () + 3, levelTextEditor.getY (), 40, 20);

    linAMLabel.setBounds (columnOneXOffset, levelTextEditor.getBottom () + 5, columnOneWidth, 25);
    linAMComboBox.setBounds (linAMLabel.getX (), linAMLabel.getBottom () + 3, (linAMLabel.getWidth () / 2) - 2, 20);
    linAMTextEditor.setBounds (linAMComboBox.getRight () + 3, linAMComboBox.getY (), linAMLabel.getWidth () - (linAMLabel.getWidth () / 2) - 1, 20);
    linAMisExtEnvButton.setBounds (linAMTextEditor.getRight () + 3, linAMTextEditor.getY (), 30, 20);

    expAMLabel.setBounds (columnOneXOffset, linAMComboBox.getBottom () + 5, columnOneWidth, 25);
    expAMComboBox.setBounds (expAMLabel.getX (), expAMLabel.getBottom () + 3, (expAMLabel.getWidth () / 2) - 2, 20);
    expAMTextEditor.setBounds (expAMComboBox.getRight () + 3, expAMComboBox.getY (), expAMLabel.getWidth () - (expAMLabel.getWidth () / 2) - 1, 20);

    //column two
    const auto columnTwoXOffset { columnOneXOffset + 130 };
    const auto columnTwoWidth { 100 };
    phaseCVLabel.setBounds (columnTwoXOffset, 5, columnTwoWidth, 25);
    pMSourceComboBox.setBounds (phaseCVLabel.getX () + 3, phaseCVLabel.getBottom () + 3, phaseCVLabel.getWidth () - 6, 20);
    pMSourceLabel.setBounds (pMSourceComboBox.getRight () + 3, pMSourceComboBox.getY (), 40, 20);
    phaseCVComboBox.setBounds (phaseCVLabel.getX (), pMSourceComboBox.getBottom () + 3, (phaseCVLabel.getWidth () / 2) - 2, 20);
    phaseCVTextEditor.setBounds (phaseCVComboBox.getRight () + 3, phaseCVComboBox.getY (), phaseCVLabel.getWidth () - (phaseCVLabel.getWidth () / 2) - 1, 20);

    pMIndexLabel.setBounds (columnTwoXOffset, phaseCVTextEditor.getBottom () + 5, columnTwoWidth, 25);
    pMIndexTextEditor.setBounds (pMIndexLabel.getX () + 3, pMIndexLabel.getBottom () + 3, pMIndexLabel.getWidth () - 6, 20);
    pMIndexModLabel.setBounds (pMIndexTextEditor.getRight () + 3, pMIndexTextEditor.getY (), 40, 20);
    pMIndexModComboBox.setBounds (pMIndexLabel.getX (), pMIndexTextEditor.getBottom () + 3, (pMIndexLabel.getWidth () / 2) - 2, 20);
    pMIndexModTextEditor.setBounds (pMIndexModComboBox.getRight () + 3, pMIndexModComboBox.getY (), pMIndexLabel.getWidth () - (pMIndexLabel.getWidth () / 2) - 1, 20);

    panMixLabel.setBounds (columnTwoXOffset, pMIndexModTextEditor.getBottom () + 5, columnTwoWidth, 25);
    panTextEditor.setBounds (panMixLabel.getX () + 3, panMixLabel.getBottom () + 3, panMixLabel.getWidth () - 6, 20);
    panLabel.setBounds (panTextEditor.getRight () + 3, panTextEditor.getY (), 40, 20);
    panModComboBox.setBounds (panMixLabel.getX (), panTextEditor.getBottom () + 3, (panMixLabel.getWidth () / 2) - 2, 20);
    panModTextEditor.setBounds (panModComboBox.getRight () + 3, panModComboBox.getY (), panMixLabel.getWidth () - (panMixLabel.getWidth () / 2) - 1, 20);

    mixLevelTextEditor.setBounds (panMixLabel.getX () + 3, panModComboBox.getBottom () + 5, panMixLabel.getWidth () - 6, 20);
    mixLevelLabel.setBounds (mixLevelTextEditor.getRight () + 3, mixLevelTextEditor.getY (), 40, 20);
    mixModComboBox.setBounds (panMixLabel.getX (), mixLevelTextEditor.getBottom () + 3, (panMixLabel.getWidth () / 2) - 2, 20);
    mixModTextEditor.setBounds (mixModComboBox.getRight () + 3, mixModComboBox.getY (), panMixLabel.getWidth () - (panMixLabel.getWidth () / 2) - 1, 20);
    mixModIsFaderButton.setBounds (mixModTextEditor.getRight () + 3, mixModTextEditor.getY (), 40, 20);

    // column three
    const auto columnThreeXOffset { columnTwoXOffset + 160 };
    const auto columnThreeWidth { 100 };

    // MUTATE
    mutateLabel.setBounds (columnThreeXOffset, 5, columnThreeWidth, 25);
    // BITS
    bitsTextEditor.setBounds (mutateLabel.getX () + 3, mutateLabel.getBottom () + 3, mutateLabel.getWidth () - 6, 20);
    bitsLabel.setBounds (bitsTextEditor.getRight () + 3, bitsTextEditor.getY (), 40, 20);
    bitsModComboBox.setBounds (mutateLabel.getX (), bitsTextEditor.getBottom () + 3, (mutateLabel.getWidth () / 2) - 2, 20);
    bitsModTextEditor.setBounds (bitsModComboBox.getRight () + 3, bitsModComboBox.getY (), mutateLabel.getWidth () - (mutateLabel.getWidth () / 2) - 1, 20);
    // ALIASING
    aliasingTextEditor.setBounds (mutateLabel.getX () + 3, bitsModComboBox.getBottom () + 5, mutateLabel.getWidth () - 6, 20);
    aliasingLabel.setBounds (aliasingTextEditor.getRight () + 3, aliasingTextEditor.getY (), 40, 20);
    aliasingModComboBox.setBounds (mutateLabel.getX (), aliasingTextEditor.getBottom () + 3, (mutateLabel.getWidth () / 2) - 2, 20);
    aliasingModTextEditor.setBounds (aliasingModComboBox.getRight () + 3, aliasingModComboBox.getY (), mutateLabel.getWidth () - (mutateLabel.getWidth () / 2) - 1, 20);

    // REVERSE/SMOOTH
    reverseButton.setBounds (mutateLabel.getX (), aliasingModTextEditor.getBottom () + 5, mutateLabel.getWidth () / 2 - 6, 20);
    spliceSmoothingButton.setBounds (reverseButton.getRight () + 3, aliasingModTextEditor.getBottom () + 5, mutateLabel.getWidth () - mutateLabel.getWidth () / 2 + 2, 20);

    envelopeLabel.setBounds (columnThreeXOffset, reverseButton.getBottom () + 5, columnThreeWidth, 25);
    // ATTACK
    attackTextEditor.setBounds (envelopeLabel.getX () + 3, envelopeLabel.getBottom () + 3, envelopeLabel.getWidth () - 6, 20);
    attackLabel.setBounds (attackTextEditor.getRight () + 3, attackTextEditor.getY (), 40, 20);
    attackModComboBox.setBounds (envelopeLabel.getX (), attackTextEditor.getBottom () + 3, (envelopeLabel.getWidth () / 2) - 2, 20);
    attackModTextEditor.setBounds (attackModComboBox.getRight () + 3, attackModComboBox.getY (), envelopeLabel.getWidth () - (envelopeLabel.getWidth () / 2) - 1, 20);
    attackFromCurrentButton.setBounds (attackModTextEditor.getRight () + 3, attackModTextEditor.getY (), 50, 20);

    // RELEASE
    releaseTextEditor.setBounds (envelopeLabel.getX () + 3, attackModComboBox.getBottom () + 5, envelopeLabel.getWidth () - 6, 20);
    releaseLabel.setBounds (releaseTextEditor.getRight () + 3, releaseTextEditor.getY (), 40, 20);
    releaseModComboBox.setBounds (envelopeLabel.getX (), releaseTextEditor.getBottom () + 3, (envelopeLabel.getWidth () / 2) - 2, 20);
    releaseModTextEditor.setBounds (releaseModComboBox.getRight () + 3, releaseModComboBox.getY (), envelopeLabel.getWidth () - (envelopeLabel.getWidth () / 2) - 1, 20);

    // AUTO TRIGGER
    autoTriggerLabel.setBounds (releaseModComboBox.getX (), releaseModComboBox.getBottom () + 5, ((columnThreeWidth / 3) * 2) - 15, 20);
    autoTriggerButton.setBounds (autoTriggerLabel.getRight () + 3, autoTriggerLabel.getY () + 3, (columnThreeWidth / 3 + 15), 20);

    // PLAY MODE
    playModeLabel.setBounds (autoTriggerLabel.getX (), autoTriggerLabel.getBottom () + 3, columnThreeWidth / 3, 20);
    playModeComboBox.setBounds (playModeLabel.getRight (), playModeLabel.getY () + 3, (columnThreeWidth / 3) * 2, 20);

    // LOOP MODE
    loopModeLabel.setBounds (playModeLabel.getX (), playModeLabel.getBottom () + 3, columnThreeWidth / 3, 20);
    loopModeComboBox.setBounds (loopModeLabel.getRight (), loopModeLabel.getY () + 3, (columnThreeWidth / 3) * 2, 20);

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
    aliasingModTextEditor.setText (juce::String (aliasingMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::aliasingModUiChanged (juce::String cvInput, double aliasingMod)
{
    channelProperties.setAliasingMod (cvInput, aliasingMod, false);
}

void ChannelEditor::attackDataChanged (double attack)
{
    attackTextEditor.setText (juce::String (attack));
}

void ChannelEditor::attackUiChanged (double attack)
{
    channelProperties.setAttack (attack, false);
}

void ChannelEditor::attackFromCurrentDataChanged (bool attackFromCurrent)
{
    attackFromCurrentButton.setToggleState (attackFromCurrent, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::attackFromCurrentUiChanged (bool attackFromCurrent)
{
    channelProperties.setAttackFromCurrent (attackFromCurrent, false);
}

void ChannelEditor::attackModDataChanged (juce::String cvInput, double attackMod)
{
    attackModComboBox.setSelectedItemText (cvInput);
    attackModTextEditor.setText (juce::String (attackMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::attackModUiChanged (juce::String cvInput, double attackMod)
{
    channelProperties.setAttackMod (cvInput, attackMod, false);
}

void ChannelEditor::autoTriggerDataChanged (bool autoTrigger)
{
    autoTriggerButton.setToggleState (autoTrigger, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::autoTriggerUiChanged (bool autoTrigger)
{
    channelProperties.setAutoTrigger (autoTrigger, false);
}

void ChannelEditor::bitsDataChanged (double bits)
{
    bitsTextEditor.setText (juce::String (bits));
}

void ChannelEditor::bitsUiChanged (double bits)
{
    channelProperties.setBits (bits, false);
}

void ChannelEditor::bitsModDataChanged (juce::String cvInput, double bitsMod)
{
    bitsModComboBox.setSelectedItemText (cvInput);
    bitsModTextEditor.setText (juce::String (bitsMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::bitsModUiChanged (juce::String cvInput, double bitsMod)
{
    channelProperties.setBitsMod (cvInput, bitsMod, false);
}

void ChannelEditor::channelModeDataChanged (int channelMode)
{
    channelModeComboBox.setSelectedItemIndex (channelMode, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::channelModeUiChanged (int channelMode)
{
    channelProperties.setChannelMode (channelMode, false);
}

void ChannelEditor::expAMDataChanged (juce::String cvInput, double expAM)
{
    expAMComboBox.setSelectedItemText (cvInput);
    expAMTextEditor.setText (juce::String (expAM));
}

void ChannelEditor::expAMUiChanged (juce::String cvInput, double expAM)
{
    channelProperties.setExpAM (cvInput, expAM, false);
}

void ChannelEditor::expFMDataChanged (juce::String cvInput, double expFM)
{
    expFMComboBox.setSelectedItemText (cvInput);
    expFMTextEditor.setText (juce::String (expFM));
}

void ChannelEditor::expFMUiChanged (juce::String cvInput, double expFM)
{
    channelProperties.setExpFM (cvInput, expFM, false);
}

void ChannelEditor::levelDataChanged (double level)
{
    levelTextEditor.setText (juce::String (level));
}

void ChannelEditor::levelUiChanged (double level)
{
    channelProperties.setLevel (level, false);
}

void ChannelEditor::linAMDataChanged (juce::String cvInput, double linAM)
{
    linAMComboBox.setSelectedItemText (cvInput);
    linAMTextEditor.setText (juce::String (linAM));
}

void ChannelEditor::linAMUiChanged (juce::String cvInput, double linAM)
{
    channelProperties.setLinAM (cvInput, linAM, false);
}

void ChannelEditor::linAMisExtEnvDataChanged (bool linAMisExtEnv)
{
    linAMisExtEnvButton.setToggleState (linAMisExtEnv, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::linAMisExtEnvUiChanged (bool linAMisExtEnv)
{
    channelProperties.setLinAMisExtEnv (linAMisExtEnv, false);
}

void ChannelEditor::linFMDataChanged (juce::String cvInput, double linFM)
{
    linFMComboBox.setSelectedItemText (cvInput);
    linFMTextEditor.setText (juce::String (linFM));
}

void ChannelEditor::linFMUiChanged (juce::String cvInput, double linFM)
{
    channelProperties.setLinFM (cvInput, linFM, false);
}

void ChannelEditor::loopLengthModDataChanged (juce::String cvInput, double loopLengthMod)
{
    loopLengthModComboBox.setSelectedItemText (cvInput);
    loopLengthModTextEditor.setText (juce::String (loopLengthMod), juce::NotificationType::dontSendNotification);
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
    loopStartModTextEditor.setText (juce::String (loopStartMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::loopStartModUiChanged (juce::String cvInput, double loopStartMod)
{
    channelProperties.setLoopStartMod (cvInput, loopStartMod, false);
}

void ChannelEditor::mixLevelDataChanged (double mixLevel)
{
    mixLevelTextEditor.setText (juce::String (mixLevel));
}

void ChannelEditor::mixLevelUiChanged (double mixLevel)
{
    channelProperties.setMixLevel (mixLevel, false);
}

void ChannelEditor::mixModDataChanged (juce::String cvInput, double mixMod)
{
    mixModComboBox.setSelectedItemText (cvInput);
    mixModTextEditor.setText (juce::String (mixMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::mixModUiChanged (juce::String cvInput, double mixMod)
{
    channelProperties.setMixMod (cvInput, mixMod, false);
}

void ChannelEditor::mixModIsFaderDataChanged (bool mixModIsFader)
{
    mixModIsFaderButton.setToggleState (mixModIsFader, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::mixModIsFaderUiChanged (bool mixModIsFader)
{
    channelProperties.setMixModIsFader (mixModIsFader, false);
}

void ChannelEditor::panDataChanged (double pan)
{
    panTextEditor.setText (juce::String (pan));
}

void ChannelEditor::panUiChanged (double pan)
{
    channelProperties.setPan (pan, false);
}

void ChannelEditor::panModDataChanged (juce::String cvInput, double panMod)
{
    panModComboBox.setSelectedItemText (cvInput);
    panModTextEditor.setText (juce::String (panMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::panModUiChanged (juce::String cvInput, double panMod)
{
    channelProperties.setPanMod (cvInput, panMod, false);
}

void ChannelEditor::phaseCVDataChanged (juce::String cvInput, double phaseCV)
{
    phaseCVComboBox.setSelectedItemText (cvInput);
    phaseCVTextEditor.setText (juce::String (phaseCV), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::phaseCVUiChanged (juce::String cvInput, double phaseCV)
{
    channelProperties.setPhaseCV (cvInput, phaseCV, false);
}

void ChannelEditor::pitchDataChanged (double pitch)
{
    pitchTextEditor.setText (juce::String (pitch));
}

void ChannelEditor::pitchUiChanged (double pitch)
{
    channelProperties.setPitch (pitch, false);
}

void ChannelEditor::pitchCVDataChanged (juce::String cvInput, double pitchCV)
{
    pitchCVComboBox.setSelectedItemText (cvInput);
    pitchCVTextEditor.setText (juce::String (pitchCV), juce::NotificationType::dontSendNotification);
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
    pMIndexTextEditor.setText (juce::String (pMIndex));
}

void ChannelEditor::pMIndexUiChanged (double pMIndex)
{
    channelProperties.setPMIndex (pMIndex, false);
}

void ChannelEditor::pMIndexModDataChanged (juce::String cvInput, double pMIndexMod)
{
    pMIndexModComboBox.setSelectedItemText (cvInput);
    pMIndexModTextEditor.setText (juce::String (pMIndexMod), juce::NotificationType::dontSendNotification);
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
    releaseTextEditor.setText (juce::String (release));
}

void ChannelEditor::releaseUiChanged (double release)
{
    channelProperties.setRelease (release, false);
}

void ChannelEditor::releaseModDataChanged (juce::String cvInput, double releaseMod)
{
    releaseModComboBox.setSelectedItemText (cvInput);
    releaseModTextEditor.setText (juce::String (releaseMod), juce::NotificationType::dontSendNotification);
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
    sampleEndModTextEditor.setText (juce::String (sampleEndMod), juce::NotificationType::dontSendNotification);
}

void ChannelEditor::sampleEndModUiChanged (juce::String cvInput, double sampleEndMod)
{
    channelProperties.setSampleEndMod (cvInput, sampleEndMod, false);
}

void ChannelEditor::sampleStartModDataChanged (juce::String cvInput, double sampleStartMod)
{
    sampleStartModComboBox.setSelectedItemText (cvInput);
    sampleStartModTextEditor.setText (juce::String (sampleStartMod), juce::NotificationType::dontSendNotification);
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
    zoneRTComboBox.setSelectedItemIndex (zonesRT, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::zonesRTUiChanged (int zonesRT)
{
    channelProperties.setZonesRT (zonesRT, false);
}

