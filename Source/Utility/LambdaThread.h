#pragma once

#include <JuceHeader.h>

class LambdaThread : public juce::Thread
{
public:
    LambdaThread (juce::String threadName, int stopTimeout) : Thread (threadName)
    {
        timeout = stopTimeout;
    }
    ~LambdaThread () noexcept { stop (); }

    void start () { jassert (onThreadLoop != nullptr); startThread (); }
    void stop () { stopThread (timeout); }
    void sleep (int ms) { Thread::sleep (ms); }
    bool setPriority (Priority priority) { return Thread::setPriority (priority); };

    std::function<bool ()> onThreadLoop;

private:
    int timeout { 1 };
    void run () override
    {
        jassert (onThreadLoop != nullptr);
        while (! threadShouldExit () && onThreadLoop ());
    }
};
