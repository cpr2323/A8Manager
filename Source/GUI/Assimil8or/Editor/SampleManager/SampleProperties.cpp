#include "SampleProperties.h"

void SampleProperties::initValueTree ()
{
    setAudioBufferPtr (nullptr, false);
    setBitsPerSample (0, false);
    setLengthInSamples (0, false);
    setName ("", false);
    setNumChannels (0, false);
    setStatus (SampleData::SampleDataStatus::uninitialized, false);
}

void SampleProperties::setName (juce::String name, bool includeSelfCallback)
{
    setValue (name, NamePropertyId, includeSelfCallback);
}

void SampleProperties::setBitsPerSample (int bitsPerSample, bool includeSelfCallback)
{
    setValue (bitsPerSample, BitsPerSamplePropertyId, includeSelfCallback);
}

void SampleProperties::setNumChannels (int numChannels, bool includeSelfCallback)
{
    setValue (numChannels, NumChannelsPropertyId, includeSelfCallback);
}

void SampleProperties::setLengthInSamples (juce::int64 lengthInSamples, bool includeSelfCallback)
{
    setValue (lengthInSamples, LengthInSamplesPropertyId, includeSelfCallback);
}

void SampleProperties::setStatus (SampleData::SampleDataStatus status, bool includeSelfCallback)
{
    setValue (static_cast<int> (status), StatusPropertyId, includeSelfCallback);
}

void SampleProperties::setAudioBufferPtr (AudioBufferType* audioBufferPtr, bool includeSelfCallback)
{
    setValue (audioBufferPtr, AudioBufferPtrPropertyId, includeSelfCallback);
}

SampleData::SampleDataStatus SampleProperties::getStatus ()
{
    return static_cast<SampleData::SampleDataStatus>(getValue<int> (StatusPropertyId));
}

juce::String SampleProperties::getName ()
{
    return getValue<juce::String> (NamePropertyId);
}

int SampleProperties::getBitsPerSample ()
{
    return getValue<int> (BitsPerSamplePropertyId);
}

int SampleProperties::getNumChannels ()
{
    return getValue<int> (NumChannelsPropertyId);
}

juce::int64 SampleProperties::getLengthInSamples ()
{
    return getValue<juce::int64> (LengthInSamplesPropertyId);
}

AudioBufferType* SampleProperties::getAudioBufferPtr ()
{
    return getValue<AudioBufferType*> (AudioBufferPtrPropertyId, data);
}

void SampleProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == NamePropertyId)
        {
            if (onNameChange != nullptr)
                onNameChange (getName ());
        }
        else if (property == BitsPerSamplePropertyId)
        {
            if (onBitsPerSampleChange != nullptr)
                onBitsPerSampleChange (getBitsPerSample ());
        }
        else if (property == NumChannelsPropertyId)
        {
            if (onNumChannelsChange != nullptr)
                onNumChannelsChange (getNumChannels ());
        }
        else if (property == LengthInSamplesPropertyId)
        {
            if (onLengthInSamplesChange != nullptr)
                onLengthInSamplesChange (getLengthInSamples ());
        }
        else if (property == AudioBufferPtrPropertyId)
        {
            if (onAudioBufferPtrChange != nullptr)
                onAudioBufferPtrChange (getAudioBufferPtr ());
        }
        else if (property == StatusPropertyId)
        {
            if (onStatusChange != nullptr)
                onStatusChange (getStatus ());
        }
    }
}
