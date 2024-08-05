#pragma once

#include <JuceHeader.h>

namespace ErrorHelpers
{
    bool setColorIfError (juce::TextEditor& textEditor, bool success);
    bool setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue);
    bool setColorIfError (juce::TextEditor& textEditor, double doubleValue, double minValue, double maxValue);
    bool setColorIfError (juce::TextEditor& textEditor, int minValue, int maxValue);
    bool setColorIfError (juce::TextEditor& textEditor, juce::int64 minValue, juce::int64 maxValue);
    bool setColorIfError (juce::TextEditor& textEditor, juce::File sample);
};