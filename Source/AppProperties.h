#pragma once

#include <JuceHeader.h>
#include "Utility/ValueTreeWrapper.h"

class AppProperties : public ValueTreeWrapper
{
public:
    AppProperties () noexcept : ValueTreeWrapper (AppPropertiesId) {}

    void setMostRecentFolder (juce::String folderName);
    juce::String getMostRecentFolder ();
    void addRecentlyUsedFile (juce::String fileName);
    juce::String getRecentlyUsedFile (int index);
    juce::StringArray getMRUList ();
    void setMaxMruEntries (int maxMruEntries);
    int getMaxMruEntries ();

    static inline const juce::Identifier AppPropertiesId { "App" };

    static inline const juce::Identifier FileChildPropertiesId { "Files" };
    static inline const juce::Identifier MostRecentFolderPropertyId { "mostRecentFolder" };

    static inline const juce::Identifier MRUListChildPropertiesId { "MRUList" };
    static inline const juce::Identifier MaxMRUEntriesPropertyId { "maxMruEntries" };

    static inline const juce::Identifier MRUEntryChildPropertiesId { "MRUEntry" };
    static inline const juce::Identifier MRUEntryNamePropertyId { "name" };

private:
    juce::ValueTree getMRUListChildVT ();
    int getNumMRUEntries ();

    void initValueTree () override;
    void processValueTree () override;
};