#include "ChannelProperties.h"

const auto kMaxZones { 8 };

juce::String getCvInputAndValueString (juce::String cvInput, float value, int decimalPlaces)
{
    return cvInput + " " + juce::String (value, decimalPlaces);
}

AmountAndCvInput getCvInputAndValueFromString (juce::String cvInputAndValueString)
{
    return { cvInputAndValueString.substring (0,2), cvInputAndValueString.substring (4).getFloatValue () };
}

int ChannelProperties::getNumZones ()
{
    auto numZones { 0 };
    forEachZone ([&numZones] (juce::ValueTree) { ++numZones; return true; });
    return numZones;
}

void ChannelProperties::initValueTree ()
{
    // normally in this function we create all of the properties
    // but, as the Assimil8or only writes out parameters that have changed from the defaults
    // we will emulate this by only adding properties when they change, or are in a preset file that is read in
}

juce::ValueTree ChannelProperties::create ()
{
    ChannelProperties channelProperties;
    channelProperties.wrap ({}, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::no);
    return channelProperties.getValueTree ();
}

void ChannelProperties::addZone ()
{
    jassert (getNumZones () < kMaxZones);
    auto zoneProperties { ZoneProperties::create () };
    data.addChild (zoneProperties, -1, nullptr);
}

void ChannelProperties::forEachZone (std::function<bool (juce::ValueTree zoneVT)> zoneVTCallback)
{
    jassert (zoneVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, ZoneProperties::ZoneTypeId, [this, zoneVTCallback] (juce::ValueTree zoneVT)
    {
        zoneVTCallback (zoneVT);
        return true;
    });
}

void ChannelProperties::setAliasing (int aliasing, bool includeSelfCallback)
{
    setValue (aliasing, AliasingPropertyId, includeSelfCallback);
}

void ChannelProperties::setAliasingMod (juce::String cvInput, float aliasingMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, aliasingMod, 4), AliasingModPropertyId, includeSelfCallback);
}

void ChannelProperties::setAttack (float attack, bool includeSelfCallback)
{
    setValue (attack, AttackPropertyId, includeSelfCallback);
}

void ChannelProperties::setAttackFromCurrent (int attackFromCurrent, bool includeSelfCallback)
{
    setValue (attackFromCurrent, AttackFromCurrentPropertyId, includeSelfCallback);
}

void ChannelProperties::setAttackMod (juce::String cvInput, float attackMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, attackMod, 4), AttackModPropertyId, includeSelfCallback);
}

void ChannelProperties::setAutoTrigger (bool autoTrigger, bool includeSelfCallback)
{
    setValue (autoTrigger, AutoTriggerPropertyId, includeSelfCallback);
}

void ChannelProperties::setBits (float bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void ChannelProperties::setBitsMod (juce::String cvInput, float bitsMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, bitsMod, 4), BitsModPropertyId, includeSelfCallback);
}

void ChannelProperties::setChannelMode (int channelMode, bool includeSelfCallback)
{
    setValue (channelMode, ChannelModePropertyId, includeSelfCallback);
}

void ChannelProperties::setExpAM (float expAM, bool includeSelfCallback)
{
    setValue (expAM, ExpAMPropertyId, includeSelfCallback);
}

void ChannelProperties::setExpFM (float expFM, bool includeSelfCallback)
{
    setValue (expFM, ExpFMPropertyId, includeSelfCallback);
}

void ChannelProperties::setLevel (float level, bool includeSelfCallback)
{
    setValue (level, LevelPropertyId, includeSelfCallback);
}

void ChannelProperties::setLinAM (float linAM, bool includeSelfCallback)
{
    setValue (linAM, LinAMPropertyId, includeSelfCallback);
}

void ChannelProperties::setLinAMisExtEnv (bool linAMisExtEnv, bool includeSelfCallback)
{
    setValue (linAMisExtEnv, LinAMisExtEnvPropertyId, includeSelfCallback);
}

void ChannelProperties::setLinFM (float linFM, bool includeSelfCallback)
{
    setValue (linFM, LinFMPropertyId, includeSelfCallback);
}

