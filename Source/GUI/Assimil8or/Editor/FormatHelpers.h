#pragma once

#include <JuceHeader.h>
// TODO - I am including this juce for CvInputAndAmount. move that to another file eventually
#include "../../../Assimil8or/Preset/ChannelProperties.h"

namespace FormatHelpers
{
    juce::String checkAndFormat (double value, double minValue, double maxValue, int decimalPlaces, bool includeSign);
    void setColorIfError (juce::TextEditor& textEditor, double minValue, double maxValue);
    juce::String checkAndFormat (int value, int minValue, int maxValue);
    void setColorIfError (juce::TextEditor& textEditor, int minValue, int maxValue);
    juce::String getCvInput (const CvInputAndAmount& cvInputAndAmount);
    double getAmount (const CvInputAndAmount& cvInputAndAmount);
};
