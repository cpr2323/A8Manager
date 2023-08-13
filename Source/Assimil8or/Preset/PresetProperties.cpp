#include "PresetProperties.h"
#include "ParameterNames.h"

const auto kMaxChannels { 8 };

// TODO - refactor idea
template <typename T>
void setAndHandleDefault (juce::ValueTree vt, juce::Identifier id, T value, bool includeSelfCallback, juce::ValueTree::Listener* vtListener, std::function<T ()> getDefaultValue)
{
    jassert (vtListener != nullptr);
    jassert (getDefaultValue != nullptr);
    if (includeSelfCallback)
        vt.setProperty (id, value, nullptr);
    else
        vt.setPropertyExcludingListener (vtListener, id, value, nullptr);
    if (value == getDefaultValue ())
        vt.removeProperty (id, nullptr);
}

template <typename T>
T getAndHandleDefault (const juce::ValueTree vt, const juce::Identifier id, std::function<T ()> getDefaultValue)
{
    jassert (getDefaultValue != nullptr);
    if (vt.hasProperty (id))
        return vt.getProperty (id);
    else
        return getDefaultValue ();
}

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
    setAndHandleDefault<juce::String> (data, Data2asCVPropertyId, data2AsCv, includeSelfCallback, this, [this] () { return getData2AsCVDefault (); });
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
    return getAndHandleDefault<juce::String> (data, Data2asCVPropertyId, [this] () { return getData2AsCVDefault (); });
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
