#pragma once

#include <JuceHeader.h>

namespace FormatHelpers
{
    juce::String checkAndFormatDouble (double value, double minValue, double maxValue, int decimalPlaces, bool includeSign);
    void setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue);
};
