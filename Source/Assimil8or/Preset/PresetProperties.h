#pragma once

#include <JuceHeader.h>
#include "BinaryData.h"
#include "ChannelProperties.h"
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
    int getDefaultInt () { return static_cast<int>(data.getProperty ("default")); }
    double getDefaultDouble () { return static_cast<double>(data.getProperty ("default")); }
    juce::StringPairArray getOptionalProperties ()
    {
        juce::StringPairArray optionalProperties;
        for (auto propertyIndex { 0 }; propertyIndex < data.getNumProperties (); ++propertyIndex)
        {
            const auto propertyName { data.getPropertyName (propertyIndex).toString () };
            if (propertyName != "name" && propertyName != "type" && propertyName != "default")
                optionalProperties.set (propertyName, data.getProperty (propertyName));
        }
    }
    static inline const juce::Identifier ParameterTypeId { "Parameter" };

    void initValueTree () {}
    void processValueTree () {}

private:
};

class ParameterDataListProperties : public ValueTreeWrapper<ParameterDataListProperties>
{
public:
    ParameterDataListProperties () noexcept : ValueTreeWrapper<ParameterDataListProperties> (Assimil8orParameterValuesTypeId) {}
    ParameterDataListProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ParameterDataListProperties> (Assimil8orParameterValuesTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void forEachParameter (juce::Identifier sectionTypeId, std::function<bool (juce::ValueTree parameterVT)> parameterVTCallback)
    {
        auto section { data.getChildWithName (sectionTypeId) };
        jassert (section.isValid ());
        ValueTreeHelpers::forEachChildOfType (section, "Parameter", [parameterVTCallback] (juce::ValueTree parameterVT)
        {
            parameterVTCallback (parameterVT);
            return true;
        });
    }

    juce::ValueTree getParameter (juce::Identifier sectionTypeId, juce::String parameterName)
    {
        auto section { data.getChildWithName (sectionTypeId) };
        jassert (section.isValid ());
        return section.getChildWithProperty ("name", parameterName);
    }

    static inline const juce::Identifier Assimil8orParameterValuesTypeId { "Assimil8orParameterValues" };
    static inline const juce::Identifier PresetSectionTypeId { "Preset" };
    static inline const juce::Identifier ChannelSectionTypeId { "Channel" };
    static inline const juce::Identifier ZoneSectionTypeId { "Zone" };
    static inline const juce::Identifier ParameterTypeId { "Parameter" };

    void initValueTree () {}
    void processValueTree ()
    {
        validateParameterData ();
    }

private:
    void validateParameterData ()
    {
        jassert (data.getType ().toString () == "Assimil8orParameterValues");
        auto presetSection { data.getChildWithName ("Preset") };
        jassert (presetSection.isValid ());
        auto channelSection { data.getChildWithName ("Channel") };
        jassert (channelSection.isValid ());
        auto zoneSection { data.getChildWithName ("Zone") };
        jassert (zoneSection.isValid ());

        auto validateParameters = [] (juce::ValueTree vt)
        {
            juce::Logger::outputDebugString ("Section: " + vt.getType ().toString ());
            ValueTreeHelpers::forEachChild (vt, [] (juce::ValueTree child)
            {
                // make sure there are no unexpected children
                jassert (child.getType ().toString () == "Parameter");
                // make sure the require properties are there
                jassert (child.hasProperty ("name"));
                jassert (child.hasProperty ("type"));
                jassert (child.hasProperty ("default"));
                const auto name { child.getProperty ("name").toString () };
                const auto parameterType { child.getProperty ("type").toString () };
                const auto parameterDefault { child.getProperty ("default").toString () };
                auto optionalProperties { juce::String {} };
                auto addOptional = [&optionalProperties, child] (juce::String property)
                {
                    jassert (child.hasProperty (property));
                    if (optionalProperties.isEmpty ())
                        optionalProperties = ", [";
                    else
                        optionalProperties = optionalProperties.trimCharactersAtEnd ("]") + ", ";
                    const auto value { child.getProperty (property).toString () };
                    optionalProperties += property + ": '" + value + "']";
                };
                auto ouptutString { juce::String ("  name: '" + name + "', type: '" + parameterType + "', default: '" + parameterDefault + "'") };
                if (parameterType == "bool")
                {
                    jassert (parameterDefault == "false" || parameterDefault == "true");
                }
                else if (parameterType == "cvInputChannel")
                {

                }
                else if (parameterType == "cvInputGlobal")
                {

                }
                else if (parameterType == "cvInputWithDouble")
                {

                }
                else if (parameterType == "double")
                {

                }
                else if (parameterType == "integer")
                {

                }
                else if (parameterType == "string")
                {

                }
                else if (parameterType == "stringList")
                {
                    jassert (child.hasProperty ("list"));
                    addOptional ("list");
                }
                else
                {
                    // unknown type
                    jassertfalse;
                }
                if (child.hasProperty ("min") || child.hasProperty ("max"))
                {
                    jassert (child.hasProperty ("min") && child.hasProperty ("max"));
                    addOptional ("min");
                    addOptional ("max");
                }
                juce::Logger::outputDebugString (ouptutString + optionalProperties);
                return true;
            });
        };
        validateParameters (presetSection);
        validateParameters (channelSection);
        validateParameters (zoneSection);
    }
};

class PresetProperties : public ValueTreeWrapper<PresetProperties>
{
public:
    PresetProperties () noexcept : ValueTreeWrapper<PresetProperties> (PresetTypeId)
    {
        initDefaults ();
    }
    PresetProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<PresetProperties> (PresetTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
        initDefaults ();
    }

