#pragma once

#include <JuceHeader.h>
#include "CvInputComboBox.h"
#include "EditManager.h"
#include "FormatHelpers.h"
#include "ZoneEditor.h"
#include "Envelope/AREnvelopeComponent.h"
#include "Envelope/AREnvelopeProperties.h"
#include "Waveform/ExpandedWaveformPanel.h"
#include "Waveform/WaveformDisplay.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"
#include "oolib/GUI/CustomComboBox.h"
#include "oolib/GUI/CustomTextButton.h"
#include "oolib/GUI/CustomTextEditor.h"
#include "oolib/GUI/ErrorHelpers.h"
#include "../../Theme/UiComponents.h"

class CvOffsetTextEditor : public CustomTextEditorDouble
{
public:
    CvOffsetTextEditor ()
    {
        toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
        getIncrementCallback = [] () { return 0.01; };
        onDragCallback = [this] (double valueDelta)
        {
            jassert (getCvInputAndAmount != nullptr);
            setValue (FormatHelpers::getAmount (getCvInputAndAmount ()) + valueDelta);
        };
        onPopupMenuCallback = [this] ()
        {
        };
    }
    std::function<CvInputAndAmount ()> getCvInputAndAmount;
private:
};

// washes a stereo right channel's controls back, as they are set by its left channel
class TransparantOverlay : public juce::Component
{
public:
    TransparantOverlay ()
    {
        setInterceptsMouseClicks (false, false);
    }
private:
    void paint (juce::Graphics& g) override
    {
        g.fillAll (findColour (A8Colours::disabledOverlay));
    }
};

class ChannelEditor : public juce::Component
{
public:
    ChannelEditor ();
    ~ChannelEditor ();

    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree uneditedChannelPropertiesVT, juce::ValueTree rootPropertiesVT,
               juce::ValueTree copyBufferZonePropertiesVT, bool* theZoneCopyBufferHasData);

    // TODO - can we move this to the EditManager, as it eventually just calls editManager->assignSamples (parentChannelIndex, startingZoneIndex, files); in the ZoneEditor
    void receiveSampleLoadRequest (juce::File sampleFile);

    // TODO - can we make this local, since we should be able to access the edits through the EditManager
    std::function<void (int channelIndex)> displayToolsMenu;

    // The expanded waveform is one setting for every channel, so it stays up while moving between
    // them: the expand and collapse tools ask the owner, which sets it on all of the channels.
    void setWaveformExpanded (bool isExpanded);
    std::function<void (bool isExpanded)> onWaveformExpandedChange;

