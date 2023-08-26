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
