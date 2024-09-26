#pragma once

#include <JuceHeader.h>

class LambdaThread : public juce::Thread
{
public:
    LambdaThread (juce::String threadName, int theStopTimeout) : Thread (threadName)
    {
        stopTimeout = theStopTimeout;
    }
    ~LambdaThread () noexcept { stop (); }

    bool isWaiting () { return waiting; }
    void start () { jassert (onThreadLoop != nullptr); startThread (); }
    void stop () { stopThread (stopTimeout); }
    void sleep (int ms) { Thread::sleep (ms); }
    bool setPriority (Priority priority) { return Thread::setPriority (priority); }
    void wake () { notify (); }
    bool waitForNotification (double timeout) { waiting = true; const auto returnValue { wait (timeout) }; waiting = false; return returnValue; }
    bool shouldExit () { return threadShouldExit (); }

    std::function<bool ()> onThreadLoop;

private:
    int stopTimeout { 1 };
    std::atomic<bool> waiting { false };
    void run () override
    {
        jassert (onThreadLoop != nullptr);
        while (! threadShouldExit () && onThreadLoop ());
    }
};
