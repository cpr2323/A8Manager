#include "PresetManagerProperties.h"

void PresetManagerProperties::addPreset (juce::String presetName, juce::ValueTree presetPropertiesVT)
{
    // is name already used
    jassert (! data.getChildWithName (presetName).isValid ());
    if (data.getChildWithName (presetName).isValid ())
        return;
    juce::ValueTree newPresetHolder { PresetHolderPropertiesTypeId };
    newPresetHolder.setProperty (PresetHolderNamePropertyId, presetName, nullptr);
    newPresetHolder.addChild (presetPropertiesVT, -1, nullptr);
    data.addChild (newPresetHolder, -1, nullptr);
}

juce::ValueTree PresetManagerProperties::getPreset (juce::String presetName)
{
    auto presetHolder { data.getChildWithProperty (PresetHolderNamePropertyId, presetName) };
    jassert (presetHolder.isValid ());
    if (! presetHolder.isValid ())
        return {};
    return presetHolder.getChild (0);
}
