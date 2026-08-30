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

    /*
        Thread safety helpers.

        juce::ValueTree is not thread safe. A ValueTree that has listeners attached (a 'live' tree) may only be
        read or modified on the message thread. Background threads should work with detached trees which they
        exclusively own. These helpers support that pattern:
            - callOnMessageThread: run any code on the message thread (immediately if already on it)
            - getMessageThreadSnapshot: from a background thread, get a detached deep copy of a live tree,
              made on the message thread. the copy can then be freely read on the background thread
            - replaceChildrenOnMessageThread: from a background thread, publish a detached tree that was built
              on the background thread, by having the message thread move its children into the live tree.
              after the call, the background thread must no longer touch the source tree

        See also ValueTreeWrapper::setForwardOffMessageThreadWrites, which makes wrapper setters safe to call
        from any thread.
    */
    // runs the function on the message thread: immediately if called on it, otherwise async
    void callOnMessageThread (std::function<void ()> function);
    // blocks until the message thread makes a deep copy of the given live tree, and returns the detached copy
    // returns an invalid tree if the calling thread is asked to exit, or if the message thread does not service the
    // request within timeoutMs. the timeout is generous on purpose: during startup the message thread can be busy
    // building the UI for several seconds, and a snapshot that gives up early leaves the caller with no data at all
    // NOTE: do not call while holding a lock that the message thread might need, as that can deadlock
    juce::ValueTree getMessageThreadSnapshot (juce::ValueTree liveVT, int timeoutMs = 30000);
    // on the message thread, replaces all children of liveDestinationVT with the children of detachedSourceVT,
    // then calls the optional onCompletion (also on the message thread)
    void replaceChildrenOnMessageThread (juce::ValueTree liveDestinationVT, juce::ValueTree detachedSourceVT, std::function<void ()> onCompletion = nullptr);
    void dumpValueTreeContent (juce::ValueTree vt, bool displayProperties, std::function<void (juce::String)> displayFunction);
    juce::ValueTree findChild (juce::ValueTree parent, std::function<bool (juce::ValueTree child)> findChildCallback);
    void forEachChild (juce::ValueTree parent, std::function<bool (juce::ValueTree child)> childCallback);
    void forEachChildOfType (juce::ValueTree parent, juce::Identifier childType, std::function<bool (juce::ValueTree child)> childCallback);
    void forEachProperty (juce::ValueTree vt, std::function<bool (juce::Identifier property)> propertyCallback);
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
