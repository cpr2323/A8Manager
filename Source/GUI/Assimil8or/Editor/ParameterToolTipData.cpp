#include "ParameterToolTipData.h"

// <Parameter name = "Data2asCV" tooltip = "Data 2 Knob CV Assignment, 1A - 8C, Default Off, No Relative Assignments (0A, 0B, OC)"/>

juce::String ParameterToolTipData::getToolTip (juce::String section, juce::String parameterName)
{
    if (! isValid ())
        return {};

    auto sectionVT { data.getChildWithName (section) };
    auto toolTipVT { sectionVT.getChildWithProperty ("name", parameterName) };
    jassert (toolTipVT.isValid ());
    return toolTipVT.getProperty ("tooltip");
}

void ParameterToolTipData::processValueTree ()
{
#if 1
    // process all of the tooltip strings for escape characters
    ValueTreeHelpers::forEachChild (data, [this] (juce::ValueTree sectionVT)
    {
        ValueTreeHelpers::forEachChild (sectionVT, [this] (juce::ValueTree tooltipVT)
        {
            auto rawToolTipString { tooltipVT.getProperty ("tooltip").toString () };
            juce::String processedToolTipString;

            auto escapeCharacterIndex { rawToolTipString.indexOfChar ('\\') };
            while (escapeCharacterIndex != -1)
            {
                if (escapeCharacterIndex > 0)
                    processedToolTipString += rawToolTipString.substring (0, escapeCharacterIndex);
                switch (rawToolTipString [escapeCharacterIndex + 1])
                {
                    case '\\' :
                    {
                        processedToolTipString += juce::String::charToString ('\\');
                    }
                    break;
                    case 'r':
                    {
                        processedToolTipString += juce::String::charToString ('\r');
                    }
                    break;
                    case 't':
                    {
                        processedToolTipString += juce::String::charToString ('\t');
                    }
                    break;
                }
                rawToolTipString = rawToolTipString.substring (escapeCharacterIndex + 2);
                escapeCharacterIndex = rawToolTipString.indexOfChar ('\\');
            }
            processedToolTipString += rawToolTipString;
            tooltipVT.setProperty ("tooltip", processedToolTipString, nullptr);
            return true;
        });
        return true;
    });
#endif
}
