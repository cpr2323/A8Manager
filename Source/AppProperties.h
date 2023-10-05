#pragma once

#include <JuceHeader.h>
#include "Utility/ValueTreeWrapper.h"

class AppProperties : public ValueTreeWrapper<AppProperties>
{
public:
    AppProperties () noexcept : ValueTreeWrapper<AppProperties> (AppPropertiesId) {}

    void setMostRecentFolder (juce::String folderName);
    juce::String getMostRecentFolder ();
    void addRecentlyUsedFile (juce::String fileName);
    juce::String getRecentlyUsedFile (int index);
    juce::StringArray getMRUList ();
    void setMaxMruEntries (int maxMruEntries);
    int getMaxMruEntries ();

    std::function<void (juce::String folderName)> onMostRecentFolderChange;
    std::function<void (juce::String fileName)> onMostRecentFileChange;

    static inline const juce::Identifier AppPropertiesId { "App" };

    static inline const juce::Identifier FileTypeId { "Files" };
    static inline const juce::Identifier MostRecentFolderPropertyId { "mostRecentFolder" };

    static inline const juce::Identifier MRUListTypeId { "MRUList" };
    static inline const juce::Identifier MaxMRUEntriesPropertyId { "maxMruEntries" };

    static inline const juce::Identifier MRUEntryTypeId { "MRUEntry" };
    static inline const juce::Identifier MRUEntryNamePropertyId { "name" };

    void initValueTree ();
    void processValueTree ();

private:
    juce::ValueTree getMRUListChildVT ();
    int getNumMRUEntries ();

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override;
};