    void setIndex (int index, bool includeSelfCallback);
    void setData2AsCV (juce::String data2AsCv, bool includeSelfCallback);
    void setName (juce::String name, bool includeSelfCallback);
    void setXfadeACV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeAWidth (double width, bool includeSelfCallback);
    void setXfadeBCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeBWidth (double width, bool includeSelfCallback);
    void setXfadeCCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeCWidth (double width, bool includeSelfCallback);
    void setXfadeDCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeDWidth (double width, bool includeSelfCallback);

    int getIndex ();
    juce::String getData2AsCV ();
    juce::String getName ();
    juce::String getXfadeACV ();
    double getXfadeAWidth ();
    juce::String getXfadeBCV ();
    double getXfadeBWidth ();
    juce::String getXfadeCCV ();
    double getXfadeCWidth ();
    juce::String getXfadeDCV ();
    double getXfadeDWidth ();

    juce::String getData2AsCVDefault ();
    juce::String getNameDefault ();
    juce::String getXfadeACVDefault ();
    double getXfadeAWidthDefault ();
    juce::String getXfadeBCVDefault ();
    double getXfadeBWidthDefault ();
    juce::String getXfadeCCVDefault ();
    double getXfadeCWidthDefault ();
    juce::String getXfadeDCVDefault ();
    double getXfadeDWidthDefault ();

    std::function<void (int index)> onIndexChange;
    std::function<void (juce::String data2AsCv)> onData2AsCVChange;
    std::function<void (juce::String name)> onNameChange;
    std::function<void (juce::String cvInput)> onXfadeACVChange;
    std::function<void (double width)> onXfadeAWidthChange;
    std::function<void (juce::String cvInput)> onXfadeBCVChange;
    std::function<void (double width)> onXfadeBWidthChange;
    std::function<void (juce::String cvInput)> onXfadeCCVChange;
    std::function<void (double width)> onXfadeCWidthChange;
    std::function<void (juce::String cvInput)> onXfadeDCVChange;
    std::function<void (double width)> onXfadeDWidthChange;

    void initDefaults ();
    void clear ();
    juce::ValueTree addChannel (int index);
    void forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback);
    int getNumChannels ();

    static inline const juce::Identifier PresetTypeId { "Preset" };
    static inline const juce::Identifier IndexPropertyId       { "_index" };
    static inline const juce::Identifier Data2asCVPropertyId   { "data2asCV" };
    static inline const juce::Identifier NamePropertyId        { "name" };
    static inline const juce::Identifier XfadeACVPropertyId    { "xfadeACV" };
    static inline const juce::Identifier XfadeAWidthPropertyId { "xfadeAWidth" };
    static inline const juce::Identifier XfadeBCVPropertyId    { "xfadeBCV" };
    static inline const juce::Identifier XfadeBWidthPropertyId { "xfadeBWidth" };
    static inline const juce::Identifier XfadeCCVPropertyId    { "xfadeCCV" };
    static inline const juce::Identifier XfadeCWidthPropertyId { "xfadeCWidth" };
    static inline const juce::Identifier XfadeDCVPropertyId    { "xfadeDCV" };
    static inline const juce::Identifier XfadeDWidthPropertyId { "xfadeDWidth" };

    void initValueTree ();
    void processValueTree () {}

private:
    ParameterDataListProperties parameterDataListProperties;
    std::map<juce::String, juce::var> defaults;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