private:
    enum class VoltageBalanceType
    {
        distributeAcross5V,
        distributeAcross10V,
        distribute1vPerOct,
        distribute1vPerOctMajor
    };
    AppProperties appProperties;
    ChannelProperties channelProperties;
    ChannelProperties uneditedChannelProperties;
    ChannelProperties defaultChannelProperties;
    ChannelProperties minChannelProperties;
    ChannelProperties maxChannelProperties;
    ZoneProperties defaultZoneProperties;
    ZoneProperties copyBufferZoneProperties;
    SampleManagerProperties sampleManagerProperties;
    bool* zoneCopyBufferHasData { nullptr };
    AudioPlayerProperties audioPlayerProperties;
    EditManager* editManager { nullptr };
    int channelIndex { -1 };

    juce::Label zonesLabel;
    juce::Label zoneMaxVoltage;
    LedTabbedComponent zoneTabs { juce::TabbedButtonBar::Orientation::TabsAtLeft };
    MenuButton toolsButton { "CHANNEL TOOLS", ActionButton::Size::small };
    // the labels are coloured by role, and a palette change has to reach them again
    std::vector<juce::Label*> sectionHeaderLabels;
    std::vector<juce::Label*> parameterLabels;

    juce::Label aliasingLabel;
    CustomTextEditorInt aliasingTextEditor; // integer
    CvInputChannelComboBox aliasingModComboBox; // 0A - 8C
    CvOffsetTextEditor aliasingModTextEditor; // double
    juce::Label attackLabel;
    CustomTextEditorDouble attackTextEditor; // double
    juce::Label attackFromCurrentLabel;
    CustomComboBox attackFromCurrentComboBox; // false = start from zero, true = start from last value
    CvInputChannelComboBox attackModComboBox; // 0A - 8C
    CvOffsetTextEditor attackModTextEditor; // double
    juce::Label autoTriggerLabel;
    CustomComboBox autoTriggerComboBox; //
    juce::Label bitsLabel;
    CustomTextEditorDouble bitsTextEditor; // double
    CvInputChannelComboBox bitsModComboBox; // 0A - 8C
    CvOffsetTextEditor bitsModTextEditor; // double
    juce::Label channelModeLabel;
    CustomComboBox channelModeComboBox; // 4 Channel Modes: 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
    juce::Label envelopeLabel;
    juce::Label expAMLabel;
    CvInputChannelComboBox expAMComboBox; // 0A - 8C
    CvOffsetTextEditor expAMTextEditor; // double
    juce::Label expFMLabel;
    CvInputChannelComboBox expFMComboBox; // 0A - 8C
    CvOffsetTextEditor expFMTextEditor; // double
    juce::Label levelLabel;
    juce::Label levelDbLabel;
    CustomTextEditorDouble levelTextEditor; // double
    juce::Label linAMLabel;
    CvInputChannelComboBox linAMComboBox; // 0A - 8C
    CvOffsetTextEditor linAMTextEditor; // double
    juce::Label linAMisExtEnvLabel; //
    CustomComboBox linAMisExtEnvComboBox; //
    juce::Label linFMLabel;
    CvInputChannelComboBox linFMComboBox; // 0A - 8C
    CvOffsetTextEditor linFMTextEditor; // double
    juce::Label loopLengthIsEndLabel;
    CustomComboBox loopLengthIsEndComboBox;
    juce::Label loopLengthModLabel;
    CvInputChannelComboBox loopLengthModComboBox; // 0A - 8C
    CvOffsetTextEditor loopLengthModTextEditor; // double
    juce::Label loopModeLabel;
    CustomComboBox loopModeComboBox; // 0 = No Loop, 1 = Loop, 2 = Loop and Release
    juce::Label loopStartModLabel;
    CvInputChannelComboBox loopStartModComboBox; // 0A - 8C
    CvOffsetTextEditor loopStartModTextEditor; // double
    juce::Label mixLevelLabel;
    CustomTextEditorDouble mixLevelTextEditor; // double
    CvInputChannelComboBox mixModComboBox; // 0A - 8C
    CvOffsetTextEditor mixModTextEditor; // double
    juce::Label mixModIsFaderLabel; //
    CustomComboBox mixModIsFaderComboBox; //
    juce::Label mutateLabel;
    juce::Label panMixLabel;
    CustomTextEditorDouble panTextEditor; // double
    juce::Label panLabel;
    CvInputChannelComboBox panModComboBox; // 0A - 8C
    CvOffsetTextEditor panModTextEditor; // double
    juce::Label phaseSourceSectionLabel;
    CvInputChannelComboBox phaseCVComboBox; // 0A - 8C
    CvOffsetTextEditor phaseCVTextEditor; // double
    juce::Label pitchLabel;
    juce::Label pitchSemiLabel;
    CustomTextEditorDouble pitchTextEditor; // double
    CvInputChannelComboBox pitchCVComboBox; // 0A - 8C
    CvOffsetTextEditor pitchCVTextEditor;
    juce::Label playModeLabel;
    CustomComboBox playModeComboBox; // 2 Play Modes: 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
    juce::Label phaseModIndexSectionLabel;
    CustomTextEditorDouble pMIndexTextEditor; // double
    juce::Label pMIndexLabel;
    CvInputChannelComboBox pMIndexModComboBox; // 0A - 8C
    CvOffsetTextEditor pMIndexModTextEditor; // double
    juce::Label pMSourceLabel;
    CustomComboBox pMSourceComboBox; // Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
    juce::Label releaseLabel;
    CustomTextEditorDouble releaseTextEditor; // double
    CvInputChannelComboBox releaseModComboBox; // 0A - 8C
    CvOffsetTextEditor releaseModTextEditor; // double
    CustomTextButton reverseButton; //
    juce::Label sampleEndModLabel;
    CvInputChannelComboBox sampleEndModComboBox; // 0A - 8C
    CvOffsetTextEditor sampleEndModTextEditor; // double
    juce::Label sampleStartModLabel;
    CvInputChannelComboBox sampleStartModComboBox; // 0A - 8C
    CvOffsetTextEditor sampleStartModTextEditor; // double
    CustomTextButton spliceSmoothingButton; //
    juce::Label xfadeGroupLabel;
    CustomComboBox xfadeGroupComboBox; // Off, A, B, C, D
    juce::Label zonesCVLabel;
    CvInputChannelComboBox zonesCVComboBox; // 0A - 8C
    juce::Label zonesRTLabel;
    CustomComboBox zonesRTComboBox; // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
    TransparantOverlay stereoRightTransparantOverly;

    AREnvelopeComponent arEnvelopeComponent;
    AREnvelopeProperties arEnvelopeProperties;

    WaveformDisplay sampleWaveformDisplay;
    ExpandedWaveformPanel expandedWaveformPanel;

    std::array<ZoneEditor, 8> zoneEditors;
    std::array<ZoneProperties, 8> zoneProperties;

    void balanceVoltages (VoltageBalanceType balanceType);
    void checkStereoRightOverlay ();
    void clearAllZones ();
    void configAudioPlayer ();
    void copyZone (int zoneIndex, bool settingsOnly);
    void deleteZone (int zoneIndex);
    void duplicateZone (int zoneIndex);
    void ensureProperZoneIsSelected ();
    void explodeZone (int zoneIndex, int explodeCount);
    int getEnvelopeValueResolution (double envelopeValue);
    void flipZones (int zoneIndex, int flipCount);
    void pasteZone (int zoneIndex);
    void positionColumnOne (int xOffset, int width);
    void positionColumnTwo (int xOffset, int width);
    void positionColumnThree (int xOffset, int width);
    void positionColumnFour (int xOffset, int width);
    void removeEmptyZones ();
    void setupChannelComponents ();
    void setupChannelPropertiesCallbacks ();
    double snapBitsValue (double rawValue);
    double snapEnvelopeValue (double rawValue);
    double truncateToDecimalPlaces (double rawValue, int decimalPlaces);
    double snapValue (double rawValue, double snapAmount);
    juce::PopupMenu createChannelCloneMenu (std::function <void (ChannelProperties&)> setter, std::function<bool (ChannelProperties&)> canCloneCallback, std::function<bool (ChannelProperties&)> canCloneToAllCallback);
    juce::PopupMenu createChannelEditMenu (std::function <void (ChannelProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter);
    void updateAllZoneTabNames ();
    void updateZoneTabName (int zoneIndex);

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
    void loopLengthIsEndDataChanged (bool loopLengthIsEnd);
    void loopLengthIsEndUiChanged (bool loopLengthIsEnd);
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

    void visibilityChanged () override;
    void lookAndFeelChanged () override;
    void resized () override;
    void updateWaveformDisplay ();
    void requestWaveformExpanded (bool isExpanded);
};
