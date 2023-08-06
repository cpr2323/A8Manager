#include "AppProperties.h"

void AppProperties::initValueTree ()
{
    juce::ValueTree filesChildVT { juce::ValueTree (FileTypeId) };
    filesChildVT.setProperty (MostRecentFolderPropertyId, "", nullptr);
    juce::ValueTree mruListChildVT { juce::ValueTree (MRUListTypeId) };
    mruListChildVT.setProperty (MaxMRUEntriesPropertyId, 10, nullptr);
    filesChildVT.addChild (mruListChildVT, -1, nullptr);
    data.addChild (filesChildVT, -1, nullptr);
}

void AppProperties::processValueTree ()
{
    if (auto mruListChildVT { getMRUListChildVT () }; ! mruListChildVT.hasProperty (MaxMRUEntriesPropertyId))
        mruListChildVT.setProperty (MaxMRUEntriesPropertyId, 10, nullptr);
}

int AppProperties::getNumMRUEntries ()
{
    auto mruListVT { getMRUListChildVT () };
    auto count { 0 };
    ValueTreeHelpers::forEachChildOfType (mruListVT, MRUEntryTypeId , [this, &count] (juce::ValueTree child)
    {
        ++count;
        return true;
    });

    return count;
}

void AppProperties::setMostRecentFolder (juce::String folderName)
{
    auto filesChildVT { data.getChildWithName (FileTypeId) };
    filesChildVT.setProperty (MostRecentFolderPropertyId, folderName, nullptr);
}

juce::String AppProperties::getMostRecentFolder ()
{
    auto filesChildVT { data.getChildWithName (FileTypeId) };
    return filesChildVT.getProperty (MostRecentFolderPropertyId);
}

void AppProperties::addRecentlyUsedFile (juce::String fileName)
{
    auto mruListVT { getMRUListChildVT () };

    auto alreadyInList { mruListVT.getChildWithProperty (MRUEntryNamePropertyId, fileName) };
    if (alreadyInList.isValid ())
        mruListVT.removeChild (alreadyInList, nullptr);

    // create our new entry
    juce::ValueTree mruListEntry (MRUEntryTypeId);
    mruListEntry.setProperty (MRUEntryNamePropertyId, fileName, nullptr);

    // add new entry to list
    mruListVT.addChild (mruListEntry, 0, nullptr);

    // trim list if it has gotten too long
    if (auto numMruEntries { getNumMRUEntries () }; numMruEntries > getMaxMruEntries ())
        mruListVT.removeChild (numMruEntries - 1, nullptr);
}

juce::String AppProperties::getRecentlyUsedFile (int index)
{
    if (index > getMaxMruEntries ())
        return {};
    return getMRUListChildVT ().getChild (index).getProperty (MRUEntryNamePropertyId);
}

juce::StringArray AppProperties::getMRUList ()
{
    auto mruListVT { getMRUListChildVT () };
    juce::StringArray mruList;
    ValueTreeHelpers::forEachChildOfType (mruListVT, MRUEntryTypeId , [this, &mruList] (juce::ValueTree child)
    {
        mruList.add (child.getProperty (MRUEntryNamePropertyId));
        return true;
    });

    return mruList;
}

void AppProperties::setMaxMruEntries (int maxMruEntries)
{
    getMRUListChildVT ().setProperty (MaxMRUEntriesPropertyId, maxMruEntries, nullptr);
}

int AppProperties::getMaxMruEntries ()
{
    return getMRUListChildVT ().getProperty (MaxMRUEntriesPropertyId);
}

juce::ValueTree AppProperties::getMRUListChildVT ()
{
    return data.getChildWithName (FileTypeId).getChildWithName (MRUListTypeId);
}

void AppProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt.getParent () == data)
    {
        if (property == MostRecentFolderPropertyId)
        {
            if (onMostRecentFolderChange != nullptr)
                onMostRecentFolderChange (getMostRecentFolder ());
        }
    }
}
