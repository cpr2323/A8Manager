#pragma once

#include <JuceHeader.h>
#include "CvInputComboBox.h"
#include "ZoneEditor.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"

class ChannelEditor : public juce::Component
{
public:
private:
    ChannelProperties channelProperties;

    juce::Label aliasingLabel;
    juce::TextEditor aliasingTextEdit; // integer
    juce::Label aliasingModLabel;
    CvInputChannelComboBox aliasingModComboBox;
    juce::TextEditor aliasingModTextEditor; 
    juce::Label attackLabel;
    juce::TextEditor attackTextEditor; // double
    juce::Label attackFromCurrentLabel;
    juce::ToggleButton attackFromCurrentCheckBox; // false = start from zero, true = start from last value
    juce::Label attackModLabel;
    CvInputChannelComboBox attackModComboBox;
    juce::TextEditor attackModTextEditor;
    juce::Label autoTriggerLabel;
    juce::ToggleButton autoTriggerCheckBox; //
    juce::Label bitsLabel;
    juce::TextEditor bitsTextEditor; // double
    juce::Label bitsModLabel;
    CvInputChannelComboBox bitsModComboBox;
    juce::TextEditor bitsModTextEditor;
    juce::Label channelModeLabel;
    juce::ComboBox channelModeComboBox; // 4 Channel Modes: 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    juce::Label expAMLabel;
    juce::TextEditor expAMTextEditor;
    juce::Label expFMLabel;
    juce::TextEditor expFMTextEditor;
    juce::Label levelLabel;
    juce::TextEditor LevelTextEditor;
    juce::Label linAMLabel;
    juce::TextEditor linAMTextEditor;
    juce::Label linAMisExtEnvLabel;
    juce::ToggleButton linAMisExtEnvCheckBox; // 
    juce::Label linFMLabel;
    juce::TextEditor linFMTextEditor;
    juce::Label loopLengthModLabel;
    CvInputChannelComboBox loopLengthModComboBox;
    juce::TextEditor loopLengthModTextEditor;
    juce::Label loopModeLabel;
    juce::ComboBox loopModeComboBox; // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    juce::Label loopStartModLabel;
    CvInputChannelComboBox loopStartModComboBox;
    juce::TextEditor loopStartModTextEditor;
    juce::Label mixLevelLabel;
    juce::TextEditor mixLevelTextEditor;
    juce::Label mixModLabel;
    CvInputChannelComboBox mixModComboBox;
    juce::TextEditor mixModTextEditor;
    juce::Label mixModIsFaderLabel;
    juce::ToggleButton mixModIsFaderCheckBox; // 
    juce::Label panLabel;
    juce::TextEditor panTextEditor;
    juce::Label panModLabel;
    CvInputChannelComboBox panModComboBox;
    juce::TextEditor panModTextEditor;
    juce::Label phaseCVLabel;
    CvInputChannelComboBox phaseCVComboBox;
    juce::TextEditor phaseCVTextEditor;
    juce::Label pitchLabel;
    juce::TextEditor pitchTextEditor;
    juce::Label pitchCVLabel;
    CvInputChannelComboBox pitchCVComboBox;
    juce::TextEditor pitchCVTextEditor;
    juce::Label playModeLabel;
    juce::ComboBox playModeComboBox; // 2 Play Modes: 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
    juce::Label pMIndexLabel;
    juce::TextEditor pMIndexTextEditor;
    juce::Label pMIndexModLabel;
    CvInputChannelComboBox pMIndexModComboBox;
    juce::TextEditor pMIndexModTextEditor;
    juce::Label pMSourceLabel;
    juce::ComboBox pMSourceComboBox; // Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
    juce::Label releaseLabel;
    juce::TextEditor releaseTextEditor;
    juce::Label releaseModLabel;
    CvInputChannelComboBox releaseModComboBox;
    juce::TextEditor releaseModTextEditor;
    juce::Label reverseLabel;
    juce::ToggleButton reverseCheckBox; // 
    juce::Label sampleEndModLabel;
    CvInputChannelComboBox sampleEndModComboBox;
    juce::TextEditor sampleEndModTextEditor;
    juce::Label xfadeGroupLabel;
    juce::ComboBox xfadeGroupComboBox; // Off, A, B, C, D
    juce::Label zoneRTLabel;
    juce::ComboBox zoneRTComboBox; // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random

    std::array<ZoneEditor, 8> zoneEditors;

