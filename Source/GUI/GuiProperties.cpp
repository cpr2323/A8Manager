#include "GuiProperties.h"

const auto defaultXPos { -1 };
const auto defaultYPos { -1 };
const auto defaultWidth { 1117 };
const auto defaultHeight { 609 };
const auto defaultSplitter1Offset { 140 };
const auto defaultSplitter2Offset { 170 };
const auto defaultSplitter3Offset { 480 };
const auto defaultBackgroundLevel { 0.0f };
// empty means 'no preference yet', so the window opens on its first tab
const auto defaultSettingsTabName { juce::String () };

void GuiProperties::initValueTree ()
{
    setPosition (defaultXPos, defaultYPos, false);
    setSize (defaultWidth, defaultHeight, false);
    setPaneSizes (defaultSplitter1Offset, defaultSplitter2Offset, defaultSplitter3Offset, false);
    setBackgroundLevel (defaultBackgroundLevel, false);
    setValue (false, ShowSettingsDialogPropertyId, false);
    setSettingsTabName (defaultSettingsTabName, false);
}

void GuiProperties::processValueTree ()
{
    if (! data.hasProperty (PositionPropertyId))
        setPosition (defaultXPos, defaultYPos, false);
    if (! data.hasProperty (SizePropertyId))
        setSize (defaultWidth, defaultHeight, false);
    if (! data.hasProperty (PaneSizesPropertyId))
        setPaneSizes (defaultSplitter1Offset, defaultSplitter2Offset, defaultSplitter3Offset, false);
    if (! data.hasProperty (BackgroundLevelPropertyId))
        setBackgroundLevel (defaultBackgroundLevel, false);
    if (! data.hasProperty (ShowSettingsDialogPropertyId))
        setValue (false, ShowSettingsDialogPropertyId, false);
    if (! data.hasProperty (SettingsTabNamePropertyId))
        setSettingsTabName (defaultSettingsTabName, false);
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

void GuiProperties::setBackgroundLevel (float backgroundLevel, bool includeSelfCallback)
{
    setValue (std::clamp (backgroundLevel, 0.0f, 1.0f), BackgroundLevelPropertyId, includeSelfCallback);
}

void GuiProperties::showSettingsDialog (bool includeSelfCallback)
{
    toggleValue (ShowSettingsDialogPropertyId, includeSelfCallback);
}

void GuiProperties::setSettingsTabName (juce::String tabName, bool includeSelfCallback)
{
    setValue (tabName, SettingsTabNamePropertyId, includeSelfCallback);
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

float GuiProperties::getBackgroundLevel ()
{
    return getValue<float> (BackgroundLevelPropertyId);
}

juce::String GuiProperties::getSettingsTabName ()
{
    return getValue<juce::String> (SettingsTabNamePropertyId);
}

void GuiProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == BackgroundLevelPropertyId)
        {
            if (onBackgroundLevelChange != nullptr)
                onBackgroundLevelChange (getBackgroundLevel ());
        }
        else if (property == ShowSettingsDialogPropertyId)
        {
            if (onShowSettingsDialog != nullptr)
                onShowSettingsDialog ();
        }
    }
}
