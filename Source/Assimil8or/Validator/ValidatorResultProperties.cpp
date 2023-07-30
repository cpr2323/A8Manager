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

void ValidatorResultProperties::updateType (juce::String resultType, bool includeSelfCallback)
{
    const auto curType { getType () };

    if (resultType == "" || curType == "error")
        return;
    if (curType == "")
    {
        setType (resultType, includeSelfCallback);
    }
    else if (curType == "info")
    {
        if (resultType != "")
            setType (resultType, includeSelfCallback);
    }
    else if (curType == "warning")
    {
        if (resultType != "" && resultType != "info")
            setType (resultType, includeSelfCallback);
    }
    else
    {
        jassertfalse;
    }
}

void ValidatorResultProperties::updateText (juce::String resultText, bool includeSelfCallback)
{
    if (resultText.isEmpty ())
        return;

    auto curText{ getText() };

    if (resultText.isNotEmpty ())
        curText += ", ";

    curText += resultText;
    setText (curText, includeSelfCallback);
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
