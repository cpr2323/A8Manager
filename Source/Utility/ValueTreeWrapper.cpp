#include "ValueTreeWrapper.h"

ValueTreeWrapper::ValueTreeWrapper (juce::Identifier newType) noexcept
{
    type = newType;
}

void ValueTreeWrapper::wrap (juce::ValueTree vt, ValueTreeWrapper::WrapperType wrapperType, ValueTreeWrapper::EnableCallbacks shouldEnableCallbacks)
{
    init (vt, wrapperType == ValueTreeWrapper::WrapperType::owner);
    enableCallbacks (ValueTreeWrapper::EnableCallbacks::yes == shouldEnableCallbacks);
}

void ValueTreeWrapper::init (juce::ValueTree vt, bool createIfNotFound)
{
    dataWasRestored = false;

    if (vt.isValid ())
    {
        if (vt.getType () == type)
        {
            data = vt;
            dataWasRestored = true;
            processValueTree ();
        }
        else
        {
            juce::ValueTree child { vt.getChildWithName (type) };
            if (child.isValid ())
            {
                data = child;
                dataWasRestored = true;
                processValueTree ();
            }
            else if (createIfNotFound)
            {
                createValueTree ();
                vt.appendChild (data, nullptr);
            }
            else
            {
                jassertfalse;
            }
        }
    }
    else if (createIfNotFound)
    {
        createValueTree ();
    }
    else
    {
        jassertfalse;
    }
}

void ValueTreeWrapper::createValueTree ()
{
    data = juce::ValueTree (type);
    initValueTree ();
}

juce::ValueTree ValueTreeWrapper::getValueTree () noexcept
{
    return data;
}

juce::ValueTree& ValueTreeWrapper::getValueTreeRef () noexcept
{
    return data;
}

void ValueTreeWrapper::enableCallbacks (bool enableCallbacks)
{
    if (enableCallbacks)
        data.addListener (this);
    else
        data.removeListener (this);
}
