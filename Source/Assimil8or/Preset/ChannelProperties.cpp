#include "ChannelProperties.h"
#include "ParameterDataProperties.h"
#include "ParameterNames.h"

const auto kMaxZones { 8 };

void ChannelProperties::initValueTree ()
{
    for (auto zoneIndex { 0 }; zoneIndex < kMaxZones; ++zoneIndex)
        addZone (zoneIndex);
}

int ChannelProperties::getNumZones ()
{
    auto numZones { 0 };
    forEachZone ([&numZones] (juce::ValueTree) { ++numZones; return true; });
    return numZones;
}

juce::ValueTree ChannelProperties::create (int index)
{
    ChannelProperties channelProperties;
    channelProperties.setIndex (index, false);
    return channelProperties.getValueTree ();
}

juce::ValueTree ChannelProperties::addZone (int index)
{
    jassert (getNumZones () < kMaxZones);
    auto zoneProperties { ZoneProperties::create (index) };
    data.addChild (zoneProperties, -1, nullptr);
    return zoneProperties;
}

void ChannelProperties::forEachZone (std::function<bool (juce::ValueTree zoneVT)> zoneVTCallback)
{
    jassert (zoneVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, ZoneProperties::ZoneTypeId, [this, zoneVTCallback] (juce::ValueTree zoneVT)
    {
        return zoneVTCallback (zoneVT);
    });
}

juce::ValueTree ChannelProperties::getZoneVT (int zoneIndex)
{
    jassert (zoneIndex < getNumZones ());
    juce::ValueTree requestedChannelVT;
    auto curZoneIndex { 0 };
    forEachZone ([this, &requestedChannelVT, &curZoneIndex, zoneIndex] (juce::ValueTree channelVT)
        {
            if (curZoneIndex == zoneIndex)
            {
                requestedChannelVT = channelVT;
                return false;
            }
            ++curZoneIndex;
            return true;
        });
    jassert (requestedChannelVT.isValid ());
    return requestedChannelVT;
}

juce::String ChannelProperties::getCvInputAndValueString (juce::String cvInput, double value, int decimalPlaces)
{
    return cvInput + " " + juce::String (value, decimalPlaces);
}

juce::String ChannelProperties::getCvInputAndValueString (CvInputAndAmount cvInputAndValue, int decimalPlaces)
{
    const auto& [cvInput, value] { cvInputAndValue };
    return getCvInputAndValueString (cvInput, value, decimalPlaces);
}

CvInputAndAmount ChannelProperties::getCvInputAndValueFromString (juce::String cvInputAndValueString)
{
    const auto delimiterLocation { cvInputAndValueString.indexOfChar (0, ' ') };
    jassert (delimiterLocation != 0);
    return { cvInputAndValueString.substring (0, delimiterLocation), cvInputAndValueString.substring (delimiterLocation + 1).getFloatValue () };
}


void ChannelProperties::initToDefaults ()
{
    clear (false); // clear everything
}

void ChannelProperties::initToDefaultIsMissing ()
{
    clear (true); // only init the missing things
}

