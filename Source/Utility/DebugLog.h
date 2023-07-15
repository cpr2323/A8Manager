#pragma once

#include <JuceHeader.h>

static auto lockDelayLogThreshold { 5 };

namespace DebugLogger
{
    void addUnnamedThread (juce::Thread::ThreadID threadID, juce::String threadName);
    juce::String getUnnamedThread (juce::Thread::ThreadID threadID);
}

void DebugLog (juce::String moduleName, juce::String logLine);
void FlushDebugLog ();
