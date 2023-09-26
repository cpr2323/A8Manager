#include "FormatHelpers.h"

namespace FormatHelpers
{
    juce::String formatDouble (double value, int decimalPlaces, bool includeSign)
    {
        juce::String signString;
        if (includeSign && value >= 0.0)
            signString = "+";
        return signString + juce::String (value, decimalPlaces);
    };

    void setColorIfError (juce::TextEditor& textEditor, bool success)
    {
        if (success)
            textEditor.applyColourToAllText (juce::Colours::white, true);
        else
            textEditor.applyColourToAllText (juce::Colours::red, true);
    };

    void setColorIfError (juce::TextEditor& textEditor, juce::File sample)
    {
        setColorIfError (textEditor, sample.exists ());
    };

    void setColorIfError (juce::TextEditor& textEditor, double doubleValue, double minValue, double maxValue)
    {
        setColorIfError (textEditor, doubleValue >= minValue && doubleValue <= maxValue);
    };

    void setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue)
    {
        const auto doubleValue { textEditor.getText ().getDoubleValue () };
        setColorIfError (textEditor, doubleValue >= minValue && doubleValue <= maxValue);
    };

    void setColorIfError (juce::TextEditor& textEditor, int minValue, int maxValue)
    {
        const auto intValue { textEditor.getText ().getIntValue () };
        setColorIfError (textEditor, intValue >= minValue && intValue <= maxValue);
    };

    void setColorIfError (juce::TextEditor& textEditor, int64_t minValue, int64_t maxValue)
    {
        const auto int64Value { textEditor.getText ().getLargeIntValue () };
        setColorIfError (textEditor, int64Value >= minValue && int64Value <= maxValue);
    }

    juce::String getCvInput (const CvInputAndAmount& cvInputAndAmount)
    {
        auto [cvInput, _] {cvInputAndAmount};
        return cvInput;
    }

    double getAmount (const CvInputAndAmount& cvInputAndAmount)
    {
        auto [_, amount] {cvInputAndAmount};
        return amount;
}
};
