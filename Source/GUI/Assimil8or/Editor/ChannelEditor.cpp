#include "ChannelEditor.h"

ChannelEditor::ChannelEditor ()
{
//     juce::Label aliasingLabel;
//     juce::TextEditor aliasingTextEdit; // integer
//     juce::Label aliasingModLabel;
//     CvInputChannelComboBox aliasingModComboBox;
//     juce::TextEditor aliasingModTextEditor;
//     juce::Label attackLabel;
//     juce::TextEditor attackTextEditor; // double
//     juce::Label attackFromCurrentLabel;
//     juce::ToggleButton attackFromCurrentCheckBox; // false = start from zero, true = start from last value
//     juce::Label attackModLabel;
//     CvInputChannelComboBox attackModComboBox;
//     juce::TextEditor attackModTextEditor;
//     juce::Label autoTriggerLabel;
//     juce::ToggleButton autoTriggerCheckBox; //
//     juce::Label bitsLabel;
//     juce::TextEditor bitsTextEditor; // double
//     juce::Label bitsModLabel;
//     CvInputChannelComboBox bitsModComboBox;
//     juce::TextEditor bitsModTextEditor;
//     juce::Label channelModeLabel;
//     juce::ComboBox channelModeComboBox; // 4 Channel Modes: 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
//     juce::Label expAMLabel;
//     juce::TextEditor expAMTextEditor;
//     juce::Label expFMLabel;
//     juce::TextEditor expFMTextEditor;
//     juce::Label levelLabel;
//     juce::TextEditor LevelTextEditor;
//     juce::Label linAMLabel;
//     juce::TextEditor linAMTextEditor;
//     juce::Label linAMisExtEnvLabel;
//     juce::ToggleButton linAMisExtEnvCheckBox; // 
//     juce::Label linFMLabel;
//     juce::TextEditor linFMTextEditor;
//     juce::Label loopLengthModLabel;
//     CvInputChannelComboBox loopLengthModComboBox;
//     juce::TextEditor loopLengthModTextEditor;
//     juce::Label loopModeLabel;
//     juce::ComboBox loopModeComboBox; // 0 = No Loop, 1 = Loop, 2 = Loop and Release
//     juce::Label loopStartModLabel;
//     CvInputChannelComboBox loopStartModComboBox;
//     juce::TextEditor loopStartModTextEditor;
//     juce::Label mixLevelLabel;
//     juce::TextEditor mixLevelTextEditor;
//     juce::Label mixModLabel;
//     CvInputChannelComboBox mixModComboBox;
//     juce::TextEditor mixModTextEditor;
//     juce::Label mixModIsFaderLabel;
//     juce::ToggleButton mixModIsFaderCheckBox; // 
//     juce::Label panLabel;
//     juce::TextEditor panTextEditor;
//     juce::Label panModLabel;
//     CvInputChannelComboBox panModComboBox;
//     juce::TextEditor panModTextEditor;
//     juce::Label phaseCVLabel;
//     CvInputChannelComboBox phaseCVComboBox;
//     juce::TextEditor phaseCVTextEditor;
//     juce::Label pitchLabel;
//     juce::TextEditor pitchTextEditor;
//     juce::Label pitchCVLabel;
//     CvInputChannelComboBox pitchCVComboBox;
//     juce::TextEditor pitchCVTextEditor;
//     juce::Label playModeLabel;
//     juce::ComboBox playModeComboBox; // 2 Play Modes: 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
//     juce::Label pMIndexLabel;
//     juce::TextEditor pMIndexTextEditor;
//     juce::Label pMIndexModLabel;
//     CvInputChannelComboBox pMIndexModComboBox;
//     juce::TextEditor pMIndexModTextEditor;
//     juce::Label pMSourceLabel;
//     juce::ComboBox pMSourceComboBox; // Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
//     juce::Label releaseLabel;
//     juce::TextEditor releaseTextEditor;
//     juce::Label releaseModLabel;
//     CvInputChannelComboBox releaseModComboBox;
//     juce::TextEditor releaseModTextEditor;
//     juce::Label reverseLabel;
//     juce::ToggleButton reverseCheckBox; // 
//     juce::Label sampleEndModLabel;
//     CvInputChannelComboBox sampleEndModComboBox;
//     juce::TextEditor sampleEndModTextEditor;
//     juce::Label sampleStartModLabel;
//     CvInputChannelComboBox sampleStartModComboBox;
//     juce::TextEditor sampleStartModTextEditor;
//     juce::Label xfadeGroupLabel;
//     juce::ComboBox xfadeGroupComboBox; // Off, A, B, C, D
//     juce::Label zoneRTLabel;
//     juce::ComboBox zoneRTComboBox; // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
}

