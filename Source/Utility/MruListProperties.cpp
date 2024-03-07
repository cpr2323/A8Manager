// WIP
#if 0
#include "MruListProperties.h"

void MruListProperties::initValueTree ()
{
    setListName ("");
    setMaxMruEntries (1);
}

void MruListProperties::setListName (juce::String listName)
{
    setValue (listName, ListNamePropertyId, false);
}

void MruListProperties::addMruEntry (juce::String entryName)
{
    auto alreadyInList { data.getChildWithProperty (MRUEntryNamePropertyId, entryName) };
    if (alreadyInList.isValid ())
        data.removeChild (alreadyInList, nullptr);

    // create our new entry
    juce::ValueTree mruListEntry (MRUEntryTypeId);
    mruListEntry.setProperty (MRUEntryNamePropertyId, entryName, nullptr);

    // add new entry to list
    data.addChild (mruListEntry, 0, nullptr);

    // trim list if it has gotten too long
    if (auto numMruEntries { getNumMruEntries () }; numMruEntries > getMaxMruEntries ())
        data.removeChild (numMruEntries - 1, nullptr);
}

void MruListProperties::setMaxMruEntries (int maxMruEntries)
{
    setValue (maxMruEntries, MaxMRUEntriesPropertyId, false);
    auto numMruEntries { getNumMruEntries () };
    while (numMruEntries-- > maxMruEntries)
    {
        // we use the decremented value from the while, since the index is 1 less than the count
        data.removeChild (numMruEntries, nullptr);
    }
}

juce::String MruListProperties::getListName ()
{
    return getValue<juce::String> (ListNamePropertyId);
}

juce::String MruListProperties::getMruEntry (int index)
{
    if (index > getMaxMruEntries ())
        return {};
    return data.getChild (index).getProperty (MRUEntryNamePropertyId);
}

int MruListProperties::getMaxMruEntries ()
{
    return getValue<int> (MaxMRUEntriesPropertyId);
}

int MruListProperties::getNumMruEntries ()
{
    auto count { 0 };
    ValueTreeHelpers::forEachChildOfType (data, MRUEntryTypeId, [this, &count] (juce::ValueTree child)
    {
        ++count;
        return true;
    });

    return count;
}

void MruListProperties::valueTreeChildAdded (juce::ValueTree& parentVT, juce::ValueTree& childVT)
{
    if (parentVT == data)
    {
        if (onMostRecentEntryChange != nullptr)
            onMostRecentEntryChange (childVT.getProperty (MRUEntryNamePropertyId).toString ());
    }
}
#endif