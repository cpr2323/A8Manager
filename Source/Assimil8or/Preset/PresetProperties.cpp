#include "PresetProperties.h"
#include "ChannelProperties.h"

void Assimil8orPresetProperties::initValueTree ()
{
    // none of the properties are being created by default, as the Assimil8or only writes out parameters
    // that have changed from the defaults
}

void Assimil8orPresetProperties::forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback)
{
    jassert (channelVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, Assimil8orChannelProperties::ChannelTypeId, [this, channelVTCallback] (juce::ValueTree channelVT)
    {
        channelVTCallback (channelVT);
        return true;
    });
}

void Assimil8orPresetProperties::setData2AsCV (bool data2AsCv, bool includeSelfCallback)
{
    setValue (data2AsCv, Data2asCVPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setName (juce::String name, bool includeSelfCallback)
{
    setValue (name, NamePropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeACV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeACVPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeAWidth (float width, bool includeSelfCallback)
{
    setValue (width, XfadeAWidthPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeBCV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeBCVPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeBWidth (float width, bool includeSelfCallback)
{
    setValue (width, XfadeBWidthPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeCCV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeCCVPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeCWidth (float width, bool includeSelfCallback)
{
    setValue (width, XfadeCWidthPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeDCV (juce::String cvInput, bool includeSelfCallback)
{
    setValue (cvInput, XfadeDCVPropertyId, includeSelfCallback);
}

void Assimil8orPresetProperties::setXfadeDWidth (float width, bool includeSelfCallback)
{
    setValue (width, XfadeDWidthPropertyId, includeSelfCallback);
}

bool Assimil8orPresetProperties::getData2AsCV ()
{
    return getValue<bool> (Data2asCVPropertyId);
}

juce::String Assimil8orPresetProperties::getName ()
{
    return getValue<juce::String> (NamePropertyId);
}

juce::String Assimil8orPresetProperties::getXfadeACV ()
{
    return getValue<juce::String> (XfadeACVPropertyId);
}

float Assimil8orPresetProperties::getXfadeAWidth ()
{
    return getValue<float> (XfadeAWidthPropertyId);
}

juce::String Assimil8orPresetProperties::getXfadeBCV ()
{
    return getValue<juce::String> (XfadeBCVPropertyId);
}

float Assimil8orPresetProperties::getXfadeBWidth ()
{
    return getValue<float> (XfadeBWidthPropertyId);
}

juce::String Assimil8orPresetProperties::getXfadeCCV ()
{
    return getValue<juce::String> (XfadeCCVPropertyId);
}

float Assimil8orPresetProperties::getXfadeCWidth ()
{
    return getValue<float> (XfadeCWidthPropertyId);
}

juce::String Assimil8orPresetProperties::getXfadeDCV ()
{
    return getValue<juce::String> (XfadeDCVPropertyId);
}

float Assimil8orPresetProperties::getXfadeDWidth ()
{
    return getValue<float> (XfadeDWidthPropertyId);
}

void Assimil8orPresetProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{

}
