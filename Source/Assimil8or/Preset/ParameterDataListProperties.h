#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

#define LOG_VALIDATE_PARAMETERS 0
#if LOG_VALIDATE_PARAMETERS
#define LogValidateParameters(text) juce::Logger::outputDebugString (text);
#else
#define LogValidateParameters(text) ;
#endif

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

    void initValueTree ()
    {
        juce::XmlDocument xmlDoc { BinaryData::Assimil8orParameterData_xml };
        auto xmlElement { xmlDoc.getDocumentElement (false) };
        if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
            juce::Logger::outputDebugString ("XML Parsing Error: " + parseError);
        // NOTE: this is a hard failure, which indicates there is a problem in the file Assimil8orParameterData.xml
        jassert (xmlDoc.getLastParseError () == "");
        if (xmlElement != nullptr)
        {
            auto parameterData { juce::ValueTree::fromXml (*xmlElement) };
            wrap (parameterData, ParameterDataListProperties::WrapperType::owner, ParameterDataListProperties::EnableCallbacks::no);
        }
    }
    void processValueTree ()
    {
#if JUCE_DEBUG
        validateParameterData ();
#endif
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
            LogValidateParameters ("Section: " + vt.getType ().toString ());
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
                    LogValidateParameters (ouptutString + optionalProperties);
                    return true;
                });
            };
        validateParameters (presetSection);
        validateParameters (channelSection);
        validateParameters (zoneSection);
    }
};