    void aliasingDataChanged (int aliasing);
    void aliasingUiChanged (int aliasing);
    void aliasingModDataChanged (juce::String cvInput, double aliasingMod);
    void aliasingModUiChanged (juce::String cvInput, double aliasingMod);
    void attackDataChanged (double attack);
    void attackUiChanged (double attack);
    void attackFromCurrentDataChanged (bool attackFromCurrent);
    void attackFromCurrentUiChanged (bool attackFromCurrent);
    void attackModDataChanged (juce::String cvInput, double attackMod);
    void attackModUiChanged (juce::String cvInput, double attackMod);
    void autoTriggerDataChanged (bool autoTrigger);
    void autoTriggerUiChanged (bool autoTrigger);
    void bitsDataChanged (double bits);
    void bitsUiChanged (double bits);
    void bitsModDataChanged (juce::String cvInput, double bitsMod);
    void bitsModUiChanged (juce::String cvInput, double bitsMod);
    void channelModeDataChanged (int channelMode);
    void channelModeUiChanged (int channelMode);
    void expAMDataChanged (double expAM);
    void expAMUiChanged (double expAM);
    void expFMDataChanged (double expFM);
    void expFMUiChanged (double expFM);
    void levelDataChanged (double level);
    void levelUiChanged (double level);
    void linAMDataChanged (double linAM);
    void linAMUiChanged (double linAM);
    void linAMisExtEnvDataChanged (bool linAMisExtEnv);
    void linAMisExtEnvUiChanged (bool linAMisExtEnv);
    void linFMDataChanged (double linFM);
    void linFMUiChanged (double linFM);
    void loopLengthModDataChanged (juce::String cvInput, double loopLengthMod);
    void loopLengthModUiChanged (juce::String cvInput, double loopLengthMod);
    void loopModeDataChanged (int loopMode);
    void loopModeUiChanged (int loopMode);
    void loopStartModDataChanged (juce::String cvInput, double loopStartMod);
    void loopStartModUiChanged (juce::String cvInput, double loopStartMod);
    void mixLevelDataChanged (double mixLevel);
    void mixLevelUiChanged (double mixLevel);
    void mixModDataChanged (juce::String cvInput, double mixMod);
    void mixModUiChanged (juce::String cvInput, double mixMod);
    void mixModIsFaderDataChanged (bool mixModIsFader);
    void mixModIsFaderUiChanged (bool mixModIsFader);
    void panDataChanged (double pan);
    void panUiChanged (double pan);
    void panModDataChanged (juce::String cvInput, double panMod);
    void panModUiChanged (juce::String cvInput, double panMod);
    void phaseCVDataChanged (juce::String cvInput, double phaseCV);
    void phaseCVUiChanged (juce::String cvInput, double phaseCV);
    void pitchDataChanged (double pitch);
    void pitchUiChanged (double pitch);
    void pitchCVDataChanged (juce::String cvInput, double pitchCV);
    void pitchCVUiChanged (juce::String cvInput, double pitchCV);
    void playModeDataChanged (int playMode);
    void playModeUiChanged (int playMode);
    void pMIndexDataChanged (double pMIndex);
    void pMIndexUiChanged (double pMIndex);
    void pMIndexModDataChanged (juce::String cvInput, double pMIndexMod);
    void pMIndexModUiChanged (juce::String cvInput, double pMIndexMod);
    void pMSourceDataChanged (int pMSource);
    void pMSourceUiChanged (int pMSource);
    void releaseDataChanged (double release);
    void releaseUiChanged (double release);
    void releaseModDataChanged (juce::String cvInput, double releaseMod);
    void releaseModUiChanged (juce::String cvInput, double releaseMod);
    void reverseDataChanged (bool reverse);
    void reverseUiChanged (bool reverse);
    void sampleStartModDataChanged (juce::String cvInput, double sampleStartMod);
    void sampleStartModUiChanged (juce::String cvInput, double sampleStartMod);
    void sampleEndModDataChanged (juce::String cvInput, double sampleEndMod);
    void sampleEndModUiChanged (juce::String cvInput, double sampleEndMod);
    void spliceSmoothingDataChanged (bool spliceSmoothing);
    void spliceSmoothingUiChanged (bool spliceSmoothing);
    void xfadeGroupDataChanged (juce::String xfadeGroup);
    void xfadeGroupUiChanged (juce::String xfadeGroup);
    void zonesCVDataChanged (juce::String zonesCV);
    void zonesCVUiChanged (juce::String zonesCV);
    void zonesRTDataChanged (int zonesRT);
    void zonesRTUiChanged (int zonesRT);
};
