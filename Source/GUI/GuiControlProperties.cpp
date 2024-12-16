#include "GuiControlProperties.h"

void GuiControlProperties::initValueTree ()
{
    showMidiConfigWindow (false);
}

void GuiControlProperties::showMidiConfigWindow (bool show)
{
    setValue (show, ShowMidiConfigWindowPropertyId, false);
}

bool GuiControlProperties::getShowMidiConfigWindow ()
{
    return getValue<bool> (ShowMidiConfigWindowPropertyId);
}

void GuiControlProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == ShowMidiConfigWindowPropertyId)
    {
        if (onShowMidiConfigWindowChange != nullptr)
            onShowMidiConfigWindowChange (getShowMidiConfigWindow ());
    }
}