#include "PresetProperties.h"

const auto kNumChannels { 8 };
const auto kNumZones { 8 };

void PresetProperties::initValueTree ()
{
    for (auto channelIndex { 0 }; channelIndex < kNumChannels; ++channelIndex)
    {
        auto channel { ChannelProperties::create (channelIndex + 1) };
        data.addChild (channel, -1, nullptr);
        for (auto zoneIndex { 0 }; zoneIndex < kNumZones; ++zoneIndex)
            channel.addChild (ZoneProperties::create (zoneIndex + 1), -1, nullptr);
    }
}

int PresetProperties::getNumChannels ()
{
    auto numChannels { 0 };
    forEachChannel ([&numChannels] (juce::ValueTree, int) { ++numChannels; return true; });
    return numChannels;
}

void PresetProperties::copyTreeProperties (juce::ValueTree sourcePresetPropertiesVT, juce::ValueTree destinationPresetPropertiesVT)
{
    destinationPresetPropertiesVT.copyPropertiesFrom (sourcePresetPropertiesVT, nullptr);
    for (auto channelIndex { 0 }; channelIndex < sourcePresetPropertiesVT.getNumChildren (); ++channelIndex)
    {
        auto channelSource { sourcePresetPropertiesVT.getChild (channelIndex) };
        auto channelDestination { destinationPresetPropertiesVT.getChild (channelIndex) };
        channelDestination.copyPropertiesFrom (channelSource, nullptr);

        for (auto zoneIndex { 0 }; zoneIndex < channelSource.getNumChildren (); ++zoneIndex)
        {
            ZoneProperties zoneDestinationProperties (channelDestination.getChild (zoneIndex), ZoneProperties::WrapperType::owner, ZoneProperties::EnableCallbacks::no);
            zoneDestinationProperties.copyFrom (channelSource.getChild (zoneIndex), false);
        }
    }
}

void PresetProperties::forEachChannel (std::function<bool (juce::ValueTree channelVT, int channelIndex)> channelVTCallback)
{
    jassert (channelVTCallback != nullptr);
    auto curChannelIndex { 0 };
    ValueTreeHelpers::forEachChildOfType (data, ChannelProperties::ChannelTypeId, [this, &curChannelIndex, channelVTCallback] (juce::ValueTree channelVT)
    {
        auto keepIterating { channelVTCallback (channelVT, curChannelIndex) };
        ++curChannelIndex;
        return keepIterating;
    });
}

juce::ValueTree PresetProperties::getChannelVT (int channelIndex)
{
    jassert (channelIndex < getNumChannels ());
    juce::ValueTree requestedChannelVT;
    forEachChannel ([this, &requestedChannelVT, channelIndex] (juce::ValueTree channelVT, int curChannelIndex)
    {
        if (curChannelIndex == channelIndex)
        {
            requestedChannelVT = channelVT;
            return false;
        }
        return true;
    });
    jassert (requestedChannelVT.isValid ());
    return requestedChannelVT;
}

////////////////////////////////////////////////////////////////////
// set___
////////////////////////////////////////////////////////////////////
void PresetProperties::setId (int id, bool includeSelfCallback)
{
    setValue (id, IdPropertyId, includeSelfCallback);
}

void PresetProperties::setData2AsCV (juce::String data2AsCv, bool includeSelfCallback)
{
    setValue (data2AsCv, Data2asCVPropertyId, includeSelfCallback);
}

void PresetProperties::setMidiSetup (int midiSetupId, bool includeSelfCallback)
{
    setValue (midiSetupId, MidiSetpPropertyId, includeSelfCallback);
}

