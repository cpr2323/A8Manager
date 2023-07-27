#include "ZoneProperties.h"

void ZoneProperties::initValueTree ()
{
    // normally in this function we create all of the properties
    // but, as the Assimil8or only writes out parameters that have changed from the defaults
    // we will emulate this by only adding properties when they change, or are in a preset file that is read in
}

void ZoneProperties::setLevelOffset (float levelOffset, bool includeSelfCallback)
{
    setValue (levelOffset, LevelOffsetPropertyId, includeSelfCallback);
}

void ZoneProperties::setLoopLength (float loopLength, bool includeSelfCallback)
{
    setValue (loopLength, LoopLengthPropertyId, includeSelfCallback);
}

void ZoneProperties::setLoopStart (int loopStart, bool includeSelfCallback)
{
    setValue (loopStart, LoopStartPropertyId, includeSelfCallback);
}

void ZoneProperties::setMinVoltage (float minVoltage, bool includeSelfCallback)
{
    setValue (minVoltage, MinVoltagePropertyId, includeSelfCallback);
}

void ZoneProperties::setPitchOffset (float pitchOffset, bool includeSelfCallback)
{
    setValue (pitchOffset, PitchOffsetPropertyId, includeSelfCallback);
}

void ZoneProperties::setSample (juce::String sampleFileName, bool includeSelfCallback)
{
    setValue (sampleFileName, SamplePropertyId, includeSelfCallback);
}

void ZoneProperties::setSampleStart (int sampleStart, bool includeSelfCallback)
{
    setValue (sampleStart, SampleStartPropertyId, includeSelfCallback);
}

void ZoneProperties::setSampleEnd (int sampleEnd, bool includeSelfCallback)
{
    setValue (sampleEnd, SampleEndPropertyId, includeSelfCallback);
}

void ZoneProperties::setSide (int side, bool includeSelfCallback)
{
    setValue (side, SidePropertyId, includeSelfCallback);
}

float ZoneProperties::getLevelOffset ()
{
    return getValue<float> (LevelOffsetPropertyId);
}

float ZoneProperties::getLoopLength ()
{
    return getValue<float> (LoopLengthPropertyId);
}

int ZoneProperties::getLoopStart ()
{
    return getValue<int> (LoopStartPropertyId);
}

float ZoneProperties::getMinVoltage ()
{
    return getValue<float> (MinVoltagePropertyId);
}

float ZoneProperties::getPitchOffset ()
{
    return getValue<float> (PitchOffsetPropertyId);
}

juce::String ZoneProperties::getSample ()
{
    return getValue<juce::String> (SamplePropertyId);
}

int ZoneProperties::getSampleStart ()
{
    return getValue<int> (SampleStartPropertyId);
}

int ZoneProperties::getSampleEnd ()
{
    return getValue<int> (SampleEndPropertyId);
}

int ZoneProperties::getSide ()
{
    return getValue<int> (SidePropertyId);
}

juce::ValueTree ZoneProperties::create ()
{
    ZoneProperties zoneProperties;
    zoneProperties.wrap ({}, ValueTreeWrapper::WrapperType::owner, ValueTreeWrapper::EnableCallbacks::no);
    return zoneProperties.getValueTree ();
}

void ZoneProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == LevelOffsetPropertyId)
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
