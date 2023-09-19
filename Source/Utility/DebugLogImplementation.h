#pragma once

#include <JuceHeader.h>

class ThreadedLogger : private juce::Thread
{
public:
    ThreadedLogger () : juce::Thread ("ThreadedLogger") { startThread (); }
    ~ThreadedLogger () { flush ();  stopThread (5000); }

    void logMsg (juce::String msg)
    {
        juce::ScopedLock sl (debugMsgContainerLock);
        activeLogContainer->add (msg);
    }

    void flush ()
    {
        // this will block all calls to logMsg while it writes out the current active log container
        juce::ScopedLock sl (debugMsgContainerLock);
        writeBufferToLog (activeLogContainer);
    }

private:
    juce::CriticalSection debugMsgContainerLock;
    std::array<juce::StringArray, 2> logContainer;
    juce::StringArray* activeLogContainer { &logContainer[0] };
    inline static const int sleepTime { 100 };

    void writeBufferToLog (juce::StringArray* logContainerToWrite)
    {
        for (const auto& msg : *logContainerToWrite)
            juce::Logger::writeToLog (msg);
        logContainerToWrite->clear ();
    }

    void run () override
    {
        while (! threadShouldExit ())
        {
            sleep (sleepTime);
            // take a reference to the active log container, so we can write it out
            juce::StringArray* logContainerToWrite { activeLogContainer == &logContainer[0] ? &logContainer[0] : &logContainer[1] };
            {
                // swap active containers
                juce::ScopedLock sl (debugMsgContainerLock);
                activeLogContainer = (activeLogContainer == &logContainer [0] ? &logContainer [1] : &logContainer [0]);
            }
            writeBufferToLog (logContainerToWrite);
        }
    }
};