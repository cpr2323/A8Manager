#include "ZoneProperties.h"

void Assimil8orZoneProperties::initValueTree ()
{
    // normally in this function we create all of the properties
    // but, as the Assimil8or only writes out parameters that have changed from the defaults
    // we will emulate this by only adding properties when they change, or are in a preset file that is read in
}

void Assimil8orZoneProperties::setLevelOffset (float levelOffset, bool includeSelfCallback)
{
    setValue (levelOffset, LevelOffsetPropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setLoopLength (float loopLength, bool includeSelfCallback)
{
    setValue (loopLength, LoopLengthPropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setLoopStart (int loopStart, bool includeSelfCallback)
{
    setValue (loopStart, LoopStartPropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setMinVoltage (float minVoltage, bool includeSelfCallback)
{
    setValue (minVoltage, MinVoltagePropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setPitchOffset (float pitchOffset, bool includeSelfCallback)
{
    setValue (pitchOffset, PitchOffsetPropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setSample (juce::String sampleFileName, bool includeSelfCallback)
{
    setValue (sampleFileName, SamplePropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setSampleStart (int sampleStart, bool includeSelfCallback)
{
    setValue (sampleStart, SampleStartPropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setSampleEnd (int sampleEnd, bool includeSelfCallback)
{
    setValue (sampleEnd, SampleEndPropertyId, includeSelfCallback);
}

void Assimil8orZoneProperties::setSide (int side, bool includeSelfCallback)
{
    setValue (side, SidePropertyId, includeSelfCallback);
}

float Assimil8orZoneProperties::getLevelOffset ()
{
    return getValue<float> (LevelOffsetPropertyId);
}

float Assimil8orZoneProperties::getLoopLength ()
{
    return getValue<float> (LoopLengthPropertyId);
}

int Assimil8orZoneProperties::getLoopStart ()
{
    return getValue<int> (LoopStartPropertyId);
}

float Assimil8orZoneProperties::getMinVoltage ()
{
    return getValue<float> (MinVoltagePropertyId);
}

float Assimil8orZoneProperties::getPitchOffset ()
{
    return getValue<float> (PitchOffsetPropertyId);
}

juce::String Assimil8orZoneProperties::getSample ()
{
    return getValue<juce::String> (SamplePropertyId);
}

int Assimil8orZoneProperties::getSampleStart ()
{
    return getValue<int> (SampleStartPropertyId);
}

int Assimil8orZoneProperties::getSampleEnd ()
{
    return getValue<int> (SampleEndPropertyId);
}

int Assimil8orZoneProperties::getSide ()
{
    return getValue<int> (SidePropertyId);
}

void Assimil8orZoneProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
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
