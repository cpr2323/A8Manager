#pragma once

// <ToolTips note = "Description/min, max, and default values/special behavior (increment amounts, etc), list of options for option lists?">
//      <Preset>
//           <Parameter name = "Data2asCV" tooltip = "Data 2 Knob CV Assignment, 1A - 8C, Default Off, No Relative Assignments (0A, 0B, OC)"/>
//           <Parameter name = "Name" tooltip = "Name, Max Length is 12"/>
//      </Preset>
//      <Channel>
//          <Parameter name = "Aliasing" tooltip = "Aliasing Amount, 0% - 100%"/>
//      </Channel>
// </ToolTips>

#include <JuceHeader.h>
#include "../../../Utility/ValueTreeWrapper.h"

class ParameterToolTipData : public ValueTreeWrapper<ParameterToolTipData>
{
public:
    ParameterToolTipData () noexcept : ValueTreeWrapper<ParameterToolTipData> (ParameterToolTipDataTypeId)
    {
    }
    ParameterToolTipData (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ParameterToolTipData> (ParameterToolTipDataTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    juce::String getToolTip (juce::String section, juce::String parameterName);

    static inline const juce::Identifier ParameterToolTipDataTypeId { "ToolTips" };

    void initValueTree () {}
    void processValueTree () {}

private:
};