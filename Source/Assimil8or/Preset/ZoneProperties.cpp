#include "ZoneProperties.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/DumpStack.h"

juce::ValueTree ZoneProperties::create (int id)
{
    ZoneProperties zoneProperties;
    zoneProperties.setId (id, false);
    return zoneProperties.getValueTree ();
}

void ZoneProperties::copyFrom (juce::ValueTree sourceVT)
{
    ZoneProperties sourceZoneProperties (sourceVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    setSample (sourceZoneProperties.getSample (), false);
    setLevelOffset (sourceZoneProperties.getLevelOffset (), false);
    setLoopLength (sourceZoneProperties.getLoopLength ().value_or (-1.0), false);
    setLoopStart (sourceZoneProperties.getLoopStart ().value_or (-1), false);
    setMinVoltage (sourceZoneProperties.getMinVoltage (), false);
    setPitchOffset (sourceZoneProperties.getPitchOffset (), false);
    setSampleEnd (sourceZoneProperties.getSampleEnd ().value_or (-1), false);
    setSampleStart (sourceZoneProperties.getSampleStart ().value_or (-1), false);
    setSide (sourceZoneProperties.getSide (), false);
}

////////////////////////////////////////////////////////////////////
// set___
////////////////////////////////////////////////////////////////////
void ZoneProperties::setId (int id, bool includeSelfCallback)
{
    setValue (id, IdPropertyId, includeSelfCallback);
}

void ZoneProperties::setLevelOffset (double levelOffset, bool includeSelfCallback)
{
    setValue (levelOffset, LevelOffsetPropertyId, includeSelfCallback);
}

void ZoneProperties::setLoopLength (double loopLength, bool includeSelfCallback)
{
    setValue (loopLength, LoopLengthPropertyId, includeSelfCallback);
}

void ZoneProperties::setLoopStart (juce::int64 loopStart, bool includeSelfCallback)
{
    setValue (loopStart, LoopStartPropertyId, includeSelfCallback);
}

void ZoneProperties::setMinVoltage (double minVoltage, bool includeSelfCallback)
{
    //juce::Logger::outputDebugString ("[" + juce::String (getId ()) + "]: " + juce::String (minVoltage));
    //dumpStacktrace (30, [this] (juce::String text) { juce::Logger::outputDebugString (text); });
    setValue (minVoltage, MinVoltagePropertyId, includeSelfCallback);
}

void ZoneProperties::setPitchOffset (double pitchOffset, bool includeSelfCallback)
{
    setValue (pitchOffset, PitchOffsetPropertyId, includeSelfCallback);
}

void ZoneProperties::setSample (juce::String sampleFileName, bool includeSelfCallback)
{
    //DebugLog ("ZoneProperties", "ZoneProperties[" + juce::String (getId ()) + "]::setSample: '" + sampleFileName + "'");
    setValue (sampleFileName, SamplePropertyId, includeSelfCallback);
}

void ZoneProperties::setSampleStart (juce::int64 sampleStart, bool includeSelfCallback)
{
    setValue (sampleStart, SampleStartPropertyId, includeSelfCallback);
}

void ZoneProperties::setSampleEnd (juce::int64 sampleEnd, bool includeSelfCallback)
{
    setValue (sampleEnd, SampleEndPropertyId, includeSelfCallback);
}

void ZoneProperties::setSide (int side, bool includeSelfCallback)
{
    setValue (side, SidePropertyId, includeSelfCallback);
}

////////////////////////////////////////////////////////////////////
// get___
////////////////////////////////////////////////////////////////////
int ZoneProperties::getId ()
{
    return getValue<int> (IdPropertyId);
}

double ZoneProperties::getLevelOffset ()
{
    return getValue<double> (LevelOffsetPropertyId);
}

std::optional<double> ZoneProperties::getLoopLength ()
{
    const auto loopLength { getValue<double> (LoopLengthPropertyId) };
    if (loopLength == -1.0) // -1 indicates uninitialized
        return {};
    return loopLength;
}

std::optional <juce::int64> ZoneProperties::getLoopStart ()
{
    const auto loopStart { getValue<juce::int64> (LoopStartPropertyId) };
    if (loopStart == -1) // -1 indicates uninitialized
        return {};
    return loopStart;
}

double ZoneProperties::getMinVoltage ()
{
    return getValue<double> (MinVoltagePropertyId);
}

double ZoneProperties::getPitchOffset ()
{
    return getValue<double> (PitchOffsetPropertyId);
}

juce::String ZoneProperties::getSample ()
{
    return getValue<juce::String> (SamplePropertyId);
}

std::optional <juce::int64> ZoneProperties::getSampleStart ()
{
    const auto sampleStart { getValue<juce::int64> (SampleStartPropertyId) };
    if (sampleStart == -1) // -1 indicate uninitialized
        return {};
    return sampleStart;
}

std::optional <juce::int64> ZoneProperties::getSampleEnd ()
{
    const auto sampleEnd { getValue<juce::int64> (SampleEndPropertyId) };
    if (sampleEnd == -1) // -1 indicate uninitialized
        return {};
    return sampleEnd;
}

int ZoneProperties::getSide ()
{
    return getValue<int> (SidePropertyId);
}

void ZoneProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == IdPropertyId)
        {
            if (onIdChange!= nullptr)
                onIdChange (getId ());
        }
        else if (property == LevelOffsetPropertyId)
        {
            if (onLevelOffsetChange != nullptr)
                onLevelOffsetChange (getLevelOffset ());
        }
        else if (property == LoopLengthPropertyId)
        {
            if (onLoopLengthChange != nullptr)
                onLoopLengthChange (getLoopLength ());
        }
        else if (property == LoopStartPropertyId)
        {
            if (onLoopStartChange != nullptr)
                onLoopStartChange (getLoopStart ());
        }
        else if (property == MinVoltagePropertyId)
        {
            if (onMinVoltageChange != nullptr)
                onMinVoltageChange (getMinVoltage ());
        }
        else if (property == PitchOffsetPropertyId)
        {
            if (onPitchOffsetChange != nullptr)
                onPitchOffsetChange (getPitchOffset ());
        }
        else if (property == SamplePropertyId)
        {
            if (onSampleChange != nullptr)
                onSampleChange (getSample ());
        }
        else if (property == SampleStartPropertyId)
        {
            if (onSampleStartChange != nullptr)
                onSampleStartChange (getSampleStart ());
        }
        else if (property == SampleEndPropertyId)
        {
            if (onSampleEndChange != nullptr)
                onSampleEndChange (getSampleEnd ());
        }
        else if (property == SidePropertyId)
        {
            if (onSideChange != nullptr)
                onSideChange (getSide ());
        }
    }
}
