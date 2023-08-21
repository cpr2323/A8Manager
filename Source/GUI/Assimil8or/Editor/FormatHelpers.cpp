#include "FormatHelpers.h"

namespace FormatHelpers
{
    juce::String checkAndFormatDouble (double value, double minValue, double maxValue, int decimalPlaces, bool includeSign)
    {
        if (value < minValue)
            value = minValue;
        else if (value > maxValue)
            value = maxValue;
        juce::String signString;
        if (includeSign && value >= 0.0)
            signString = "+";
        return signString + juce::String (value, decimalPlaces);
    };

    void setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue)
    {
        const auto doubleValue { textEditor.getText ().getDoubleValue () };
        if (doubleValue >= minValue && doubleValue <= maxValue)
            textEditor.applyColourToAllText (juce::Colours::white, true);
        else
            textEditor.applyColourToAllText (juce::Colours::red, true);
    };
};
