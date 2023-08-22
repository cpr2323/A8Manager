#include "ParameterHelpers.h"

namespace ParameterHelpers
{
    namespace Preset
    {
        juce::String getNameInputChars ()
        {
            return " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        }

        juce::String getXfadeWidthInputChars ()
        {
            return "+-.0123456789";
        }

        juce::String formatName (juce::String nameToFormat);
        juce::String formatXfadeAWidth (juce::String widthToFormat);
        juce::String formatXfadeBWidth (juce::String widthToFormat);
        juce::String formatXfadeCWidth (juce::String widthToFormat);
        juce::String formatXfadeDWidth (juce::String widthToFormat);

        bool isNameValid (juce::String nameToValidate);
        bool isXfadeAWidthValid (juce::String widthToValidate);
        bool isXfadeBWidthValid (juce::String widthToValidate);
        bool isXfadeCWidthValid (juce::String widthToValidate);
        bool isXfadeDWidthValid (juce::String widthToValidate);
    }
    namespace Channel
    {
        juce::String getAliasingInputChars ();
        juce::String getAliasingModInputChars ();
        juce::String getAttackInputChars ();
        juce::String getAttackModInputChars ();
        juce::String getBitsInputChars ();
        juce::String getBitsModInputChars ();
        juce::String getExpAMInputChars ();
        juce::String getExpFMInputChars ();
        juce::String getLevelInputChars ();
        juce::String getLinAMInputChars ();
        juce::String getLinFMInputChars ();
        juce::String getLoopLengthModInputChars ();
        juce::String getLoopStartModInputChars ();
        juce::String getMixLevelInputChars ();
        juce::String getMixModInputChars ();
        juce::String getPanInputChars ();
        juce::String getPanModInputChars ();
        juce::String getPhaseCVInputChars ();
        juce::String getPitchInputChars ();
        juce::String getPitchCVInputChars ();
        juce::String getPMIndexInputChars ();
        juce::String getPMIndexModInputChars ();
        juce::String getReleaseInputChars ();
        juce::String getReleaseModInputChars ();
        juce::String getSampleStartModInputChars ();
        juce::String getSampleEndModInputChars ();

        juce::String formatAliasing (juce::String aliasingToFormat);
        juce::String formatAliasingMod (juce::String aliasingModToFormat);
        juce::String formatAttack (juce::String attackToFormat);
        juce::String formatAttackMod (juce::String attackModToFormat);
        juce::String formatBits (juce::String bitsToFormat);
        juce::String formatBitsMod (juce::String bitsModToFormat);
        juce::String formatExpAM (juce::String expAMToFormat);
        juce::String formatExpFM (juce::String expFMToFormat);
        juce::String formatLevel (juce::String levelToFormat);
        juce::String formatLinAM (juce::String linAMToFormat);
        juce::String formatLinFM (juce::String linFMToFormat);
        juce::String formatLoopLengthMod (juce::String loopLengthModToFormat);
        juce::String formatLoopStartMod (juce::String loopStartModToFormat);
        juce::String formatMixLevel (juce::String mixLevelToFormat);
        juce::String formatMixMod (juce::String mixModToFormat);
        juce::String formatPan (juce::String panToFormat);
        juce::String formatPanMod (juce::String panModToFormat);
        juce::String formatPhaseCV (juce::String phaseCVToFormat);
        juce::String formatPitch (juce::String pitchToFormat);
        juce::String formatPitchCV (juce::String pitchCVToFormat);
        juce::String formatPMIndex (juce::String pMIndexToFormat);
        juce::String formatPMIndexMod (juce::String pMIndexModToFormat);
        juce::String formatRelease (juce::String releaseToFormat);
        juce::String formatReleaseMod (juce::String releaseModToFormat);
        juce::String formatSampleStartMod (juce::String sampleStartModToFormat);
        juce::String formatSampleEndMod (juce::String sampleEndModToFormat);

        bool isAliasingValid (juce::String aliasingToValidate);
        bool isAliasingModValid (juce::String aliasingModToValidate);
        bool isAttackValid (juce::String attackToValidate);
        bool isAttackModValid (juce::String attackModToValidate);
        bool isBitsValid (juce::String bitsToValidate);
        bool isBitsModValid (juce::String bitsModToValidate);
        bool isExpAMValid (juce::String expAMToValidate);
        bool isExpFMValid (juce::String expFMToValidate);
        bool isLevelValid (juce::String levelToValidate);
        bool isLinAMValid (juce::String linAMToValidate);
        bool isLinFMValid (juce::String linFMToValidate);
        bool isLoopLengthModValid (juce::String loopLengthModToValidate);
        bool isLoopStartModValid (juce::String loopStartModToValidate);
        bool isMixLevelValid (juce::String mixLevelToValidate);
        bool isMixModValid (juce::String mixModToValidate);
        bool isPanValid (juce::String panToValidate);
        bool isPanModValid (juce::String panModToValidate);
        bool isPhaseCVValid (juce::String phaseCVToValidate);
        bool isPitchValid (juce::String pitchToValidate);
        bool isPitchCVValid (juce::String pitchCVToValidate);
        bool isPMIndexValid (juce::String pMIndexToValidate);
        bool isPMIndexModValid (juce::String pMIndexModToValidate);
        bool isReleaseValid (juce::String releaseToValidate);
        bool isReleaseModValid (juce::String releaseModToValidate);
        bool isSampleStartModValid (juce::String sampleStartModToValidate);
        bool isSampleEndModValid (juce::String sampleEndModToValidate);
    }
    namespace Zone
    {
        juce::String getLevelOffsetInputChars ();
        juce::String getLoopLengthInputChars ();
        juce::String getLoopStartInputChars ();
        juce::String getMinVoltageInputChars ();
        juce::String getPitchOffsetInputChars ();
        juce::String getSampleInputChars ();
        juce::String getSampleStartInputChars ();
        juce::String getSampleEndInputChars ();

        juce::String formatLevelOffset (juce::String levelOffsetToFormat);
        juce::String formatLoopLength (juce::String loopLengthToFormat);
        juce::String formatLoopStart (juce::String loopStartToFormat);
        juce::String formatMinVoltage (juce::String minVoltageToFormat);
        juce::String formatPitchOffset (juce::String pitchOffsetToFormat);
        juce::String formatSample (juce::String sampleToFormat);
        juce::String formatSampleStart (juce::String sampleStartToFormat);
        juce::String formatSampleEnd (juce::String sampleEndToFormat);

        bool isLevelOffsetValid (juce::String levelOffsetToValidate);
        bool isLoopLengthValid (juce::String loopLengthToValidate);
        bool isLoopStartValid (juce::String loopStartToValidate);
        bool isMinVoltageValid (juce::String minVoltageToValidate);
        bool isPitchOffsetValid (juce::String pitchOffsetToValidate);
        bool isSampleValid (juce::String sampleToValidate);
        bool isSampleStartValid (juce::String sampleStartToValidate);
        bool isSampleEndValid (juce::String sampleEndToValidate);
    }
}