void ChannelProperties::setLoopLengthMod (juce::String cvInput, float loopLengthMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, loopLengthMod, 4), LoopLengthModPropertyId, includeSelfCallback);
}

void ChannelProperties::setLoopMode (int loopMode, bool includeSelfCallback)
{
    setValue (loopMode, LoopModePropertyId, includeSelfCallback);
}

void ChannelProperties::setLoopStartMod (juce::String cvInput, float loopStartMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, loopStartMod, 4), LoopStartModPropertyId, includeSelfCallback);
}

void ChannelProperties::setMixLevel (float mixLevel, bool includeSelfCallback)
{
    setValue (mixLevel, MixLevelPropertyId, includeSelfCallback);
}

void ChannelProperties::setMixMod (juce::String cvInput, float mixMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, mixMod, 4), MixModPropertyId, includeSelfCallback);
}

void ChannelProperties::setMixModIsFader (bool mixModIsFader, bool includeSelfCallback)
{
    setValue (mixModIsFader, MixModIsFaderPropertyId, includeSelfCallback);
}

void ChannelProperties::setPan (float pan, bool includeSelfCallback)
{
    setValue (pan, PanPropertyId, includeSelfCallback);
}

void ChannelProperties::setPanMod (juce::String cvInput, float panMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, panMod, 4), PanModPropertyId, includeSelfCallback);
}

void ChannelProperties::setPhaseCV (juce::String cvInput, float phaseCV, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, phaseCV, 4), PhaseCVPropertyId, includeSelfCallback);
}

void ChannelProperties::setPitch (float pitch, bool includeSelfCallback)
{
    setValue (pitch, PitchPropertyId, includeSelfCallback);
}

void ChannelProperties::setPitchCV (juce::String cvInput, float pitchCV, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, pitchCV, 4), PitchCVPropertyId, includeSelfCallback);
}

void ChannelProperties::setPlayMode (int PlayMode, bool includeSelfCallback)
{
    setValue (PlayMode, PlayModePropertyId, includeSelfCallback);
}

void ChannelProperties::setPMIndex (float pMIndex, bool includeSelfCallback)
{
    setValue (pMIndex, PMIndexPropertyId, includeSelfCallback);
}

void ChannelProperties::setPMIndexMod (juce::String cvInput, float pMIndexMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, pMIndexMod, 4), PMIndexModPropertyId, includeSelfCallback);
}

void ChannelProperties::setPMSource (int pMSource, bool includeSelfCallback)
{
    setValue (pMSource, PMSourcePropertyId, includeSelfCallback);
}

void ChannelProperties::setRelease (float release, bool includeSelfCallback)
{
    setValue (release, ReleasePropertyId, includeSelfCallback);
}

void ChannelProperties::setReleaseMod (juce::String cvInput, float releaseMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, releaseMod, 4), ReleaseModPropertyId, includeSelfCallback);
}

void ChannelProperties::setReverse (bool reverse, bool includeSelfCallback)
{
    setValue (reverse, ReversePropertyId, includeSelfCallback);
}

void ChannelProperties::setSampleStartMod (juce::String cvInput, float sampleStartMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, sampleStartMod, 4), SampleStartModPropertyId, includeSelfCallback);
}

void ChannelProperties::setSampleEndMod (juce::String cvInput, float sampleEndMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, sampleEndMod, 4), SampleEndModPropertyId, includeSelfCallback);
}

void ChannelProperties::setSpliceSmoothing (bool spliceSmoothing, bool includeSelfCallback)
{
    setValue (spliceSmoothing, SpliceSmoothingPropertyId, includeSelfCallback);
}

void ChannelProperties::setXfadeGroup (juce::String xfadeGroup, bool includeSelfCallback)
{
    setValue (xfadeGroup, XfadeGroupPropertyId, includeSelfCallback);
}

void ChannelProperties::setZonesCV (juce::String zonesCV, bool includeSelfCallback)
{
    setValue (zonesCV, ZonesCVPropertyId, includeSelfCallback);
}

void ChannelProperties::setZonesRT (int zonesRT, bool includeSelfCallback)
{
    setValue (zonesRT, ZonesRTPropertyId, includeSelfCallback);
}

