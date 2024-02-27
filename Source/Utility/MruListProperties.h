#pragma once

// WIP
#if 0
#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

class MruListProperties : public ValueTreeWrapper<MruListProperties>
{
public:
    MruListProperties () noexcept : ValueTreeWrapper<MruListProperties> (MRUListTypeId) {}

    void setListName (juce::String listName);
    void addMruEntry (juce::String entryName);
    void setMaxMruEntries (int maxMruEntries);

    juce::String getListName ();
    juce::String getMruEntry (int index);
    int getMaxMruEntries ();
    int getNumMruEntries ();

    std::function<void (juce::String entryName)> onMostRecentEntryChange;

    static inline const juce::Identifier MRUListTypeId { "MRUList" };
    static inline const juce::Identifier ListNamePropertyId { "name" };
    static inline const juce::Identifier MaxMRUEntriesPropertyId { "maxMruEntries" };

    static inline const juce::Identifier MRUEntryTypeId { "MRUEntry" };
    static inline const juce::Identifier MRUEntryNamePropertyId { "name" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override;
};
#endif