void ChannelProperties::clear (bool onlyClearIfPropertyMissing)
{
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (AliasingPropertyId))
        setAliasing (getAliasingDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (AliasingModPropertyId))
    {
        const auto [cvInput, aliasingModAmount] { getAliasingModDefault () };
        setAliasingMod (cvInput, aliasingModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (AttackPropertyId))
        setAttack (getAttackDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (AttackFromCurrentPropertyId))
        setAttackFromCurrent (getAttackFromCurrentDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (AttackModPropertyId))
    {
        const auto [cvInput, attackModAmount] { getAttackModDefault () };
        setAttackMod (cvInput, attackModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (AutoTriggerPropertyId))
        setAutoTrigger (getAutoTriggerDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (BitsPropertyId))
        setBits (getBitsDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (BitsModPropertyId))
    {
        const auto [cvInput, bitsModAmount] { getBitsModDefault () };
        setBitsMod (cvInput, bitsModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ChannelModePropertyId))
        setChannelMode (getChannelModeDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ExpAMPropertyId))
        setExpAM (getExpAMDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ExpFMPropertyId))
        setExpFM (getExpFMDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LevelPropertyId))
        setLevel (getLevelDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LinAMPropertyId))
        setLinAM (getLinAMDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LinAMisExtEnvPropertyId))
        setLinAMisExtEnv (getLinAMisExtEnvDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LinFMPropertyId))
        setLinFM (getLinFMDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LoopLengthModPropertyId))
    {
        const auto [cvInput, loopLengthModAmount] { getLoopLengthModDefault () };
        setLoopLengthMod (cvInput, loopLengthModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LoopModePropertyId))
        setLoopMode (getLoopModeDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (LoopStartModPropertyId))
    {
        const auto [cvInput, loopStartModAmount] { getLoopStartModDefault () };
        setLoopStartMod (cvInput, loopStartModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (MixLevelPropertyId))
        setMixLevel (getMixLevelDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (MixModPropertyId))
    {
        const auto [cvInput, mixModAmount] { getMixModDefault () };
        setMixMod (cvInput, mixModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (MixModIsFaderPropertyId))
        setMixModIsFader (getMixModIsFaderDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PanPropertyId))
        setPan (getPanDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PanModPropertyId))
    {
        const auto [cvInput, panModAmount] { getPanModDefault () };
        setPanMod (cvInput, panModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PhaseCVPropertyId))
    {
        const auto [cvInput, phaseCvModAmount] { getPhaseCVDefault () };
        setPhaseCV (cvInput, phaseCvModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PitchPropertyId))
        setPitch (getPitchDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PitchCVPropertyId))
    {
        const auto [cvInput, pitchCvModAmount] { getPitchCVDefault () };
        setPitchCV (cvInput, pitchCvModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PlayModePropertyId))
        setPlayMode (getPlayModeDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PMIndexPropertyId))
        setPMIndex (getPMIndexDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PMIndexModPropertyId))
    {
        const auto [cvInput, pmIndexModAmount] { getPMIndexModDefault () };
        setPMIndexMod (cvInput, pmIndexModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (PMSourcePropertyId))
        setPMSource (getPMSourceDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ReleasePropertyId))
        setRelease (getReleaseDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ReleaseModPropertyId))
    {
        const auto [cvInput, releaseModAmount] { getReleaseModDefault () };
        setReleaseMod (cvInput, releaseModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ReversePropertyId))
        setReverse (getReverseDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (SampleStartModPropertyId))
    {
        const auto [cvInput, sampleStartModAmount] { getSampleStartModDefault () };
        setSampleStartMod (cvInput, sampleStartModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (SampleEndModPropertyId))
    {
        const auto [cvInput, sampleEndModAmount] { getSampleEndModDefault () };
        setSampleEndMod (cvInput, sampleEndModAmount, false);
    }
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (SpliceSmoothingPropertyId))
        setSpliceSmoothing (getSpliceSmoothingDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (XfadeGroupPropertyId))
        setXfadeGroup (getXfadeGroupDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ZonesCVPropertyId))
        setZonesCV (getZonesCVDefault (), false);
    if (! onlyClearIfPropertyMissing || ! data.hasProperty (ZonesRTPropertyId))
        setZonesRT (getZonesRTDefault (), false);

    forEachZone ([this, onlyClearIfPropertyMissing] (juce::ValueTree zonePropertiesVT)
    {
        ZoneProperties zoneProperties (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        if (onlyClearIfPropertyMissing)
            zoneProperties.initToDefaultIsMissing ();
        else
            zoneProperties.initToDefaults ();
        return true;
    });
}

////////////////////////////////////////////////////////////////////
// set___
////////////////////////////////////////////////////////////////////
void ChannelProperties::setIndex (int index, bool includeSelfCallback)
{
    setValue (index, IndexPropertyId, includeSelfCallback);
}

void ChannelProperties::setAliasing (int aliasing, bool includeSelfCallback)
{
    setValue (aliasing, AliasingPropertyId, includeSelfCallback);
}

void ChannelProperties::setAliasingMod (juce::String cvInput, double aliasingMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, aliasingMod, 4), AliasingModPropertyId, includeSelfCallback);
}

void ChannelProperties::setAttack (double attack, bool includeSelfCallback)
{
    setValue (attack, AttackPropertyId, includeSelfCallback);
}

void ChannelProperties::setAttackFromCurrent (bool attackFromCurrent, bool includeSelfCallback)
{
    setValue (attackFromCurrent, AttackFromCurrentPropertyId, includeSelfCallback);
}

