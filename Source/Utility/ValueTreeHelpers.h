// Copyright (c) 2019 Artiphon, Inc. All Rights Reserved.

#pragma once

#include <JuceHeader.h>

namespace ValueTreeHelpers
{
    enum class StopAtFirstFailure
    {
        no,
        yes
    };
    enum class LogCompareFailures
    {
        no,
        yes
    };
    bool compareChidrenAndThierPropertiesUnordered (juce::ValueTree firstVT, juce::ValueTree secondVT, LogCompareFailures logCompareFailures, StopAtFirstFailure stopAtFirstFailure);
    bool comparePropertiesUnOrdered (juce::ValueTree firstVT, juce::ValueTree secondVT, LogCompareFailures logCompareFailures, StopAtFirstFailure stopAtFirstFailure);
    void dumpValueTreeContent (juce::ValueTree vt, bool displayProperties, std::function<void (juce::String)> displayFunction);
    juce::ValueTree findChild (juce::ValueTree parent, std::function<bool (juce::ValueTree child)> findChildCallback);
    void forEachChild (juce::ValueTree parent, std::function<bool (juce::ValueTree child)> childCallback);
    void forEachChildOfType (juce::ValueTree parent, juce::Identifier childType, std::function<bool (juce::ValueTree child)> childCallback);
    juce::ValueTree fromXmlData (const void* data, size_t size);
    juce::ValueTree fromXmlString (juce::StringRef xmlString);
    uint32_t getCrc (juce::ValueTree tree);
    juce::ValueTree getParentOfType (const juce::ValueTree child, const juce::Identifier parentId);
    juce::ValueTree getTypeFromRoot (const juce::ValueTree child, const juce::Identifier type);
    void overwriteExistingChildren (juce::ValueTree source, juce::ValueTree dest);
    void overwriteExistingChildrenAndProperties (juce::ValueTree source, juce::ValueTree dest);
    void overwriteExistingProperties (juce::ValueTree source, juce::ValueTree dest);
    void removePropertyIfExists (juce::ValueTree vt, juce::Identifier property);
};
