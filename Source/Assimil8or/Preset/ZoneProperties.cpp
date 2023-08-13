#include "ZoneProperties.h"
#include "DefaultHelpers.h"
#include "ParameterDataProperties.h"
#include "ParameterNames.h"

void ZoneProperties::initValueTree ()
{
    // normally in this function we create all of the properties
    // but, as the Assimil8or only writes out parameters that have changed from the defaults
    // we will emulate this by only adding properties when they change, or are in a preset file that is read in
}

juce::ValueTree ZoneProperties::create (int index)
{
    ZoneProperties zoneProperties;
    zoneProperties.setIndex (index, false);
    return zoneProperties.getValueTree ();
}

////////////////////////////////////////////////////////////////////
// set___
////////////////////////////////////////////////////////////////////
void ZoneProperties::setIndex (int index, bool includeSelfCallback)
{
    setValue (index, IndexPropertyId, includeSelfCallback);
}

void ZoneProperties::setLevelOffset (double levelOffset, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, LevelOffsetPropertyId, levelOffset, includeSelfCallback, this, [this] () { return getLevelOffsetDefault (); });
}

void ZoneProperties::setLoopLength (double loopLength, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, LoopLengthPropertyId, loopLength, includeSelfCallback, this, [this] () { return getLoopLengthDefault (); });
}

void ZoneProperties::setLoopStart (int loopStart, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, LoopStartPropertyId, loopStart, includeSelfCallback, this, [this] () { return getLoopStartDefault (); });
}

void ZoneProperties::setMinVoltage (double minVoltage, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, MinVoltagePropertyId, minVoltage, includeSelfCallback, this, [this] () { return getMinVoltageDefault (); });
}

void ZoneProperties::setPitchOffset (double pitchOffset, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, PitchOffsetPropertyId, pitchOffset, includeSelfCallback, this, [this] () { return getPitchOffsetDefault (); });
}

void ZoneProperties::setSample (juce::String sampleFileName, bool includeSelfCallback)
{
    setValue (sampleFileName, SamplePropertyId, includeSelfCallback);
}

void ZoneProperties::setSampleStart (int sampleStart, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<int> (data, SampleStartPropertyId, sampleStart, includeSelfCallback, this, [this] () { return getSampleStartDefault (); });
}

void ZoneProperties::setSampleEnd (int sampleEnd, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<int> (data, SampleEndPropertyId, sampleEnd, includeSelfCallback, this, [this] () { return getSampleEndDefault (); });
}

void ZoneProperties::setSide (int side, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<int> (data, SidePropertyId, side, includeSelfCallback, this, [this] () { return getSideDefault (); });
}

////////////////////////////////////////////////////////////////////
// get___
////////////////////////////////////////////////////////////////////
int ZoneProperties::getIndex ()
{
    return getValue<int> (IndexPropertyId);
}

double ZoneProperties::getLevelOffset ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, LevelOffsetPropertyId, [this] () { return getLevelOffsetDefault (); });
}

double ZoneProperties::getLoopLength ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, LoopLengthPropertyId, [this] () { return getLoopLengthDefault (); });
}

int ZoneProperties::getLoopStart ()
{
    return DefaultHelpers::getAndHandleDefault<int> (data, LoopStartPropertyId, [this] () { return getLoopStartDefault (); });
}

double ZoneProperties::getMinVoltage ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, MinVoltagePropertyId, [this] () { return getMinVoltageDefault (); });
}

double ZoneProperties::getPitchOffset ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, PitchOffsetPropertyId, [this] () { return getPitchOffsetDefault (); });
}

juce::String ZoneProperties::getSample ()
{
    return getValue<juce::String> (SamplePropertyId);
}

int ZoneProperties::getSampleStart ()
{
    return DefaultHelpers::getAndHandleDefault<int> (data, SampleStartPropertyId, [this] () { return getSampleStartDefault (); });
}

int ZoneProperties::getSampleEnd ()
{
    return DefaultHelpers::getAndHandleDefault<int> (data, SampleEndPropertyId, [this] () { return getSampleEndDefault (); });
}

int ZoneProperties::getSide ()
{
    return DefaultHelpers::getAndHandleDefault<int> (data, SidePropertyId, [this] () { return getSideDefault (); });
}

////////////////////////////////////////////////////////////////////
// get___Defaults
////////////////////////////////////////////////////////////////////
double ZoneProperties::getLevelOffsetDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::LevelOffsetId));
}

double ZoneProperties::getLoopLengthDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::LoopLengthId));
}

int ZoneProperties::getLoopStartDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::LoopStartId));
}

double ZoneProperties::getMinVoltageDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::MinVoltageId));
}

double ZoneProperties::getPitchOffsetDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::PitchOffsetId));
}

int ZoneProperties::getSampleStartDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::SampleStartId));
}

int ZoneProperties::getSampleEndDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::SampleEndId));
}

int ZoneProperties::getSideDefault ()
{
    return ParameterDataProperties::getDefaultInt (parameterDataListProperties.getParameter (Section::ZoneId, Parameter::Zone::SideId));
}

void ZoneProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == IndexPropertyId)
        {
            if (onIndexChange!= nullptr)
                onIndexChange (getIndex ());
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
