#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class WatchdogTimer : public juce::Timer
{
public:
    WatchdogTimer () = default;
    ~WatchdogTimer ()
    {
        stop ();
    }

    void start(int timeOut)
    {
        timeExpired = false;
        startTimer (timeOut);
        startTime = juce::Time::currentTimeMillis ();
    }

    void stop ()
    {
        stopTimer ();
    }

    bool hasTimedOut ()
    {
        return timeExpired;
    }

    int64_t getElapsedTime ()
    {
        if (timeExpired)
            return std::numeric_limits<int64_t>::max ();
        else
            return juce::Time::currentTimeMillis () - startTime;
    }
private:
    bool timeExpired { true };
    int64_t startTime {};

    void timerCallback () override
    {
        timeExpired = true;
        stop ();
    }
};
