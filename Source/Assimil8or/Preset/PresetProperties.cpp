#include "PresetProperties.h"
#include "DefaultHelpers.h"
#include "ParameterDataProperties.h"
#include "ParameterNames.h"

const auto kMaxChannels { 8 };

int PresetProperties::getNumChannels ()
{
    auto numChannels { 0 };
    forEachChannel ([&numChannels] (juce::ValueTree) { ++numChannels; return true; });
    return numChannels;
}

void PresetProperties::initValueTree ()
{
    // normally in this function we create all of the properties
    // but, as the Assimil8or only writes out parameters that have changed from the defaults
    // we will emulate this by only adding properties when they change, or are in a preset file that is read in, and removing them when they are set to the default value
    clear ();
}

void PresetProperties::clear ()
{
    data.removeAllChildren (nullptr);
    data.removeAllProperties (nullptr);

    const auto kDefaultPresetName { "New" };
    setIndex (1, false);
    setName (kDefaultPresetName, false);
}

juce::ValueTree PresetProperties::addChannel (int index)
{
    jassert (getNumChannels () < kMaxChannels);
    auto channelProperties { ChannelProperties::create (index) };
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

////////////////////////////////////////////////////////////////////
// set___
////////////////////////////////////////////////////////////////////
void PresetProperties::setIndex (int index, bool includeSelfCallback)
{
    setValue (index, IndexPropertyId, includeSelfCallback);
}

void PresetProperties::setData2AsCV (juce::String data2AsCv, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<juce::String> (data, Data2asCVPropertyId, data2AsCv, includeSelfCallback, this, [this] () { return getData2AsCVDefault (); });
}

void PresetProperties::setName (juce::String name, bool includeSelfCallback)
{
    setValue(name, NamePropertyId, includeSelfCallback);
}

void PresetProperties::setXfadeACV (juce::String cvInput, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<juce::String> (data, XfadeACVPropertyId, cvInput, includeSelfCallback, this, [this] () { return getXfadeACVDefault(); });
}

void PresetProperties::setXfadeAWidth (double width, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, XfadeAWidthPropertyId, width, includeSelfCallback, this, [this] () { return getXfadeAWidthDefault(); });
}

void PresetProperties::setXfadeBCV (juce::String cvInput, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<juce::String> (data, XfadeBCVPropertyId, cvInput, includeSelfCallback, this, [this] () { return getXfadeBCVDefault(); });
}

void PresetProperties::setXfadeBWidth (double width, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, XfadeBWidthPropertyId, width, includeSelfCallback, this, [this] () { return getXfadeBWidthDefault(); });
}

void PresetProperties::setXfadeCCV (juce::String cvInput, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<juce::String> (data, XfadeCCVPropertyId, cvInput, includeSelfCallback, this, [this] () { return getXfadeCCVDefault(); });
}

void PresetProperties::setXfadeCWidth (double width, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, XfadeCWidthPropertyId, width, includeSelfCallback, this, [this] () { return getXfadeCWidthDefault(); });
}

void PresetProperties::setXfadeDCV (juce::String cvInput, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<juce::String> (data, XfadeDCVPropertyId, cvInput, includeSelfCallback, this, [this] () { return getXfadeDCVDefault(); });
}

void PresetProperties::setXfadeDWidth (double width, bool includeSelfCallback)
{
    DefaultHelpers::setAndHandleDefault<double> (data, XfadeDWidthPropertyId, width, includeSelfCallback, this, [this] () { return getXfadeDWidthDefault(); });
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
    return DefaultHelpers::getAndHandleDefault<juce::String> (data, Data2asCVPropertyId, [this] () { return getData2AsCVDefault (); });
}

juce::String PresetProperties::getName ()
{
    return getValue<juce::String>(NamePropertyId);
}

juce::String PresetProperties::getXfadeACV ()
{
    return DefaultHelpers::getAndHandleDefault<juce::String> (data, XfadeACVPropertyId, [this] () { return getXfadeACVDefault(); });
}

double PresetProperties::getXfadeAWidth ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, XfadeAWidthPropertyId, [this] () { return getXfadeAWidthDefault(); });
}

juce::String PresetProperties::getXfadeBCV ()
{
    return DefaultHelpers::getAndHandleDefault<juce::String> (data, XfadeBCVPropertyId, [this] () { return getXfadeBCVDefault(); });
}

double PresetProperties::getXfadeBWidth ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, XfadeBWidthPropertyId, [this] () { return getXfadeBWidthDefault(); });
}

juce::String PresetProperties::getXfadeCCV ()
{
    return DefaultHelpers::getAndHandleDefault<juce::String> (data, XfadeCCVPropertyId, [this] () { return getXfadeCCVDefault(); });
}

double PresetProperties::getXfadeCWidth ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, XfadeCWidthPropertyId, [this] () { return getXfadeCWidthDefault(); });
}

juce::String PresetProperties::getXfadeDCV ()
{
    return DefaultHelpers::getAndHandleDefault<juce::String> (data, XfadeDCVPropertyId, [this] () { return getXfadeDCVDefault(); });
}

double PresetProperties::getXfadeDWidth ()
{
    return DefaultHelpers::getAndHandleDefault<double> (data, XfadeDWidthPropertyId, [this] () { return getXfadeDWidthDefault(); });
}

////////////////////////////////////////////////////////////////////
// get___Defaults
////////////////////////////////////////////////////////////////////
juce::String PresetProperties::getData2AsCVDefault ()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::Data2asCVId));
}

juce::String PresetProperties::getNameDefault()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::NameId));
}

juce::String PresetProperties::getXfadeACVDefault()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeACVId));
}

double PresetProperties::getXfadeAWidthDefault()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeAWidthId));
}

juce::String PresetProperties::getXfadeBCVDefault()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeBCVId));
}

double PresetProperties::getXfadeBWidthDefault()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeBWidthId));
}

juce::String PresetProperties::getXfadeCCVDefault()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeCCVId));
}

double PresetProperties::getXfadeCWidthDefault()
{
    return ParameterDataProperties::getDefaultDouble (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeCWidthId));
}

juce::String PresetProperties::getXfadeDCVDefault()
{
    return ParameterDataProperties::getDefaultString (parameterDataListProperties.getParameter (Section::PresetId, Parameter::Preset::XfadeDCVId));
}

double PresetProperties::getXfadeDWidthDefault()
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
