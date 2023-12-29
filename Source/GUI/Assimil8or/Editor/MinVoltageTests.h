#pragma once

#include <JuceHeader.h>

namespace MinVoltageTests
{
    void runDistributionTests ()
    {
        auto generateData = [] (std::vector<float>& inputData, int startIndex, int endIndex) -> std::vector<float>
            {
                const auto maxValue { 5.0f };
                const auto minValue { -5.0f };
                const auto initialDataSize { static_cast<int>(inputData.size ()) };
                const auto initialEndIndex { initialDataSize - 1 };

                // If the provided array is smaller than endIndex, resize it
                std::vector<float> outputData { inputData.begin (), inputData.end () };
                if (endIndex < inputData.size ())
                    return outputData;

                outputData.resize (endIndex + 1);

                if (endIndex - initialEndIndex > 0)
                {
                    const auto initialValue { startIndex == 0 || (startIndex == 1 && initialDataSize == 1) ? maxValue : outputData [initialEndIndex - 1] };
                    const auto initialIndex { startIndex > 0 && startIndex == initialDataSize ? startIndex - 1 : startIndex };
                    // Calculate the step size for even distribution
                    const float stepSize { (minValue - initialValue) / (endIndex - initialIndex + 1) };
                    const auto updateThreshold { initialEndIndex - 1 };

                    // Update values from the start index to the new end index
                    for (int i = initialIndex; i < endIndex; ++i)
                    {
                        if (i > updateThreshold)
                            outputData [i] = initialValue + (i - initialIndex + 1) * stepSize;
                    }
                }

                outputData [outputData.size () - 1] = minValue;
                return outputData;
            };

        auto runTest = [&generateData] (std::vector<float>& testData, int startIndex, int endIndex, std::vector<float> expectedOutputData)
            {
                auto simpleCompare = [] (float firstNumber, float secondNumber)
                    {
                        const auto multiplier { 100 };
                        const auto firstNumberMultiplied { static_cast<int>(firstNumber * multiplier) };
                        const auto secondNumberMultiplied { static_cast<int>(secondNumber * multiplier) };
                        const auto approxEqual { std::abs (firstNumberMultiplied - secondNumberMultiplied) < 2 };
                        jassert (approxEqual);
                        return approxEqual;
                    };
                auto outputData { generateData (testData, startIndex, endIndex) };

                jassert (outputData.size () == expectedOutputData.size ());
                for (auto dataIndex { 0 }; dataIndex < outputData.size (); ++dataIndex)
                {
                    jassert (simpleCompare (outputData [dataIndex], expectedOutputData [dataIndex]) == true);
                }
            };

        std::vector<float> inputData;
        runTest (inputData, 0, 0, { -5.0f });
        inputData.clear ();
        runTest (inputData, 0, 1, { 0.0f, -5.0f });
        inputData.clear ();
        runTest (inputData, 0, 2, { 1.67f, -1.67f, -5.0f });
        inputData.clear ();
        runTest (inputData, 0, 3, { 2.50f, 0.00f, -2.50f, -5.0f });
        inputData.clear ();
        runTest (inputData, 0, 7, { 3.75f, 2.50f, 1.25f, 0.00f, -1.25f, -2.50f, -3.75f, -5.0f });
        inputData = { -5.0f };
        runTest (inputData, 0, 0, { -5.0f });
        inputData = { -5.0f };
        runTest (inputData, 0, 1, { 0.0f, -5.0f });
        inputData = { -5.0f };
        runTest (inputData, 1, 1, { 0.0f, -5.0f });
        inputData = { 0.0f, -5.0f };
        runTest (inputData, 1, 2, { 0.0f, -2.5f, -5.0f });
        inputData = { 0.0f, -5.0f };
        runTest (inputData, 2, 2, { 0.0f, -2.5f, -5.0f });
        inputData = { 0.0, -2.5f, -5.0f };
        runTest (inputData, 2, 3, { 0.0f, -2.5f, -3.75f, -5.0f });
        inputData = { 0.0f, -2.5f, -5.0f };
        runTest (inputData, 2, 4, { 0.0f, -2.5f, -3.33f, -4.16f, -5.0f });
        inputData = { 0.0f, -2.5f, -3.33f, -4.16f, -5.0f };
        runTest (inputData, 1, 3, { 0.0f, -2.5f, -3.33f, -4.16f, -5.0f });
        inputData = { 0.0f, -2.5f, -3.33f, -4.16f, -5.0f };
        runTest (inputData, 2, 6, { 0.0f, -2.5f, -3.33f, -4.16f, -4.66f, -4.83f, -5.0f });
    }
};
