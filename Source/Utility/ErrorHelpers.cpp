#include "ErrorHelpers.h"

namespace ErrorHelpers
{
    bool setColorIfError (juce::TextEditor& textEditor, bool success)
    {
        if (success)
            textEditor.applyColourToAllText (juce::Colours::white, true);
        else
            textEditor.applyColourToAllText (juce::Colours::red, true);

        return success;
    }

    bool setColorIfError (juce::TextEditor& textEditor, juce::File sample)
    {
        return setColorIfError (textEditor, sample.exists ());
    }

    bool setColorIfError (juce::TextEditor& textEditor, double doubleValue, double minValue, double maxValue)
    {
        return setColorIfError (textEditor, doubleValue >= minValue && doubleValue <= maxValue);
    }

    bool setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue)
    {
        const auto doubleValue { textEditor.getText ().getDoubleValue () };
        return setColorIfError (textEditor, doubleValue >= minValue && doubleValue <= maxValue);
    }

    bool setColorIfError (juce::TextEditor& textEditor, int minValue, int maxValue)
    {
        const auto intValue { textEditor.getText ().getIntValue () };
        return setColorIfError (textEditor, intValue >= minValue && intValue <= maxValue);
    }

    bool setColorIfError (juce::TextEditor& textEditor, juce::int64 minValue, juce::int64 maxValue)
    {
        const auto int64Value { textEditor.getText ().getLargeIntValue () };
        return setColorIfError (textEditor, int64Value >= minValue && int64Value <= maxValue);
    }
};
