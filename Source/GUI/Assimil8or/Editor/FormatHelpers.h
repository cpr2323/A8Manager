#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ParameterHelpers.h"

namespace FormatHelpers
{
    juce::String constrainAndFormat (double value, double minValue, double maxValue, int decimalPlaces, bool includeSign);
    void setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue);
    juce::String constrainAndFormat (int value, int minValue, int maxValue);
    void setColorIfError (juce::TextEditor& textEditor, int minValue, int maxValue);
    juce::String getCvInput (const CvInputAndAmount& cvInputAndAmount);
    double getAmount (const CvInputAndAmount& cvInputAndAmount);
}