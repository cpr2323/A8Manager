#include "DumpStack.h"

void dumpStacktrace (int depth, std::function<void (juce::String)> logger)
{
    jassert (logger != nullptr);
    auto rawStackTrace { juce::SystemStats::getStackBacktrace () };
    auto stackTraceLines { juce::StringArray::fromLines (rawStackTrace) };
    // 0: getStackBacktrace
    // 1: getStacktrace
    // 2: this should be the first function name of interest
    const auto kStartOffset { 2 };
    for (auto stackLineIndex { 0 }; (depth == -1 || stackLineIndex < depth) && stackLineIndex < stackTraceLines.size () - kStartOffset; ++stackLineIndex)
    {
        auto& curLine { stackTraceLines [kStartOffset + stackLineIndex] };
        if (curLine.isNotEmpty ())
            logger (curLine);
    }
}
