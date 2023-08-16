#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ParameterDataProperties : public ValueTreeWrapper<ParameterDataProperties >
{
public:
    ParameterDataProperties () noexcept : ValueTreeWrapper<ParameterDataProperties> (ParameterTypeId) {}
    ParameterDataProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ParameterDataProperties> (ParameterTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    juce::String getName () { return data.getProperty ("name"); }
    juce::String getType () { return data.getProperty ("type"); }

    juce::String getDefaultString () { return data.getProperty ("default"); }
    int getDefaultInt () { return static_cast<int> (data.getProperty ("default")); }
    double getDefaultDouble () { return static_cast<double> (data.getProperty ("default")); }

    juce::StringPairArray getOptionalProperties ()
    {
        juce::StringPairArray optionalProperties;
        for (auto propertyIndex { 0 }; propertyIndex < data.getNumProperties (); ++propertyIndex)
        {
            const auto propertyName { data.getPropertyName (propertyIndex).toString () };
            if (propertyName != "name" && propertyName != "type" && propertyName != "default")
                optionalProperties.set (propertyName, data.getProperty (propertyName));
        }
        return optionalProperties;
    }

    static juce::String getDefaultString (juce::ValueTree vt) { return vt.getProperty ("default"); }
    static int getDefaultInt (juce::ValueTree vt) { return static_cast<int> (vt.getProperty ("default")); }
    static double getDefaultDouble (juce::ValueTree vt) { return static_cast<double> (vt.getProperty ("default")); }

    static inline const juce::Identifier ParameterTypeId { "Parameter" };

    void initValueTree () {}
    void processValueTree () {}

private:
};
