#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class FixerEntryProperties : public ValueTreeWrapper<FixerEntryProperties>
{
public:
    FixerEntryProperties () noexcept : ValueTreeWrapper<FixerEntryProperties> (FixerEntryTypeId) {}
    FixerEntryProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
        : ValueTreeWrapper<FixerEntryProperties> (FixerEntryTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void setType (juce::String fixerType, bool includeSelfCallback);
    void setFileName (juce::String filename, bool includeSelfCallback);

    juce::String getType ();
    juce::String getFileName ();

    std::function<void (juce::String fixerType)> onTypeChange;
    std::function<void (juce::String fileName)> onFileNameChange;

    static inline const juce::String FixerTypeNone { "" };
    static inline const juce::String FixerTypeRename { "rename" };
    static inline const juce::String FixerTypeConvert { "convert" };
    static inline const juce::String FixerTypeNotFound { "notFound" };

    static inline const juce::Identifier FixerEntryTypeId { "FixerEntry" };
    static inline const juce::Identifier FixTypePropertyId     { "type" };
    static inline const juce::Identifier FixFilenamePropertyId { "filename" };

    void initValueTree () {}
    void processValueTree () {}

private:
};
