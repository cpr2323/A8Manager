#include "AudioPlayerProperties.h"

void AudioPlayerProperties::initValueTree ()
{
    setPlayState (PlayState::stop, false);
    setSampleSource (-1, -1, false);
    setSamplePointsSelector (SamplePointsSelector::SamplePoints, false);
    setValue (0, TriggerPropertyId, false);
    setGate (false, false);
}

void AudioPlayerProperties::setPlayState (PlayState playState, bool includeSelfCallback)
{
    setValue (static_cast<int> (playState), PlayStatePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setSampleSource (int channelIndex, int zoneIndex, bool includeSelfCallback)
{
    const auto channelAndZoneIndiciesString { juce::String (channelIndex) + "," + juce::String (zoneIndex) };
    setValue (channelAndZoneIndiciesString, SampleSourcePropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setSamplePointsSelector (SamplePointsSelector samplePointsSelector, bool includeSelfCallback)
{
    setValue (static_cast<int> (samplePointsSelector), SamplePointsSelectorPropertyId, includeSelfCallback);
}

void AudioPlayerProperties::trigger (bool includeSelfCallback)
{
    // a count, so that every trigger is a change, and reaches the listeners
    setValue (getValue<int> (TriggerPropertyId) + 1, TriggerPropertyId, includeSelfCallback);
}

void AudioPlayerProperties::setGate (bool gateHigh, bool includeSelfCallback)
{
    setValue (gateHigh, GatePropertyId, includeSelfCallback);
}

AudioPlayerProperties::PlayState AudioPlayerProperties::getPlayState ()
{
    return static_cast<PlayState> (getValue<int> (PlayStatePropertyId));
}

std::tuple<int, int> AudioPlayerProperties::getSampleSource ()
{
    const auto channelAndZoneIndiciesStrings { juce::StringArray::fromTokens (getValue<juce::String> (SampleSourcePropertyId), ",", "") };
    jassert (channelAndZoneIndiciesStrings.size () == 2);
    return { channelAndZoneIndiciesStrings[0].getIntValue (), channelAndZoneIndiciesStrings [1].getIntValue () };
}

AudioPlayerProperties::SamplePointsSelector AudioPlayerProperties::getSamplePointsSelector ()
{
    return static_cast<SamplePointsSelector> (getValue<int> (SamplePointsSelectorPropertyId));
}

bool AudioPlayerProperties::getGate ()
{
    return getValue<bool> (GatePropertyId);
}

void AudioPlayerProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == PlayStatePropertyId)
        {
            if (onPlayStateChange != nullptr)
                onPlayStateChange (getPlayState ());
        }
        else if (property == SampleSourcePropertyId)
        {
            if (onSampleSourceChanged != nullptr)
                onSampleSourceChanged (getSampleSource ());
        }
        else if (property == SamplePointsSelectorPropertyId)
        {
            if (onSamplePointsSelectorChanged != nullptr)
                onSamplePointsSelectorChanged (getSamplePointsSelector ());
        }
        else if (property == TriggerPropertyId)
        {
            if (onTrigger != nullptr)
                onTrigger ();
        }
        else if (property == GatePropertyId)
        {
            if (onGateChange != nullptr)
                onGateChange (getGate ());
        }
    }
}

