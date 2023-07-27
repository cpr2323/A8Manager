#ifndef _VALUETREE_WRAPPER_H_
#define _VALUETREE_WRAPPER_H_

#include <JuceHeader.h>
#include "ValueTreeHelpers.h"
#include <type_traits>

/*
    ValueTreeWrapper allows you to create specialized ValueTree handlers, takes care of some heavy lifting of ValueTrees,
    and allows clients to use them without doing ValueTree things.
*/

class ValueTreeWrapper : private juce::ValueTree::Listener
{
public:
    ValueTreeWrapper (juce::Identifier type) noexcept;
    ValueTreeWrapper (ValueTreeWrapper&&) = default;
    ValueTreeWrapper& operator = (ValueTreeWrapper&&) = default;

    /*
        wrap will do one of four things:
            1. Given an empty/invalid ValueTree, it will create a valid one to wrap
            2. Given a valid ValueTree of the initialized 'type', it will wrap that one
            3. Given a valid ValueTree no of initialized 'type'
                a. If there is a child of the initialized 'type' it will wrap that one
                b. If there is not a child of the initialized 'type', is will create one, wrap that one, and add it to the passing in ValueTree
    */
    enum class WrapperType { owner, client };
    enum class EnableCallbacks { no, yes };
    void wrap (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks);

    bool isValid () const noexcept { return data.isValid (); }
    void release () { data = juce::ValueTree (); }
    juce::ValueTree getValueTree () noexcept;
    juce::ValueTree& getValueTreeRef () noexcept;
    bool wasDataRestored () { return dataWasRestored; }
    void enableCallbacks (bool enableCallbacks);

    /*
        Pattern for creating specialized wrapper

            Identifier PropertyAIdentifier {"propertyA"};

            std::function <void (int propertyValue)> onPropertyAChanged;

            void setProperyA (int propertyValue, bool includeSelfCallback = false)
            {
                setValue (propertyValue, PropertyAIdentifier, includeSelfCallback);
            }

            int getPropertyA ()
            {
                return getValue<int> (PropertyAIdentifier);
            }

            void valueTreePropertyChanged (ValueTree&, const Identifier& property)
            {
                if (property == PropertyAIdentifier)
                {
                    if (onPropertyAChanged != nullptr)
                        onPropertyAChanged (getProperty ());
                }
            }
    */

    bool setFilterNonChanges (bool doFilterNonChange) noexcept { const auto prevFilterNonChangeSetting { filterNonChange }; filterNonChange = doFilterNonChange; return prevFilterNonChangeSetting; }

    class ScopedFilterNonChanges
    {
    public:
        ScopedFilterNonChanges (ValueTreeWrapper& theVtWrapper, bool doFilterNonChange)
            : vtWrapper {theVtWrapper}
        {
            previousFilterNonChange = vtWrapper.setFilterNonChanges (doFilterNonChange);
        }

        ~ScopedFilterNonChanges ()
        {
            vtWrapper.setFilterNonChanges (previousFilterNonChange);
        }
    private:
        ValueTreeWrapper& vtWrapper;
        bool previousFilterNonChange { false };
    };

protected:
    juce::ValueTree data;

    // non-pointer version
    template<class T, std::enable_if_t<! std::is_pointer_v<T>, void*> = nullptr>
    void setValue (T value, const juce::Identifier property, bool includeSelfCallback)
    {
        T previousValue {};
        if (! filterNonChange)
            previousValue = getValue<T> (property);

        if (includeSelfCallback)
            data.setProperty (property, value, nullptr);
        else
            data.setPropertyExcludingListener (this, property, value, nullptr);

        if (! filterNonChange && previousValue == value)
            data.sendPropertyChangeMessage (property);
    }

    // pointer specialization version
    template<class T, std::enable_if_t<std::is_pointer_v<T>, void*> = nullptr>
    void setValue (T value, const juce::Identifier property, bool includeSelfCallback)
    {
        const auto int64Value { reinterpret_cast<int64_t> (value)};
        int64_t previousValue {};
        if (! filterNonChange)
            previousValue = getValue<int64_t> (property);

        if (includeSelfCallback)
            data.setProperty (property, int64Value, nullptr);
        else
            data.setPropertyExcludingListener (this, property, int64Value, nullptr);

        if (! filterNonChange && previousValue == int64Value)
            data.sendPropertyChangeMessage (property);
    }

    // non-pointer version
    template<class T, std::enable_if_t<! std::is_pointer_v<T>, void*> = nullptr>
    T getValue (const juce::Identifier property) const noexcept
    {
        //TODO: this is probably something we want, but also this means we can't use any "trigger"-like functions in initValueTree (), e.g, RuntimeRootProperties::triggerAppResumed ()
        //jassert (data.hasProperty (property));
        return data.getProperty (property);
    }

    // pointer specialization version
    template<class T, std::enable_if_t<std::is_pointer_v<T>, void*> = nullptr>
    T getValue (const juce::Identifier property) const noexcept
    {
        //TODO: this is probably something we want, but also this means we can't use any "trigger"-like functions in initValueTree (), e.g, RuntimeRootProperties::triggerAppResumed ()
        //jassert (data.hasProperty (property));
        return (int64_t) data.getProperty (property);
    }

    // non-pointer version
    template<class T, std::enable_if_t<! std::is_pointer_v<T>, void*> = nullptr>
    static T getValue (const juce::Identifier property, const juce::ValueTree vt) noexcept
    {
        jassert (vt.hasProperty (property));
        return vt.getProperty (property);
    }

    // pointer specialization version
    template<class T, std::enable_if_t<std::is_pointer_v<T>, void*> = nullptr>
    static T getValue (const juce::Identifier property, const juce::ValueTree vt) noexcept
    {
        jassert (vt.hasProperty (property));
        return (T) ((int64_t) vt.getProperty (property));
    }

    template<class T>
    void setValueInOtherVt (T value, juce::ValueTree vt, const juce::Identifier property, bool includeSelfCallback)
    {
        T previousValue {};
        if (! filterNonChange)
            previousValue = vt.getProperty (property);

        if (includeSelfCallback)
            vt.setProperty (property, value, nullptr);
        else
            vt.setPropertyExcludingListener (this, property, value, nullptr);

        if (! filterNonChange && previousValue == value)
            vt.sendPropertyChangeMessage (property);
    }

    void toggleValue (const juce::Identifier property, bool includeSelfCallback)
    {
        setValue (! getValue<bool> (property), property, includeSelfCallback);
    }

    void removePropertyIfExists (juce::Identifier property)
    {
        ValueTreeHelpers::removePropertyIfExists (data, property);
    };

    void addPropertyIfMissing (juce::Identifier property, std::function<void ()> addPropertyCallback)
    {
        jassert (addPropertyCallback != nullptr);
        if (! data.hasProperty (property))
            addPropertyCallback ();
    };

private:
    juce::Identifier type;
    bool filterNonChange { true };
    bool dataWasRestored { false };

    void init (juce::ValueTree vt, bool createIfNotFound);

    void createValueTree ();
    virtual void initValueTree () = 0; // called when wrapping an empty ValueTree
    virtual void processValueTree () {} // called after wrapping an initialized ValueTree

    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override {};
    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override {}
    void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) override {}
    void valueTreeChildOrderChanged (juce::ValueTree&, int, int) override {}
    void valueTreeParentChanged (juce::ValueTree&) override {}

    JUCE_DECLARE_NON_COPYABLE (ValueTreeWrapper)
};

#endif // _VALUETREE_WRAPPER_H_
