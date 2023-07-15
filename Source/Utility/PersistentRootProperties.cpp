#include "PersistentRootProperties.h"

void PersistentRootProperties::initValueTree ()
{
}

juce::ValueTree PersistentRootProperties::addSection (juce::Identifier sectionType)
{
    if (auto theSection { getSection (sectionType) }; theSection.isValid ())
        return theSection;

    auto newSection { juce::ValueTree (sectionType) };
    data.addChild (newSection, -1, nullptr);
    return newSection;
}

bool PersistentRootProperties::removeSection (juce::Identifier sectionType)
{
    if (auto section { getSection (sectionType) }; section.isValid ())
    {
        data.removeChild (section, nullptr);
        return true;
    }
    return false; // child does not exist
}

juce::ValueTree PersistentRootProperties::getSection (juce::Identifier sectionType)
{
    return data.getChildWithName (sectionType);
}
