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