void ChannelProperties::setAttackMod (juce::String cvInput, double attackMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, attackMod, 4), AttackModPropertyId, includeSelfCallback);
}

void ChannelProperties::setAutoTrigger (bool autoTrigger, bool includeSelfCallback)
{
    setValue (autoTrigger, AutoTriggerPropertyId, includeSelfCallback);
}

void ChannelProperties::setBits (double bits, bool includeSelfCallback)
{
    setValue (bits, BitsPropertyId, includeSelfCallback);
}

void ChannelProperties::setBitsMod (juce::String cvInput, double bitsMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, bitsMod, 4), BitsModPropertyId, includeSelfCallback);
}

void ChannelProperties::setChannelMode (int channelMode, bool includeSelfCallback)
{
    setValue (channelMode, ChannelModePropertyId, includeSelfCallback);
}

void ChannelProperties::setExpAM (double expAM, bool includeSelfCallback)
{
    setValue (expAM, ExpAMPropertyId, includeSelfCallback);
}

void ChannelProperties::setExpFM (double expFM, bool includeSelfCallback)
{
    setValue (expFM, ExpFMPropertyId, includeSelfCallback);
}

void ChannelProperties::setLevel (double level, bool includeSelfCallback)
{
    setValue (level, LevelPropertyId, includeSelfCallback);
}

void ChannelProperties::setLinAM (double linAM, bool includeSelfCallback)
{
    setValue (linAM, LinAMPropertyId, includeSelfCallback);
}

void ChannelProperties::setLinAMisExtEnv (bool linAMisExtEnv, bool includeSelfCallback)
{
    setValue (linAMisExtEnv, LinAMisExtEnvPropertyId, includeSelfCallback);
}

void ChannelProperties::setLinFM (double linFM, bool includeSelfCallback)
{
    setValue (linFM, LinFMPropertyId, includeSelfCallback);
}

void ChannelProperties::setLoopLengthMod (juce::String cvInput, double loopLengthMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, loopLengthMod, 4), LoopLengthModPropertyId, includeSelfCallback);
}

void ChannelProperties::setLoopMode (int loopMode, bool includeSelfCallback)
{
    setValue (loopMode, LoopModePropertyId, includeSelfCallback);
}

void ChannelProperties::setLoopStartMod (juce::String cvInput, double loopStartMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, loopStartMod, 4), LoopStartModPropertyId, includeSelfCallback);
}

void ChannelProperties::setMixLevel (double mixLevel, bool includeSelfCallback)
{
    setValue (mixLevel, MixLevelPropertyId, includeSelfCallback);
}

void ChannelProperties::setMixMod (juce::String cvInput, double mixMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, mixMod, 4), MixModPropertyId, includeSelfCallback);
}

void ChannelProperties::setMixModIsFader (bool mixModIsFader, bool includeSelfCallback)
{
    setValue (mixModIsFader, MixModIsFaderPropertyId, includeSelfCallback);
}

void ChannelProperties::setPan (double pan, bool includeSelfCallback)
{
    setValue (pan, PanPropertyId, includeSelfCallback);
}

void ChannelProperties::setPanMod (juce::String cvInput, double panMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, panMod, 4), PanModPropertyId, includeSelfCallback);
}

void ChannelProperties::setPhaseCV (juce::String cvInput, double phaseCV, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, phaseCV, 4), PhaseCVPropertyId, includeSelfCallback);
}

void ChannelProperties::setPitch (double pitch, bool includeSelfCallback)
{
    setValue (pitch, PitchPropertyId, includeSelfCallback);
}

void ChannelProperties::setPitchCV (juce::String cvInput, double pitchCV, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, pitchCV, 4), PitchCVPropertyId, includeSelfCallback);
}

void ChannelProperties::setPlayMode (int playMode, bool includeSelfCallback)
{
    setValue (playMode, PlayModePropertyId, includeSelfCallback);
}

void ChannelProperties::setPMIndex (double pMIndex, bool includeSelfCallback)
{
    setValue (pMIndex, PMIndexPropertyId, includeSelfCallback);
}

void ChannelProperties::setPMIndexMod (juce::String cvInput, double pMIndexMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, pMIndexMod, 4), PMIndexModPropertyId, includeSelfCallback);
}

