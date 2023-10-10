#include "GuiProperties.h"

void GuiProperties::initValueTree ()
{
    setPosition (0, 0, false);
    setSize (100, 100, false);
    setPaneSizes (140, 170, 480, false);
}

void GuiProperties::processValueTree ()
{
    if (! data.hasProperty (PaneSizesPropertyId))
        setPaneSizes (140, 170, 480, false);
    if (! data.hasProperty (SizePropertyId))
        setSize (1117, 585, false);
}

void GuiProperties::setPosition (int x, int y, bool includeSelfCallback)
{
    setValue (juce::String (x) + "," + juce::String (y), PositionPropertyId, includeSelfCallback);
}

void GuiProperties::setSize (int width, int height, bool includeSelfCallback)
{
    setValue (juce::String (width) + "," + juce::String (height), SizePropertyId, includeSelfCallback);

}

void GuiProperties::setPaneSizes (double pane1Size, double pane2Size, double pane3Size, bool includeSelfCallback)
{
    const auto paneSizes { juce::String (pane1Size) + "," + juce::String (pane2Size) + "," + juce::String (pane3Size) };
    setValue (paneSizes, PaneSizesPropertyId, includeSelfCallback);
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

std::tuple<double, double, double> GuiProperties::getPaneSizes ()
{
    const auto values { juce::StringArray::fromTokens (getValue<juce::String> (PaneSizesPropertyId), ",", {}) };
    jassert (values.size () == 3);
    return { values [0].getDoubleValue (), values [1].getDoubleValue (), values [2].getDoubleValue () };
}

void GuiProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{

}
