#include "AppActionProperties.h"

void AppActionProperties::initValueTree ()
{
    setPresetLoadState (false);
}

void AppActionProperties::setPresetLoadState (bool loading)
{
    setValue (loading, PresetLoadStatePropertyId, false);
}

bool AppActionProperties::getPresetLoadState ()
{
    return getValue<bool> (PresetLoadStatePropertyId);
}

void AppActionProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == PresetLoadStatePropertyId)
    {
        if (onPresetLoadStateChange)
            onPresetLoadStateChange (getPresetLoadState ());
    }
}