int ChannelProperties::getAliasing ()
{
    return getValue<int> (AliasingPropertyId);
}

AmountAndCvInput ChannelProperties::getAliasingMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (AliasingModPropertyId));
}

float ChannelProperties::getAttack ()
{
    return getValue<float> (AttackPropertyId);
}

int ChannelProperties::getAttackFromCurrent ()
{
    return getValue<int> (AttackFromCurrentPropertyId);
}

AmountAndCvInput ChannelProperties::getAttackMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (AttackModPropertyId));
}

bool ChannelProperties::getAutoTrigger ()
{
    return getValue<bool> (AutoTriggerPropertyId);
}

float ChannelProperties::getBits ()
{
    return getValue<float> (BitsPropertyId);
}

AmountAndCvInput ChannelProperties::getBitsMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (BitsModPropertyId));
}

int ChannelProperties::getChannelMode ()
{
    return getValue<int> (ChannelModePropertyId);
}

float ChannelProperties::getExpAM ()
{
    return getValue<float> (ExpAMPropertyId);
}

float ChannelProperties::getExpFM ()
{
    return getValue<float> (ExpFMPropertyId);
}

float ChannelProperties::getLevel ()
{
    return getValue<float> (LevelPropertyId);
}

float ChannelProperties::getLinAM ()
{
    return getValue<float> (LinAMPropertyId);
}

bool ChannelProperties::getLinAMisExtEnv ()
{
    return getValue<bool> (LinAMisExtEnvPropertyId);
}

float ChannelProperties::getLinFM ()
{
    return getValue<float> (LinFMPropertyId);
}

AmountAndCvInput ChannelProperties::getLoopLengthMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (LoopLengthModPropertyId));
}

int ChannelProperties::getLoopMode ()
{
    return getValue<int> (LoopModePropertyId);
}

AmountAndCvInput ChannelProperties::getLoopStartMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (LoopStartModPropertyId));
}

float ChannelProperties::getMixLevel ()
{
    return getValue<float> (MixLevelPropertyId);
}

AmountAndCvInput ChannelProperties::getMixMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (MixModPropertyId));
}

bool ChannelProperties::getMixModIsFader ()
{
    return getValue<bool> (MixModIsFaderPropertyId);
}

float ChannelProperties::getPan ()
{
    return getValue<float> (PanPropertyId);
}

AmountAndCvInput ChannelProperties::getPanMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PanModPropertyId));
}

AmountAndCvInput ChannelProperties::getPhaseCV ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PhaseCVPropertyId));
}

float ChannelProperties::getPitch ()
{
    return getValue<float> (PitchPropertyId);
}

AmountAndCvInput ChannelProperties::getPitchCV ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PitchCVPropertyId));
}

int ChannelProperties::getPlayMode ()
{
    return getValue<int> (PlayModePropertyId);
}

float ChannelProperties::getPMIndex ()
{
    return getValue<float> (PMIndexPropertyId);
}

AmountAndCvInput ChannelProperties::getPMIndexMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PMIndexModPropertyId));
}

int ChannelProperties::getPMSource ()
{
    return getValue<int> (PMSourcePropertyId);
}

float ChannelProperties::getRelease ()
{
    return getValue<float> (ReleasePropertyId);
}

AmountAndCvInput ChannelProperties::getReleaseMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (ReleaseModPropertyId));
}

bool ChannelProperties::getReverse ()
{
    return getValue<bool> (ReversePropertyId);
}

AmountAndCvInput ChannelProperties::getSampleStartMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (SampleStartModPropertyId));
}

AmountAndCvInput ChannelProperties::getSampleEndMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (SampleEndModPropertyId));
}

bool ChannelProperties::getSpliceSmoothing ()
{
    return getValue<bool> (SpliceSmoothingPropertyId);
}

juce::String ChannelProperties::getXfadeGroup ()
{
    return getValue<juce::String> (XfadeGroupPropertyId);
}

juce::String ChannelProperties::getZonesCV ()
{
    return getValue<juce::String> (ZonesCVPropertyId);
}

int ChannelProperties::getZonesRT ()
{
    return getValue<int> (ZonesRTPropertyId);
}

void ChannelProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
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
