#include "ParameterPresetListProperties.h"
#include "PresetProperties.h"

const auto TypePropertyId { "_type" };
void ParameterPresetListProperties::forEachParameterPreset (std::function<bool (juce::ValueTree parameterVT)> parameterPresetVTCallback)
{
    ValueTreeHelpers::forEachChildOfType (data, PresetProperties::PresetTypeId, [parameterPresetVTCallback] (juce::ValueTree parameterPresetVT)
    {
        parameterPresetVTCallback (parameterPresetVT);
        return true;
    });
}

juce::ValueTree ParameterPresetListProperties::getParameterPreset (juce::String parameterPresetType)
{
    return data.getChildWithProperty (TypePropertyId, parameterPresetType);
}

void ParameterPresetListProperties::initValueTree ()
{
    auto addParameterPresetList = [this] (juce::String parameterPresetType, const char* parameterPresetXml)
    {
        auto getPresetPropertiesForType = [] (juce::String parameterPresetType, const char* parameterPresetXml)
        {
            juce::XmlDocument xmlDoc { parameterPresetXml };
            auto xmlElement { xmlDoc.getDocumentElement (false) };
            if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
                juce::Logger::outputDebugString ("XML Parsing Error for ParameterPreset type '" + parameterPresetType + "': " + parseError);
            // NOTE: this is a hard failure, which indicates there is a problem in the file the parameterPresetXml passed in
            jassert (xmlDoc.getLastParseError () == "");
            if (xmlElement == nullptr)
                return juce::ValueTree ();

            auto parameterPreset { juce::ValueTree::fromXml (*xmlElement) };
            return parameterPreset;
        };
        auto parameterPresetVT { getPresetPropertiesForType (parameterPresetType, parameterPresetXml) };
        parameterPresetVT.setProperty (TypePropertyId, parameterPresetType, nullptr);
        data.addChild (parameterPresetVT, -1, nullptr);
    };

    addParameterPresetList (DefaultParameterPresetType, BinaryData::DefaultPreset_xml);
    addParameterPresetList (MinParameterPresetType, BinaryData::MinPreset_xml);
    addParameterPresetList (MaxParameterPresetType, BinaryData::MaxPreset_xml);
}
