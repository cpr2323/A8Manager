#pragma once

#include <JuceHeader.h>

namespace DefaultHelpers
{
    template <typename T>
    void setAndHandleDefault (juce::ValueTree vt, juce::Identifier id, T value, bool includeSelfCallback, juce::ValueTree::Listener* vtListener, std::function<T ()> getDefaultValue)
    {
        jassert (vtListener != nullptr);
        jassert (getDefaultValue != nullptr);
        if (includeSelfCallback)
            vt.setProperty (id, value, nullptr);
        else
            vt.setPropertyExcludingListener (vtListener, id, value, nullptr);
        if (value == getDefaultValue ())
            vt.removeProperty (id, nullptr);
    }

    template <typename T>
    T getAndHandleDefault (const juce::ValueTree vt, const juce::Identifier id, std::function<T ()> getDefaultValue)
    {
        jassert (getDefaultValue != nullptr);
        if (vt.hasProperty (id))
            return vt.getProperty (id);
        else
            return getDefaultValue ();
    }
};