void ChannelProperties::setPMSource (int pMSource, bool includeSelfCallback)
{
    setValue (pMSource, PMSourcePropertyId, includeSelfCallback);
}

void ChannelProperties::setRelease (double release, bool includeSelfCallback)
{
    setValue (release, ReleasePropertyId, includeSelfCallback);
}

void ChannelProperties::setReleaseMod (juce::String cvInput, double releaseMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, releaseMod, 4), ReleaseModPropertyId, includeSelfCallback);
}

void ChannelProperties::setReverse (bool reverse, bool includeSelfCallback)
{
    setValue (reverse, ReversePropertyId, includeSelfCallback);
}

void ChannelProperties::setSampleStartMod (juce::String cvInput, double sampleStartMod, bool includeSelfCallback)
{
    setValue (getCvInputAndValueString (cvInput, sampleStartMod, 4), SampleStartModPropertyId, includeSelfCallback);
}

void ChannelProperties::setSampleEndMod (juce::String cvInput, double sampleEndMod, bool includeSelfCallback)
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

////////////////////////////////////////////////////////////////////
// get___
////////////////////////////////////////////////////////////////////
int ChannelProperties::getIndex ()
{
    return getValue<int> (IndexPropertyId);
}

int ChannelProperties::getAliasing ()
{
    return getValue<int> (AliasingPropertyId);
}

CvInputAndAmount ChannelProperties::getAliasingMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (AliasingModPropertyId));
}

double ChannelProperties::getAttack ()
{
    return getValue<double> (AttackPropertyId);
}

bool ChannelProperties::getAttackFromCurrent ()
{
    return getValue<bool> (AttackFromCurrentPropertyId);
}

CvInputAndAmount ChannelProperties::getAttackMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (AttackModPropertyId));
}

bool ChannelProperties::getAutoTrigger ()
{
    return getValue<bool> (AutoTriggerPropertyId);
}

double ChannelProperties::getBits ()
{
    return getValue<double> (BitsPropertyId);
}

CvInputAndAmount ChannelProperties::getBitsMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (BitsModPropertyId));
}

int ChannelProperties::getChannelMode ()
{
    return getValue<int> (ChannelModePropertyId);
}

double ChannelProperties::getExpAM ()
{
    return getValue<double> (ExpAMPropertyId);
}

double ChannelProperties::getExpFM ()
{
    return getValue<double> (ExpFMPropertyId);
}

double ChannelProperties::getLevel ()
{
    return getValue<double> (LevelPropertyId);
}

double ChannelProperties::getLinAM ()
{
    return getValue<double> (LinAMPropertyId);
}

bool ChannelProperties::getLinAMisExtEnv ()
{
    return getValue<bool> (LinAMisExtEnvPropertyId);
}

double ChannelProperties::getLinFM ()
{
    return getValue<double> (LinFMPropertyId);
}

CvInputAndAmount ChannelProperties::getLoopLengthMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (LoopLengthModPropertyId));
}

int ChannelProperties::getLoopMode ()
{
    return getValue<int> (LoopModePropertyId);
}

CvInputAndAmount ChannelProperties::getLoopStartMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (LoopStartModPropertyId));
}

double ChannelProperties::getMixLevel ()
{
    return getValue<double> (MixLevelPropertyId);
}

CvInputAndAmount ChannelProperties::getMixMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (MixModPropertyId));
}

bool ChannelProperties::getMixModIsFader ()
{
    return getValue<bool> (MixModIsFaderPropertyId);
}

double ChannelProperties::getPan ()
{
    return getValue<double> (PanPropertyId);
}

CvInputAndAmount ChannelProperties::getPanMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PanModPropertyId));
}

CvInputAndAmount ChannelProperties::getPhaseCV ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PhaseCVPropertyId));
}

double ChannelProperties::getPitch ()
{
    return getValue<double> (PitchPropertyId);
}

CvInputAndAmount ChannelProperties::getPitchCV ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PitchCVPropertyId));
}

int ChannelProperties::getPlayMode ()
{
    return getValue<int> (PlayModePropertyId);
}

double ChannelProperties::getPMIndex ()
{
    return getValue<double> (PMIndexPropertyId);
}

CvInputAndAmount ChannelProperties::getPMIndexMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (PMIndexModPropertyId));
}

int ChannelProperties::getPMSource ()
{
    return getValue<int> (PMSourcePropertyId);
}

