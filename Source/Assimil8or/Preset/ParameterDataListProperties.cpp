#include "ParameterDataListProperties.h"

void ParameterDataListProperties::forEachParameter (juce::Identifier sectionTypeId, std::function<bool (juce::ValueTree parameterVT)> parameterVTCallback)
{
    auto section { data.getChildWithName (sectionTypeId) };
    jassert (section.isValid ());
    ValueTreeHelpers::forEachChildOfType (section, "Parameter", [parameterVTCallback] (juce::ValueTree parameterVT)
    {
        parameterVTCallback (parameterVT);
        return true;
    });
}

juce::ValueTree ParameterDataListProperties::getParameter (juce::Identifier sectionTypeId, juce::String parameterName)
{
    auto section { data.getChildWithName (sectionTypeId) };
    jassert (section.isValid ());
    return section.getChildWithProperty ("name", parameterName);
}

void ParameterDataListProperties::initValueTree ()
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

void ParameterDataListProperties::validateParameterData ()
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