void PresetProperties::setName (juce::String name, bool includeSelfCallback)
{
    setValue (name, NamePropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeACV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeACVPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeAWidth (double width, bool includeSelfCallback)
{
    setValue (width, XfadeAWidthPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeBCV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeBCVPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeBWidth (double width, bool includeSelfCallback)
{
    setValue (width, XfadeBWidthPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeCCV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeCCVPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeCWidth (double width, bool includeSelfCallback)
{
    setValue (width, XfadeCWidthPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeDCV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeDCVPropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeDWidth (double width, bool includeSelfCallback)
{
    setValue (width, XfadeDWidthPropertyId, includeSelfCallback);
}

////////////////////////////////////////////////////////////////////
// get___
////////////////////////////////////////////////////////////////////
int PresetProperties::getId ()
{
    return getValue<int> (IdPropertyId);
}

juce::String PresetProperties::getData2AsCV ()
{
    return getValue<juce::String> (Data2asCVPropertyId);
}

int PresetProperties::getMidiSetup ()
{
    return getValue<int> (MidiSetpPropertyId);
}

juce::String PresetProperties::getName ()
{
    return getValue<juce::String> (NamePropertyId);
}

juce::String PresetProperties::getXfadeACV ()
{
    return getValue<juce::String> (XfadeACVPropertyId);
}

double PresetProperties::getXfadeAWidth ()
{
    return getValue<double> (XfadeAWidthPropertyId);
}

juce::String PresetProperties::getXfadeBCV ()
{
    return getValue<juce::String> (XfadeBCVPropertyId);
}

double PresetProperties::getXfadeBWidth ()
{
    return getValue<double> (XfadeBWidthPropertyId);
}

juce::String PresetProperties::getXfadeCCV ()
{
    return getValue<juce::String> (XfadeCCVPropertyId);
}

double PresetProperties::getXfadeCWidth ()
{
    return getValue<double> (XfadeCWidthPropertyId);
}

juce::String PresetProperties::getXfadeDCV ()
{
    return getValue<juce::String> (XfadeDCVPropertyId);
}

double PresetProperties::getXfadeDWidth ()
{
    return getValue<double> (XfadeDWidthPropertyId);
}

void PresetProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == IdPropertyId)
        {
            if (onIdChange != nullptr)
                onIdChange (getId ());
        }
        else if (property == Data2asCVPropertyId)
        {
            if (onData2AsCVChange != nullptr)
                onData2AsCVChange (getData2AsCV ());
        }
        else if (property == MidiSetpPropertyId)
        {
            if (onMidiSetupChange != nullptr)
                onMidiSetupChange (getMidiSetup ());
        }
        else if (property == NamePropertyId)
        {
            if (onNameChange != nullptr)
                onNameChange (getName ());
        }
        else if (property == XfadeACVPropertyId)
        {
            if (onXfadeACVChange != nullptr)
                onXfadeACVChange (getXfadeACV ());
        }
        else if (property == XfadeAWidthPropertyId)
        {
            if (onXfadeAWidthChange != nullptr)
                onXfadeAWidthChange (getXfadeAWidth ());
        }
        else if (property == XfadeBCVPropertyId)
        {
            if (onXfadeBCVChange != nullptr)
                onXfadeBCVChange (getXfadeBCV ());
        }
        else if (property == XfadeBWidthPropertyId)
        {
            if (onXfadeBWidthChange != nullptr)
                onXfadeBWidthChange (getXfadeBWidth ());
        }
        else if (property == XfadeCCVPropertyId)
        {
            if (onXfadeCCVChange != nullptr)
                onXfadeCCVChange (getXfadeCCV ());
        }
        else if (property == XfadeCWidthPropertyId)
        {
            if (onXfadeCWidthChange != nullptr)
                onXfadeCWidthChange (getXfadeCWidth ());
        }
        else if (property == XfadeDCVPropertyId)
        {
            if (onXfadeDCVChange != nullptr)
                onXfadeDCVChange (getXfadeDCV ());
        }
        else if (property == XfadeDWidthPropertyId)
        {
            if (onXfadeDWidthChange != nullptr)
                onXfadeDWidthChange (getXfadeDWidth ());
        }
    }
}
