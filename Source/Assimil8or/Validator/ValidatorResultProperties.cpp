#include "ValidatorResultProperties.h"

void ValidatorResultProperties::initValueTree ()
{
    reset (false);
}

void ValidatorResultProperties::setType (juce::String resultType, bool includeSelfCallback)
{
    setValue (resultType, TypePropertyId, includeSelfCallback);
}

void ValidatorResultProperties::setText (juce::String resultText, bool includeSelfCallback)
{
    setValue (resultText, TextPropertyId, includeSelfCallback);
}

void ValidatorResultProperties::reset (bool includeSelfCallback)
{
    setType ({}, includeSelfCallback);
    setText ({}, includeSelfCallback);
}

juce::String ValidatorResultProperties::getNewTypeBasedOnPriority (juce::String newType)
{
    const auto curType { getType () };

    if (newType == ResultTypeNone || curType == ResultTypeError)
        return curType;
    if (curType == ResultTypeNone)
    {
        return newType;
    }
    else if (curType == ResultTypeInfo)
    {
        if (newType != ResultTypeNone)
            return newType;
    }
    else if (curType == ResultTypeWarning)
    {
        if (newType != ResultTypeNone && newType != ResultTypeInfo)
            return newType;
    }

    return curType;

}
void ValidatorResultProperties::updateType (juce::String resultType, bool includeSelfCallback)
{
    setType (getNewTypeBasedOnPriority (resultType), includeSelfCallback);
}

void ValidatorResultProperties::updateText (juce::String resultText, bool includeSelfCallback)
{
    if (resultText.isEmpty ())
        return;

    auto curText { getText () };

    if (curText.isNotEmpty ())
        curText += ", ";

    curText += resultText;
    setText (curText, includeSelfCallback);
}

void ValidatorResultProperties::update (juce::String resultType, juce::String resultText, bool includeSelfCallback)
{
    updateType (resultType, includeSelfCallback);
    updateText (resultText, includeSelfCallback);
}

juce::String ValidatorResultProperties::getType ()
{
    return getValue<juce::String> (TypePropertyId);
}

juce::String ValidatorResultProperties::getText ()
{
    return getValue<juce::String> (TextPropertyId);
}

void ValidatorResultProperties::addFixerEntry (juce::String fixerType, juce::String fileName)
{
    FixerEntryProperties fixerEntryProperties;
    fixerEntryProperties.setType (fixerType, false);
    fixerEntryProperties.setFileName (fileName, false);
    data.addChild (fixerEntryProperties.getValueTree (), -1, nullptr);
}

void ValidatorResultProperties::forEachFixerEntry (std::function<bool (juce::ValueTree fixerEntryVT)> fixerEntryVTCallback)
{
    jassert (fixerEntryVTCallback != nullptr);
    ValueTreeHelpers::forEachChildOfType (data, FixerEntryProperties::FixerEntryTypeId, [this, fixerEntryVTCallback] (juce::ValueTree validatorResultsVT)
    {
        return fixerEntryVTCallback (validatorResultsVT);
    });
}

int ValidatorResultProperties::getNumFixerEntries ()
{
    auto numFixerEntries { 0 };
    forEachFixerEntry ([&numFixerEntries] (juce::ValueTree) { ++numFixerEntries; return true; });
    return numFixerEntries;
}

void ValidatorResultProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == TypePropertyId)
        {
            if (onTypeChange != nullptr)
                onTypeChange (getType ());
        }
        else if (property == TextPropertyId)
        {
            if (onTextChange != nullptr)
                onTextChange (getText ());
        }
    }
}