void ChannelEditor::init (juce::ValueTree channelPropertiesVT)
{
    channelProperties.wrap (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    setupChannelPropertiesCallbacks ();

    auto splitAndPassAsParams = [] (CvInputAndAmount amountAndCvInput, std::function<void(juce::String,double)> setter)
    {
        jassert (setter != nullptr);
        const auto& [cvInput, value] { amountAndCvInput };
        setter (cvInput, value);
    };
    aliasingDataChanged (channelProperties.getAliasing ());
    splitAndPassAsParams (channelProperties.getAliasingMod (), [this] (juce::String cvInput, double value) { aliasingModDataChanged (cvInput, value); });
}

void ChannelEditor::setupChannelPropertiesCallbacks ()
{
    channelProperties.onIndexChange = [this] ([[maybe_unused]] int index) { jassertfalse; /* I don't think this should change while we are editing */};
    channelProperties.onAliasingChange = [this] (int aliasing) { aliasingDataChanged (aliasing);  };
    channelProperties.onAliasingModChange = [this] (CvInputAndAmount amountAndCvInput) { const auto& [cvInput, value] { amountAndCvInput }; aliasingModDataChanged (cvInput, value); };
}

void ChannelEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::teal);
    g.drawRect (getLocalBounds ());
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
    attackFromCurrentCheckBox.setToggleState (attackFromCurrent, juce::NotificationType::dontSendNotification);
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
    autoTriggerCheckBox.setToggleState (autoTrigger, juce::NotificationType::dontSendNotification);
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

void ChannelEditor::expAMDataChanged (double expAM)
{
    expAMTextEditor.setText (juce::String (expAM));
}

void ChannelEditor::expAMUiChanged (double expAM)
{
    channelProperties.setExpAM (expAM, false);
}

void ChannelEditor::expFMDataChanged (double expFM)
{
    expFMTextEditor.setText (juce::String (expFM));
}

void ChannelEditor::expFMUiChanged (double expFM)
{
    channelProperties.setExpFM (expFM, false);
}

void ChannelEditor::levelDataChanged (double level)
{
    levelTextEditor.setText (juce::String (level));
}

void ChannelEditor::levelUiChanged (double level)
{
    channelProperties.setLevel (level, false);
}

void ChannelEditor::linAMDataChanged (double linAM)
{
    linAMTextEditor.setText (juce::String (linAM));
}

void ChannelEditor::linAMUiChanged (double linAM)
{
    channelProperties.setLinAM (linAM, false);
}

void ChannelEditor::linAMisExtEnvDataChanged (bool linAMisExtEnv)
{
    linAMisExtEnvCheckBox.setToggleState (linAMisExtEnv, juce::NotificationType::dontSendNotification);
}

void ChannelEditor::linAMisExtEnvUiChanged (bool linAMisExtEnv)
{
    channelProperties.setLinAMisExtEnv (linAMisExtEnv, false);
}

void ChannelEditor::linFMDataChanged (double linFM)
{
    linFMTextEditor.setText (juce::String (linFM));
}

void ChannelEditor::linFMUiChanged (double linFM)
{
    channelProperties.setLinFM (linFM, false);
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
    mixModIsFaderCheckBox.setToggleState (mixModIsFader, juce::NotificationType::dontSendNotification);
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
    reverseCheckBox.setToggleState (reverse, juce::NotificationType::dontSendNotification);
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
    spliceSmoothingCheckBox.setToggleState (spliceSmoothing, juce::NotificationType::dontSendNotification);
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

