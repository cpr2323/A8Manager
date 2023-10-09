/*
  ==============================================================================

    GuiProperties.cpp
    Created: 9 Oct 2023 7:41:26am
    Author:  cpran

  ==============================================================================
*/

#include "GuiProperties.h"

void GuiProperties::initValueTree ()
{
    setPosition (0, 0, false);
    setSize (100, 100, false);
}

void GuiProperties::setPosition (int x, int y, bool includeSelfCallback)
{
    setValue (juce::String (x) + "," + juce::String (y), PositionPropertyId, includeSelfCallback);
}

void GuiProperties::setSize (int width, int height, bool includeSelfCallback)
{
    setValue (juce::String (width) + "," + juce::String (height), SizePropertyId, includeSelfCallback);

}

void GuiProperties::setPaneSizes (int width, int height, bool includeSelfCallback)
{

}

std::tuple<int, int> GuiProperties::getPosition ()
{
    const auto values { juce::StringArray::fromTokens (getValue<juce::String> (PositionPropertyId), ",", {}) };
    jassert (values.size () == 2);
    return { values [0].getIntValue (), values [1].getIntValue () };
}

std::tuple<int, int> GuiProperties::getSize ()
{
    const auto values { juce::StringArray::fromTokens (getValue<juce::String> (SizePropertyId), ",", {}) };
    jassert (values.size () == 2);
    return { values [0].getIntValue (), values [1].getIntValue () };
}

void GuiProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{

}
