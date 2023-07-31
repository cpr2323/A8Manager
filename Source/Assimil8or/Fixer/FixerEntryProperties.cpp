#include "FixerEntryProperties.h"

void FixerEntryProperties::setType (juce::String fixerType, bool includeSelfCallback)
{
    setValue (fixerType, FixTypePropertyId, includeSelfCallback);
}

void FixerEntryProperties::setFileName (juce::String fileName, bool includeSelfCallback)
{
    setValue (fileName, FixFilenamePropertyId, includeSelfCallback);
}

juce::String FixerEntryProperties::getType ()
{
    return getValue<juce::String> (FixTypePropertyId);
}

juce::String FixerEntryProperties::getFileName ()
{
    return getValue<juce::String> (FixFilenamePropertyId);
}
