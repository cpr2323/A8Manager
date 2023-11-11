#pragma once

#include <JuceHeader.h>
#include "CvInputComboBox.h"
#include "ZoneEditor.h"
#include "Envelope/AREnvelopeComponent.h"
#include "Envelope/AREnvelopeProperties.h"
#include "SamplePool/SamplePool.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../Utility/NoArrowComboBoxLnF.h"

class TabbedComponentWithChangeCallback : public juce::TabbedComponent
{
public:
    TabbedComponentWithChangeCallback (juce::TabbedButtonBar::Orientation orientation) : juce::TabbedComponent (orientation) {}

    std::function<void (int)> onSelectedTabChanged;

private:
    void currentTabChanged (int newTabIndex, [[maybe_unused]] const juce::String& tabName)
    {
        if (onSelectedTabChanged != nullptr)
            onSelectedTabChanged (newTabIndex);
    }
};

class TransparantOverlay : public juce::Component
{
public:
    TransparantOverlay ()
    {
        setInterceptsMouseClicks (false, false);
    }
private:
    juce::Colour overlayColor { juce::Colours::black };
    float alphaAmount { 0.3f };
    void paint (juce::Graphics& g) override
    {
        g.setColour (overlayColor.withAlpha (alphaAmount));
        g.fillRect (getLocalBounds ());
    }
};

