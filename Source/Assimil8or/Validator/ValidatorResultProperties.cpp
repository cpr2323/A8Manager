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
    {
        jassertfalse;
        return newType;
    }

}
void ValidatorResultProperties::updateType (juce::String resultType, bool includeSelfCallback)
{
    setType (getNewTypeBasedOnPriority(resultType), includeSelfCallback);
}

void ValidatorResultProperties::updateText (juce::String resultText, bool includeSelfCallback)
{
    if (resultText.isEmpty ())
        return;

    auto curText{ getText() };

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

juce::ValueTree ValidatorResultProperties::addTag (juce::String tag, juce::String description)
{
    return {};
}

void ValidatorResultProperties::forEachTag (std::function<bool (juce::ValueTree tagVT)> tagVTCallback)
{

}

int ValidatorResultProperties::getNumTags ()
{
    return 0;
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
