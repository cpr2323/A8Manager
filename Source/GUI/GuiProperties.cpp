#include "GuiProperties.h"

const auto defaultXPos { -1 };
const auto defaultYPos { -1 };
const auto defaultWidth { 1117 };
const auto defaultHeight { 585 };
const auto defaultSplitter1Offset { 140 };
const auto defaultSplitter2Offset { 170 };
const auto defaultSplitter3Offset { 480 };

void GuiProperties::initValueTree ()
{
    setPosition (defaultXPos, defaultYPos, false);
    setSize (defaultWidth, defaultHeight, false);
    setPaneSizes (defaultSplitter1Offset, defaultSplitter2Offset, defaultSplitter3Offset, false);
}

void GuiProperties::processValueTree ()
{
    if (! data.hasProperty (PositionPropertyId))
        setPosition (defaultXPos, defaultYPos, false);
    if (! data.hasProperty (SizePropertyId))
        setSize (defaultWidth, defaultHeight, false);
    if (! data.hasProperty (PaneSizesPropertyId))
        setPaneSizes (defaultSplitter1Offset, defaultSplitter2Offset, defaultSplitter3Offset, false);
}

void GuiProperties::setPosition (int x, int y, bool includeSelfCallback)
{
    setValue (juce::String (x) + "," + juce::String (y), PositionPropertyId, includeSelfCallback);
}

void GuiProperties::setSize (int width, int height, bool includeSelfCallback)
{
    setValue (juce::String (width) + "," + juce::String (height), SizePropertyId, includeSelfCallback);

}

void GuiProperties::setPaneSizes (int pane1Size, int pane2Size, int pane3Size, bool includeSelfCallback)
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

std::tuple<int, int, int> GuiProperties::getPaneSizes ()
{
    const auto values { juce::StringArray::fromTokens (getValue<juce::String> (PaneSizesPropertyId), ",", {}) };
    jassert (values.size () == 3);
    return { values [0].getIntValue (), values [1].getIntValue (), values [2].getIntValue () };
}

void GuiProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{

}