class ZonesTabbedLookAndFeel : public juce::LookAndFeel_V4
{
public:
//     int getTabButtonSpaceAroundImage () override;
//     int getTabButtonOverlap (int tabDepth) override;
    int getTabButtonBestWidth (juce::TabBarButton& button, [[maybe_unused]] int tabDepth) override
    {
        auto& bar { button.getTabbedButtonBar () };
        return button.getTabbedButtonBar ().getHeight () / bar.getNumTabs ();
    }
//     juce::Rectangle< int > getTabButtonExtraComponentBounds (const juce::TabBarButton&, juce::Rectangle< int > &textArea, juce::Component & extraComp) override;
    void drawTabButton (juce::TabBarButton & button, juce::Graphics & g, bool isMouseOver, bool isMouseDown) override
    {
        const auto activeArea { button.getActiveArea () };
        const auto o { button.getTabbedButtonBar ().getOrientation () };
        const auto bkg { button.getTabBackgroundColour () };
        if (button.getToggleState ())
        {
            g.setColour (bkg);
        }
        else
        {
            juce::Point<int> p1, p2;

            switch (o)
            {
                case juce::TabbedButtonBar::TabsAtBottom:   p1 = activeArea.getBottomLeft (); p2 = activeArea.getTopLeft ();    break;
                case juce::TabbedButtonBar::TabsAtTop:      p1 = activeArea.getTopLeft ();    p2 = activeArea.getBottomLeft (); break;
                case juce::TabbedButtonBar::TabsAtRight:    p1 = activeArea.getTopRight ();   p2 = activeArea.getTopLeft ();    break;
                case juce::TabbedButtonBar::TabsAtLeft:     p1 = activeArea.getTopLeft ();    p2 = activeArea.getTopRight ();   break;
                default:                              jassertfalse; break;
            }
            g.setGradientFill (juce::ColourGradient (bkg.brighter (0.2f), p1.toFloat (), bkg.darker (0.1f), p2.toFloat (), false));
        }

        g.fillRect (activeArea);
        g.setColour (button.findColour (juce::TabbedButtonBar::tabOutlineColourId));

        juce::Rectangle<int> r (activeArea);
        if (o != juce::TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
        if (o != juce::TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
        if (o != juce::TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
        if (o != juce::TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));

        const float alpha { button.isEnabled () ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f };

        juce::Colour col (bkg.contrasting ().withMultipliedAlpha (alpha));

        if (juce::TabbedButtonBar * bar { button.findParentComponentOfClass<juce::TabbedButtonBar> () })
        {
            juce::TabbedButtonBar::ColourIds colID { button.isFrontTab () ? juce::TabbedButtonBar::frontTextColourId :
                                                                            juce::TabbedButtonBar::tabTextColourId };

            if (bar->isColourSpecified (colID))
                col = bar->findColour (colID);
            else if (isColourSpecified (colID))
                col = findColour (colID);
        }

        const juce::Rectangle<float> area (button.getTextArea ().toFloat ());

        float length { area.getWidth () };
        float depth { area.getHeight () };

        if (button.getTabbedButtonBar ().isVertical ())
            std::swap (length, depth);

        juce::TextLayout textLayout;
        juce::Font indexFont (depth * 0.4f);
        juce::Font voltageFont (depth * 0.35f);
        juce::AttributedString s;
        auto textToDraw { button.getButtonText ().trim () };
        auto zoneIndexString { textToDraw.upToFirstOccurrenceOf ("\r" , false, true) };
        auto minVoltageString { textToDraw.fromFirstOccurrenceOf ("\r", true, true)};
        s.setJustification (juce::Justification::centred);
        s.append ("   " + zoneIndexString, indexFont, col);
        if (minVoltageString.isNotEmpty ())
        {
#define COLORIZE_VOLTAGE_VALUES 0
#if COLORIZE_VOLTAGE_VALUES
            if (auto minVoltage { minVoltageString.getDoubleValue () }; minVoltage > 0.01)
                col = juce::Colours::green;
            else if (minVoltage <= 0.01 && minVoltage >= -0.01)
                col = juce::Colours::lightgrey.darker (0.2f);
            else
                col = juce::Colours::red;
#else
            col = juce::Colours::lightgrey.darker (0.2f);
#endif
            s.setJustification (juce::Justification::centredLeft);
            s.append (minVoltageString, voltageFont, col);
        }
        textLayout.createLayout (s, length);
        g.setOrigin (1, 2);
        textLayout.draw (g, juce::Rectangle<float> (length, depth));
        g.setOrigin (0, 0);
    }

//     juce::Font getTabButtonFont (juce::TabBarButton&, float height) override;
//     void drawTabButtonText (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
//     void drawTabbedButtonBarBackground (juce::TabbedButtonBar&, juce::Graphics&) override;
//     void drawTabAreaBehindFrontButton (juce::TabbedButtonBar&, juce::Graphics&, int w, int h) override;
//     void createTabButtonShape (juce::TabBarButton&, juce::Path & path, bool isMouseOver, bool isMouseDown) override;
//     void fillTabButtonShape (juce::TabBarButton&, juce::Graphics&, const juce::Path & path, bool isMouseOver, bool isMouseDown) override;
//     juce::Button* createTabBarExtrasButton () override;
private:
};

class ChannelEditor : public juce::Component
{
public:
    ChannelEditor ();
    ~ChannelEditor ();

    void init (juce::ValueTree channelPropertiesVT, juce::ValueTree rootPropertiesVT, SamplePool* theSamplePool);
    void receiveSampleLoadRequest (juce::File sampleFile);
    void checkSampleFileExistence ();

    std::function<void (int channelIndex)> displayToolsMenu;

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
    ChannelProperties defaultChannelProperties;
    ChannelProperties minChannelProperties;
    ChannelProperties maxChannelProperties;
    ZoneProperties defaultZoneProperties;
    ZoneProperties copyBufferZoneProperties;
    bool copyBufferHasData { false };
    AudioPlayerProperties audioPlayerProperties;
    SamplePool* samplePool { nullptr };

    juce::Label zonesLabel;
    juce::Label zoneMaxVoltage;
    TabbedComponentWithChangeCallback zoneTabs { juce::TabbedButtonBar::Orientation::TabsAtLeft };
    juce::TextButton toolsButton;

    juce::Label aliasingLabel;
    juce::TextEditor aliasingTextEditor; // integer
    CvInputChannelComboBox aliasingModComboBox; // 0A - 8C
    juce::TextEditor aliasingModTextEditor; // double
    juce::Label attackLabel;
    juce::TextEditor attackTextEditor; // double
    juce::Label attackFromCurrentLabel;
    juce::ComboBox attackFromCurrentComboBox; // false = start from zero, true = start from last value
    CvInputChannelComboBox attackModComboBox; // 0A - 8C
    juce::TextEditor attackModTextEditor; // double
    juce::Label autoTriggerLabel;
    juce::ComboBox autoTriggerComboBox; //
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
    juce::Label linAMisExtEnvLabel; //
    juce::ComboBox linAMisExtEnvComboBox; //
    juce::Label linFMLabel;
    CvInputChannelComboBox linFMComboBox; // 0A - 8C
    juce::TextEditor linFMTextEditor; // double
    juce::Label loopLengthIsEndLabel;
    juce::ComboBox loopLengthIsEndComboBox;
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
    juce::Label mixModIsFaderLabel; //
    juce::ComboBox mixModIsFaderComboBox; //
    juce::Label mutateLabel;
    juce::Label panMixLabel;
    juce::TextEditor panTextEditor; // double
    juce::Label panLabel;
    CvInputChannelComboBox panModComboBox; // 0A - 8C
    juce::TextEditor panModTextEditor; // double
    juce::Label phaseSourceSectionLabel;
    CvInputChannelComboBox phaseCVComboBox; // 0A - 8C
    juce::TextEditor phaseCVTextEditor; // double
    juce::Label pitchLabel;
    juce::Label pitchSemiLabel;
    juce::TextEditor pitchTextEditor; // double
    CvInputChannelComboBox pitchCVComboBox; // 0A - 8C
    juce::TextEditor pitchCVTextEditor; // double
    juce::Label playModeLabel;
    juce::ComboBox playModeComboBox; // 2 Play Modes: 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
    juce::Label phaseModIndexSectionLabel;
    juce::TextEditor pMIndexTextEditor; // double
    juce::Label pMIndexLabel;
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
    TransparantOverlay stereoRightTransparantOverly;

    AREnvelopeComponent arEnvelopeComponent;
    AREnvelopeProperties arEnvelopeProperties;

    NoArrowComboBoxLnF noArrowComboBoxLnF;
    ZonesTabbedLookAndFeel zonesTabbedLookAndFeel;

    std::array<ZoneEditor, 8> zoneEditors;
    std::array<ZoneProperties, 8> zoneProperties;

    void balanceVoltages (VoltageBalanceType balanceType);
    void checkStereoRightOverlay ();
    void configAudioPlayer ();
    void duplicateZone (int zoneIndex);
    void ensureProperZoneIsSelected ();
    int getEnvelopeValueResolution (double envelopeValue);
    int getNumUsedZones ();
    std::tuple<double, double> getVoltageBoundaries (int zoneIndex, int topDepth);
    void positionColumnOne (int xOffset, int width);
    void positionColumnTwo (int xOffset, int width);
    void positionColumnThree (int xOffset, int width);
    void positionColumnFour (int xOffset, int width);
    void removeEmptyZones ();
    void setupChannelComponents ();
    void setupChannelPropertiesCallbacks ();
    double snapBitsValue (double rawValue);
    double snapEnvelopeValue (double rawValue);
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
    void paint (juce::Graphics& g) override;
    void resized () override;
};
