#pragma once

#include <JuceHeader.h>

void dumpStacktrace (int depth, std::function<void (juce::String)> logger);
