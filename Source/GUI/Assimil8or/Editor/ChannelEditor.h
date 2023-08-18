#pragma once

#include <JuceHeader.h>
#include "CvInputComboBox.h"
#include "ZoneEditor.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../Utility/NoArrowComboBoxLnF.h"

class ChannelEditor : public juce::Component
{
public:
    ChannelEditor ();
    ~ChannelEditor ();

    void init (juce::ValueTree channelPropertiesVT);

private:
    ChannelProperties channelProperties;
    juce::Label zonesLabel;
    juce::TabbedComponent zoneTabs { juce::TabbedButtonBar::Orientation::TabsAtLeft };

    juce::Label aliasingLabel;
    juce::TextEditor aliasingTextEditor; // integer
    CvInputChannelComboBox aliasingModComboBox; // 0A - 8C
    juce::TextEditor aliasingModTextEditor; // double
    juce::Label attackLabel;
    juce::TextEditor attackTextEditor; // double
    juce::TextButton attackFromCurrentButton; // false = start from zero, true = start from last value
    CvInputChannelComboBox attackModComboBox; // 0A - 8C
    juce::TextEditor attackModTextEditor; // double
    juce::Label autoTriggerLabel;
    juce::TextButton autoTriggerButton; //
    juce::Label bitsLabel;
    juce::TextEditor bitsTextEditor; // double
    CvInputChannelComboBox bitsModComboBox; // 0A - 8C
    juce::TextEditor bitsModTextEditor; // double
    juce::Label channelModeLabel;
    juce::ComboBox channelModeComboBox; // 4 Channel Modes: 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    juce::Label envelopeLabel;
    juce::Label expAMLabel;
    CvInputChannelComboBox expAMComboBox; // 0A - 8C
    juce::TextEditor expAMTextEditor; // double
    juce::Label expFMLabel;
    CvInputChannelComboBox expFMComboBox; // 0A - 8C
    juce::TextEditor expFMTextEditor; // double
    juce::Label levelLabel;
    juce::Label levelDbLabel;
    juce::TextEditor levelTextEditor; // double
    juce::Label linAMLabel;
    CvInputChannelComboBox linAMComboBox; // 0A - 8C
    juce::TextEditor linAMTextEditor; // double
    juce::TextButton linAMisExtEnvButton; // 
    juce::Label linFMLabel;
    CvInputChannelComboBox linFMComboBox; // 0A - 8C
    juce::TextEditor linFMTextEditor; // double
    juce::Label loopLengthModLabel;
    CvInputChannelComboBox loopLengthModComboBox; // 0A - 8C
    juce::TextEditor loopLengthModTextEditor; // double
    juce::Label loopModeLabel;
    juce::ComboBox loopModeComboBox; // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    juce::Label loopStartModLabel;
    CvInputChannelComboBox loopStartModComboBox; // 0A - 8C
    juce::TextEditor loopStartModTextEditor; // double
    juce::Label mixLevelLabel;
    juce::TextEditor mixLevelTextEditor; // double
    CvInputChannelComboBox mixModComboBox; // 0A - 8C
    juce::TextEditor mixModTextEditor; // double
    juce::TextButton mixModIsFaderButton; // 
    juce::Label mutateLabel;
    juce::Label panMixLabel;
    juce::TextEditor panTextEditor; // double
    juce::Label panLabel;
    CvInputChannelComboBox panModComboBox; // 0A - 8C
    juce::TextEditor panModTextEditor; // double
    juce::Label phaseCVLabel;
    CvInputChannelComboBox phaseCVComboBox; // 0A - 8C
    juce::TextEditor phaseCVTextEditor; // double
    juce::Label pitchLabel;
    juce::Label pitchSemiLabel;
    juce::TextEditor pitchTextEditor; // double
    CvInputChannelComboBox pitchCVComboBox; // 0A - 8C
    juce::TextEditor pitchCVTextEditor; // double
    juce::Label playModeLabel;
    juce::ComboBox playModeComboBox; // 2 Play Modes: 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
    juce::Label pMIndexLabel;
    juce::TextEditor pMIndexTextEditor; // double
    juce::Label pMIndexModLabel;
    CvInputChannelComboBox pMIndexModComboBox; // 0A - 8C
    juce::TextEditor pMIndexModTextEditor; // double
    juce::Label pMSourceLabel;
    juce::ComboBox pMSourceComboBox; // Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
    juce::Label releaseLabel;
    juce::TextEditor releaseTextEditor; // double
    CvInputChannelComboBox releaseModComboBox; // 0A - 8C
    juce::TextEditor releaseModTextEditor; // double
    juce::TextButton reverseButton; // 
    juce::Label sampleEndModLabel;
    CvInputChannelComboBox sampleEndModComboBox; // 0A - 8C
    juce::TextEditor sampleEndModTextEditor; // double
    juce::Label sampleStartModLabel;
    CvInputChannelComboBox sampleStartModComboBox; // 0A - 8C
    juce::TextEditor sampleStartModTextEditor; // double
    juce::TextButton spliceSmoothingButton; //
    juce::Label xfadeGroupLabel;
    juce::ComboBox xfadeGroupComboBox; // Off, A, B, C, D
    juce::Label zonesCVLabel;
    CvInputChannelComboBox zonesCVComboBox; // 0A - 8C
    juce::Label zonesRTLabel;
    juce::ComboBox zonesRTComboBox; // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random

    NoArrowComboBoxLnF noArrowComboBoxLnF;

    std::array<ZoneEditor, 8> zoneEditors;

    void setupChannelControls ();
    void setupChannelPropertiesCallbacks ();

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
    void expAMDataChanged (juce::String cvInput, double expAM);
    void expAMUiChanged (juce::String cvInput, double expAM);
    void expFMDataChanged (juce::String cvInput, double expFM);
    void expFMUiChanged (juce::String cvInput, double expFM);
    void levelDataChanged (double level);
    void levelUiChanged (double level);
    void linAMDataChanged (juce::String cvInput, double linAM);
    void linAMUiChanged (juce::String cvInput, double linAM);
    void linAMisExtEnvDataChanged (bool linAMisExtEnv);
    void linAMisExtEnvUiChanged (bool linAMisExtEnv);
    void linFMDataChanged (juce::String cvInput, double linFM);
    void linFMUiChanged (juce::String cvInput, double linFM);
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

    void paint (juce::Graphics& g) override;
    void resized () override;
};
