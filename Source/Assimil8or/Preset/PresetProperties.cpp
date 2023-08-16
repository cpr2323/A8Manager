#include "PresetProperties.h"
#include "ParameterDataProperties.h"
#include "ParameterNames.h"

const auto kMaxChannels { 8 };

void PresetProperties::initValueTree ()
{
    for (auto channelIndex { 0 }; channelIndex < kMaxChannels; ++channelIndex)
        addChannel (channelIndex);
}

int PresetProperties::getNumChannels ()
{
    auto numChannels { 0 };
    forEachChannel ([&numChannels] (juce::ValueTree) { ++numChannels; return true; });
    return numChannels;
}

void PresetProperties::clear ()
{
    const auto kDefaultPresetName { "New" };
    setIndex (1, false);
    setName (kDefaultPresetName, false);

    setData2AsCV (getData2AsCVDefault (), false);
    //setName (juce::String name, false); this is currently set in the clear function
    setXfadeACV (getXfadeACVDefault (), false);
    setXfadeAWidth (getXfadeAWidthDefault (), false);
    setXfadeBCV (getXfadeBCVDefault (), false);
    setXfadeBWidth (getXfadeBWidthDefault (), false);
    setXfadeCCV (getXfadeCCVDefault (), false);
    setXfadeCWidth (getXfadeCWidthDefault (), false);
    setXfadeDCV (getXfadeDCVDefault (), false);
    setXfadeDWidth (getXfadeDWidthDefault (), false);

    forEachChannel ([this] (juce::ValueTree channelPropertiesVT)
    {
        ChannelProperties channelProperties (channelPropertiesVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        channelProperties.clear ();
        return true;
    });
}

juce::ValueTree PresetProperties::addChannel (int index)
{
    //jassert (index < getNumChannels ()); // this breaks when running from the ctor
    auto channelProperties { ChannelProperties::create (index + 1) };
    data.addChild (channelProperties, -1, nullptr);
    return channelProperties;
}

void PresetProperties::forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback)
{
    jassert (channelVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, ChannelProperties::ChannelTypeId, [this, channelVTCallback] (juce::ValueTree channelVT)
    {
        return channelVTCallback (channelVT);
    });
}

juce::ValueTree PresetProperties::getChannelVT (int channelIndex)
{
    jassert (channelIndex < getNumChannels ());
    juce::ValueTree requestedChannelVT;
    auto curChannelIndex { 0 };
    forEachChannel ([this, &requestedChannelVT, &curChannelIndex, channelIndex] (juce::ValueTree channelVT)
    {
        if (curChannelIndex == channelIndex)
        {
            requestedChannelVT = channelVT;
            return false;
        }
        ++curChannelIndex;
        return true;
    });
    jassert (requestedChannelVT.isValid ());
    return requestedChannelVT;
}

////////////////////////////////////////////////////////////////////
// set___
////////////////////////////////////////////////////////////////////
void PresetProperties::setIndex (int index, bool includeSelfCallback)
{
    setValue (index, IndexPropertyId, includeSelfCallback);
}

void PresetProperties::setData2AsCV (juce::String data2AsCv, bool includeSelfCallback)
{
    setValue (data2AsCv, Data2asCVPropertyId, includeSelfCallback);
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
int PresetProperties::getIndex ()
{
    return getValue<int> (IndexPropertyId);
}

juce::String PresetProperties::getData2AsCV ()
{
    return getValue<juce::String> (Data2asCVPropertyId);
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

////////////////////////////////////////////////////////////////////
// get___Defaults
////////////////////////////////////////////////////////////////////
juce::String PresetProperties::getData2AsCVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::Data2asCVId));
}

juce::String PresetProperties::getNameDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::NameId));
}

juce::String PresetProperties::getXfadeACVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeACVId));
}

double PresetProperties::getXfadeAWidthDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeAWidthId));
}

juce::String PresetProperties::getXfadeBCVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeBCVId));
}

double PresetProperties::getXfadeBWidthDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeBWidthId));
}

juce::String PresetProperties::getXfadeCCVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeCCVId));
}

double PresetProperties::getXfadeCWidthDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeCWidthId));
}

juce::String PresetProperties::getXfadeDCVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeDCVId));
}

double PresetProperties::getXfadeDWidthDefault ()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeDWidthId));
}

void PresetProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == IndexPropertyId)
        {
            if (onIndexChange != nullptr)
                onIndexChange (getIndex ());
        }
        else if (property == Data2asCVPropertyId)
        {
            if (onData2AsCVChange != nullptr)
                onData2AsCVChange (getData2AsCV ());
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
