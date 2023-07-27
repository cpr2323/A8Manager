#include "ChannelProperties.h"
#include "ZoneProperties.h"

juce::String getCvInputAndValueString (juce::String cvInput, float value, int decimalPlaces)
{
    return cvInput + " " + juce::String (value, decimalPlaces);
}

AmountAndCvInput getCvInputAndValueFromString (juce::String cvInputAndValueString)
{
    return { cvInputAndValueString.substring (0,2), cvInputAndValueString.substring (4).getFloatValue () };
}

void Assimil8orChannelProperties::initValueTree ()
{
    // normally in this function we create all of the properties
    // but, as the Assimil8or only writes out parameters that have changed from the defaults
    // we will emulate this by only adding properties when they change, or are in a preset file that is read in
}

void Assimil8orChannelProperties::forEachZone (std::function<bool (juce::ValueTree zoneVT)> zoneVTCallback)
{
    jassert (zoneVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, Assimil8orZoneProperties::ZoneTypeId, [this, zoneVTCallback] (juce::ValueTree zoneVT)
    {
        zoneVTCallback (zoneVT);
        return true;
    });
}

void Assimil8orChannelProperties::setAliasing (int aliasing, bool includeSelfCallback)
{
    setValue (aliasing, AliasingPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setAliasingMod (juce::String cvInput, float aliasingMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, aliasingMod, 4), AliasingModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setAttack (float attack, bool includeSelfCallback)
{
    setValue (attack, AttackPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setAttackFromCurrent (int attackFromCurrent, bool includeSelfCallback)
{
    setValue (attackFromCurrent, AttackFromCurrentPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setAttackMod (juce::String cvInput, float attackMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, attackMod, 4), AttackModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setAutoTrigger (bool autoTrigger, bool includeSelfCallback)
{
    setValue (autoTrigger, AutoTriggerPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setBits (float bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setBitsMod (juce::String cvInput, float bitsMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, bitsMod, 4), BitsModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setChannelMode (int channelMode, bool includeSelfCallback)
{
    setValue (channelMode, ChannelModePropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setExpAM (float expAM, bool includeSelfCallback)
{
    setValue (expAM, ExpAMPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setExpFM (float expFM, bool includeSelfCallback)
{
    setValue (expFM, ExpFMPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLevel (float level, bool includeSelfCallback)
{
    setValue (level, LevelPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLinAM (float linAM, bool includeSelfCallback)
{
    setValue (linAM, LinAMPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLinAMisExtEnv (bool linAMisExtEnv, bool includeSelfCallback)
{
    setValue (linAMisExtEnv, LinAMisExtEnvPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLinFM (float linFM, bool includeSelfCallback)
{
    setValue (linFM, LinFMPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLoopLengthMod (juce::String cvInput, float loopLengthMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, loopLengthMod, 4), LoopLengthModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLoopMode (int loopMode, bool includeSelfCallback)
{
    setValue (loopMode, LoopModePropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setLoopStartMod (juce::String cvInput, float loopStartMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, loopStartMod, 4), LoopStartModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setMixLevel (float mixLevel, bool includeSelfCallback)
{
    setValue (mixLevel, MixLevelPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setMixMod (juce::String cvInput, float mixMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, mixMod, 4), MixModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setMixModIsFader (bool mixModIsFader, bool includeSelfCallback)
{
    setValue (mixModIsFader, MixModIsFaderPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPan (float pan, bool includeSelfCallback)
{
    setValue (pan, PanPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPanMod (juce::String cvInput, float panMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, panMod, 4), PanModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPhaseCV (juce::String cvInput, float phaseCV, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, phaseCV, 4), PhaseCVPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPitch (float pitch, bool includeSelfCallback)
{
    setValue (pitch, PitchPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPitchCV (juce::String cvInput, float pitchCV, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, pitchCV, 4), PitchCVPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPlayMode (int PlayMode, bool includeSelfCallback)
{
    setValue (PlayMode, PlayModePropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPMIndex (float pMIndex, bool includeSelfCallback)
{
    setValue (pMIndex, PMIndexPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPMIndexMod (juce::String cvInput, float pMIndexMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, pMIndexMod, 4), PMIndexModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setPMSource (int pMSource, bool includeSelfCallback)
{
    setValue (pMSource, PMSourcePropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setRelease (float release, bool includeSelfCallback)
{
    setValue (release, ReleasePropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setReleaseMod (juce::String cvInput, float releaseMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, releaseMod, 4), ReleaseModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setReverse (bool reverse, bool includeSelfCallback)
{
    setValue (reverse, ReversePropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setSampleStartMod (juce::String cvInput, float sampleStartMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, sampleStartMod, 4), SampleStartModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setSampleEndMod (juce::String cvInput, float sampleEndMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, sampleEndMod, 4), SampleEndModPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setSpliceSmoothing (bool spliceSmoothing, bool includeSelfCallback)
{
    setValue (spliceSmoothing, SpliceSmoothingPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setXfadeGroup (juce::String xfadeGroup, bool includeSelfCallback)
{
    setValue (xfadeGroup, XfadeGroupPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setZonesCV (juce::String zonesCV, bool includeSelfCallback)
{
    setValue (zonesCV, ZonesCVPropertyId, includeSelfCallback);
}

void Assimil8orChannelProperties::setZonesRT (int zonesRT, bool includeSelfCallback)
{
    setValue (zonesRT, ZonesRTPropertyId, includeSelfCallback);
}

int Assimil8orChannelProperties::getAliasing ()
{
    return getValue<int> (AliasingPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getAliasingMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (AliasingModPropertyId));
}

float Assimil8orChannelProperties::getAttack ()
{
    return getValue<float> (AttackPropertyId);
}

int Assimil8orChannelProperties::getAttackFromCurrent ()
{
    return getValue<int> (AttackFromCurrentPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getAttackMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (AttackModPropertyId));
}

bool Assimil8orChannelProperties::getAutoTrigger ()
{
    return getValue<bool> (AutoTriggerPropertyId);
}

float Assimil8orChannelProperties::getBits ()
{
    return getValue<float> (BitsPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getBitsMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (BitsModPropertyId));
}

int Assimil8orChannelProperties::getChannelMode ()
{
    return getValue<int> (ChannelModePropertyId);
}

float Assimil8orChannelProperties::getExpAM ()
{
    return getValue<float> (ExpAMPropertyId);
}

float Assimil8orChannelProperties::getExpFM ()
{
    return getValue<float> (ExpFMPropertyId);
}

float Assimil8orChannelProperties::getLevel ()
{
    return getValue<float> (LevelPropertyId);
}

float Assimil8orChannelProperties::getLinAM ()
{
    return getValue<float> (LinAMPropertyId);
}

bool Assimil8orChannelProperties::getLinAMisExtEnv ()
{
    return getValue<bool> (LinAMisExtEnvPropertyId);
}

float Assimil8orChannelProperties::getLinFM ()
{
    return getValue<float> (LinFMPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getLoopLengthMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (LoopLengthModPropertyId));
}

int Assimil8orChannelProperties::getLoopMode ()
{
    return getValue<int> (LoopModePropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getLoopStartMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (LoopStartModPropertyId));
}

float Assimil8orChannelProperties::getMixLevel ()
{
    return getValue<float> (MixLevelPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getMixMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (MixModPropertyId));
}

bool Assimil8orChannelProperties::getMixModIsFader ()
{
    return getValue<bool> (MixModIsFaderPropertyId);
}

float Assimil8orChannelProperties::getPan ()
{
    return getValue<float> (PanPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getPanMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PanModPropertyId));
}

AmountAndCvInput Assimil8orChannelProperties::getPhaseCV ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PhaseCVPropertyId));
}

float Assimil8orChannelProperties::getPitch ()
{
    return getValue<float> (PitchPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getPitchCV ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PitchCVPropertyId));
}

int Assimil8orChannelProperties::getPlayMode ()
{
    return getValue<int> (PlayModePropertyId);
}

float Assimil8orChannelProperties::getPMIndex ()
{
    return getValue<float> (PMIndexPropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getPMIndexMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PMIndexModPropertyId));
}

int Assimil8orChannelProperties::getPMSource ()
{
    return getValue<int> (PMSourcePropertyId);
}

float Assimil8orChannelProperties::getRelease ()
{
    return getValue<float> (ReleasePropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getReleaseMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (ReleaseModPropertyId));
}

bool Assimil8orChannelProperties::getReverse ()
{
    return getValue<bool> (ReversePropertyId);
}

AmountAndCvInput Assimil8orChannelProperties::getSampleStartMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (SampleStartModPropertyId));
}

AmountAndCvInput Assimil8orChannelProperties::getSampleEndMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (SampleEndModPropertyId));
}

bool Assimil8orChannelProperties::getSpliceSmoothing ()
{
    return getValue<bool> (SpliceSmoothingPropertyId);
}

juce::String Assimil8orChannelProperties::getXfadeGroup ()
{
    return getValue<juce::String> (XfadeGroupPropertyId);
}

juce::String Assimil8orChannelProperties::getZonesCV ()
{
    return getValue<juce::String> (ZonesCVPropertyId);
}

int Assimil8orChannelProperties::getZonesRT ()
{
    return getValue<int> (ZonesRTPropertyId);
}

void Assimil8orChannelProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (property == AliasingPropertyId)
    {
        if (onAliasingChange != nullptr)
            onAliasingChange (getAliasing ());
    }
    else if (property == AliasingModPropertyId)
    {
        if (onAliasingModChange != nullptr)
        {
            onAliasingModChange (getAliasingMod ());
        }
    }
    else if (property == AttackPropertyId)
    {
        if (onAttackChange != nullptr)
            onAttackChange (getAttack ());
    }
    else if (property == AttackFromCurrentPropertyId)
    {
        if (onAttackFromCurrentChange != nullptr)
            onAttackFromCurrentChange (getAttackFromCurrent ());
    }
    else if (property == AttackModPropertyId)
    {
        if (onAttackModChange != nullptr)
            onAttackModChange (getAttackMod ());
    }
    else if (property == AutoTriggerPropertyId)
    {
        if (onAutoTriggerChange != nullptr)
            onAutoTriggerChange (getAutoTrigger ());
    }
    else if (property == BitsPropertyId)
    {
        if (onBitsChange != nullptr)
            onBitsChange (getBits ());
    }
    else if (property == BitsModPropertyId)
    {
        if (onBitsModChange != nullptr)
            onBitsModChange (getBitsMod ());
    }
    else if (property == ChannelModePropertyId)
    {
        if (onChannelModeChange != nullptr)
            onChannelModeChange (getChannelMode ());
    }
    else if (property == ExpAMPropertyId)
    {
        if (onExpAMChange != nullptr)
            onExpAMChange (getExpAM ());
    }
    else if (property == ExpFMPropertyId)
    {
        if (onExpFMChange != nullptr)
            onExpFMChange (getExpFM ());
    }
    else if (property == LevelPropertyId)
    {
        if (onLevelChange != nullptr)
            onLevelChange (getLevel ());
    }
    else if (property == LinAMPropertyId)
    {
        if (onLinAMChange != nullptr)
            onLinAMChange (getLinAM ());
    }
    else if (property == LinAMisExtEnvPropertyId)
    {
        if (onLinAMisExtEnvChange != nullptr)
            onLinAMisExtEnvChange (getLinAMisExtEnv ());
    }
    else if (property == LinFMPropertyId)
    {
        if (onLinFMChange != nullptr)
            onLinFMChange (getLinFM ());
    }
    else if (property == LoopLengthModPropertyId)
    {
        if (onLoopLengthModChange != nullptr)
            onLoopLengthModChange (getLoopLengthMod ());
    }
    else if (property == LoopModePropertyId)
    {
        if (onLoopModeChange != nullptr)
            onLoopModeChange (getLoopMode ());
    }
    else if (property == LoopStartModPropertyId)
    {
        if (onLoopStartModChange != nullptr)
            onLoopStartModChange (getLoopStartMod ());
    }
    else if (property == MixLevelPropertyId)
    {
        if (onMixLevelChange != nullptr)
            onMixLevelChange (getMixLevel ());
    }
    else if (property == MixModPropertyId)
    {
        if (onMixModChange != nullptr)
            onMixModChange (getMixMod ());
    }
    else if (property == MixModIsFaderPropertyId)
    {
        if (onMixModIsFaderChange != nullptr)
            onMixModIsFaderChange (getMixModIsFader ());
    }
    else if (property == PanPropertyId)
    {
        if (onPanChange != nullptr)
            onPanChange (getPan ());
    }
    else if (property == PanModPropertyId)
    {
        if (onPanModChange != nullptr)
            onPanModChange (getPanMod ());
    }
    else if (property == PhaseCVPropertyId)
    {
        if (onPhaseCVChange != nullptr)
            onPhaseCVChange (getPhaseCV ());
    }
    else if (property == PitchPropertyId)
    {
        if (onPitchChange != nullptr)
            onPitchChange (getPitch ());
    }
    else if (property == PitchCVPropertyId)
    {
        if (onPitchCVChange != nullptr)
            onPitchCVChange (getPitchCV ());
    }
    else if (property == PlayModePropertyId)
    {
        if (onPlayModeChange != nullptr)
            onPlayModeChange (getPlayMode ());
    }
    else if (property == PMIndexPropertyId)
    {
        if (onPMIndexChange != nullptr)
            onPMIndexChange (getPMIndex ());
    }
    else if (property == PMIndexModPropertyId)
    {
        if (onPMIndexModChange != nullptr)
            onPMIndexModChange (getPMIndexMod ());
    }
    else if (property == PMSourcePropertyId)
    {
        if (onPMSourceChange != nullptr)
            onPMSourceChange (getPMSource ());
    }
    else if (property == ReleasePropertyId)
    {
        if (onReleaseChange != nullptr)
            onReleaseChange (getRelease ());
    }
    else if (property == ReleaseModPropertyId)
    {
        if (onReleaseModChange != nullptr)
            onReleaseModChange (getReleaseMod ());
    }
    else if (property == ReversePropertyId)
    {
        if (onReverseChange != nullptr)
            onReverseChange (getReverse ());
    }
    else if (property == SampleStartModPropertyId)
    {
        if (onSampleStartModChange != nullptr)
            onSampleStartModChange (getSampleStartMod ());
    }
    else if (property == SampleEndModPropertyId)
    {
        if (onSampleEndModChange != nullptr)
            onSampleEndModChange (getSampleEndMod ());
    }
    else if (property == SpliceSmoothingPropertyId)
    {
        if (onSpliceSmoothingChange != nullptr)
            onSpliceSmoothingChange (getSpliceSmoothing ());
    }
    else if (property == XfadeGroupPropertyId)
    {
        if (onXfadeGroupChange != nullptr)
            onXfadeGroupChange (getXfadeGroup ());
    }
    else if (property == ZonesCVPropertyId)
    {
        if (onZonesCVChange != nullptr)
            onZonesCVChange (getZonesCV ());
    }
    else if (property == ZonesRTPropertyId)
    {
        if (onZonesRTChange != nullptr)
            onZonesRTChange (getZonesRT ());
    }
}