double ChannelProperties::getRelease ()
{
    return getValue<double> (ReleasePropertyId);
}

CvInputAndAmount ChannelProperties::getReleaseMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (ReleaseModPropertyId));
}

bool ChannelProperties::getReverse ()
{
    return getValue<bool> (ReversePropertyId);
}

CvInputAndAmount ChannelProperties::getSampleStartMod ()
{
    return getCvInputAndValueFromString (getValue<juce::String> (SampleStartModPropertyId));
}

CvInputAndAmount ChannelProperties::getSampleEndMod ()
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

////////////////////////////////////////////////////////////////////
// get___Defaults
////////////////////////////////////////////////////////////////////
int ChannelProperties::getAliasingDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::AliasingId));
}

CvInputAndAmount ChannelProperties::getAliasingModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::AliasingModId)));
}

double ChannelProperties::getAttackDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::AttackId));
}

bool ChannelProperties::getAttackFromCurrentDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::AttackFromCurrentId)) != 0;
}

CvInputAndAmount ChannelProperties::getAttackModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::AttackModId)));
}

bool ChannelProperties::getAutoTriggerDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::AutoTriggerId)) != 0;
}

double ChannelProperties::getBitsDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::BitsId));
}

CvInputAndAmount ChannelProperties::getBitsModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::BitsModId)));
}

int ChannelProperties::getChannelModeDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ChannelModeId));
}

double ChannelProperties::getExpAMDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ExpAMId));
}

double ChannelProperties::getExpFMDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ExpFMId));
}

double ChannelProperties::getLevelDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LevelId));
}

double ChannelProperties::getLinAMDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LinAMId));
}

bool ChannelProperties::getLinAMisExtEnvDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LinAMisExtEnvId)) != 0;
}

double ChannelProperties::getLinFMDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LinFMId));
}

CvInputAndAmount ChannelProperties::getLoopLengthModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LoopLengthModId)));
}

int ChannelProperties::getLoopModeDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LoopModeId));
}

CvInputAndAmount ChannelProperties::getLoopStartModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::LoopStartModId)));
}

double ChannelProperties::getMixLevelDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::MixLevelId));
}

CvInputAndAmount ChannelProperties::getMixModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::MixModId)));
}

bool ChannelProperties::getMixModIsFaderDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::MixModIsFaderId)) != 0;
}

double ChannelProperties::getPanDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PanId));
}

CvInputAndAmount ChannelProperties::getPanModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PanModId)));
}

CvInputAndAmount ChannelProperties::getPhaseCVDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PhaseCVId)));
}

double ChannelProperties::getPitchDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PitchCVId));
}

CvInputAndAmount ChannelProperties::getPitchCVDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PitchCVId)));
}

int ChannelProperties::getPlayModeDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PlayModeId));
}

double ChannelProperties::getPMIndexDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PMIndexId));
}

CvInputAndAmount ChannelProperties::getPMIndexModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PMIndexModId)));
}

int ChannelProperties::getPMSourceDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::PMSourceId));
}

double ChannelProperties::getReleaseDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ReleaseId));
}

CvInputAndAmount ChannelProperties::getReleaseModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ReleaseModId)));
}

bool ChannelProperties::getReverseDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ReverseId)) != 0;
}

CvInputAndAmount ChannelProperties::getSampleStartModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::SampleStartModId)));
}

CvInputAndAmount ChannelProperties::getSampleEndModDefault ()
{
    return getCvInputAndValueFromString (ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::SampleEndModId)));
}

bool ChannelProperties::getSpliceSmoothingDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::SpliceSmoothingId)) != 0;
}

juce::String ChannelProperties::getXfadeGroupDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::XfadeGroupId));
}

juce::String ChannelProperties::getZonesCVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ZonesCVId));
}

int ChannelProperties::getZonesRTDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ChannelId, Parameter::Channel::ZonesRTId));
}

void ChannelProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == IndexPropertyId)
    {
        if (onIndexChange != nullptr)
            onIndexChange (getIndex ());
    }
    else if (property == AliasingPropertyId)
    {
        if (onAliasingChange != nullptr)
            onAliasingChange (getAliasing ());
    }
    else if (property == AliasingModPropertyId)
    {
        if (onAliasingModChange != nullptr)
            onAliasingModChange (getAliasingMod ());
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
