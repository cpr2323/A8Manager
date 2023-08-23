#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ParameterHelpers.h"

namespace FormatHelpers
{
    juce::String formatDouble (double value, int decimalPlaces, bool includeSign);
    void setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue);
    void setColorIfError (juce::TextEditor& textEditor, int minValue, int maxValue);
    juce::String getCvInput (const CvInputAndAmount& cvInputAndAmount);
    double getAmount (const CvInputAndAmount& cvInputAndAmount);
}