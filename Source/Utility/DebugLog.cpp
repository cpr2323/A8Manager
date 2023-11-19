#include "DebugLog.h"
#include "DebugLogImplementation.h"

#define USE_DEFERRED_LOGGER 1

static auto startTime { juce::Time::currentTimeMillis () };
static auto lastTime { startTime };

#if USE_DEFERRED_LOGGER
static auto debugLogger { ThreadedLogger () };
#endif

namespace DebugLogger
{
    using ThreadIdAndName = std::pair<juce::Thread::ThreadID, juce::String>;
    std::vector<ThreadIdAndName> UnnamedThreads;
    juce::CriticalSection UnnamedThreadsCS;
    void addUnnamedThread (juce::Thread::ThreadID threadID, juce::String threadName)
    {
        juce::ScopedLock sl (UnnamedThreadsCS);
        UnnamedThreads.push_back ({ threadID, threadName });
    }
    juce::String getUnnamedThread (juce::Thread::ThreadID threadID)
    {
        juce::ScopedLock sl (UnnamedThreadsCS);
        auto threadIdAndNameIter { std::find_if (UnnamedThreads.begin (), UnnamedThreads.end (), [threadID] (ThreadIdAndName threadIdAndName)
            {
                return std::get<0> (threadIdAndName) == threadID;
            }) };
        if (threadIdAndNameIter == UnnamedThreads.end ())
            return "";
        return std::get<1> (*threadIdAndNameIter);
    }
}

void FlushDebugLog ()
{
#if USE_DEFERRED_LOGGER
    debugLogger.flush ();
#endif
}

void DebugLog (juce::String moduleName, juce::String logLine)
{
    const auto curTime { juce::Time::currentTimeMillis () };
    const auto curThreadId { juce::String::toHexString ((uint64_t) juce::Thread::getCurrentThreadId ()).paddedLeft ('0', 8) };
    const auto curThread { juce::Thread::getCurrentThread () };
    const auto curThreadName { juce::MessageManager::existsAndIsCurrentThread () ? "MessageManager" : (curThread != nullptr ? juce::Thread::getCurrentThread ()->getThreadName () : "") };
    const auto possibleUnnamedThreadName { DebugLogger::getUnnamedThread (juce::Thread::getCurrentThreadId ())};
    const auto threadIdToDisplay { curThreadName.isNotEmpty () ? curThreadName : (possibleUnnamedThreadName.isNotEmpty () ? possibleUnnamedThreadName : curThreadId) };
    const auto timeSinceStart { juce::String (curTime - startTime).paddedLeft ('0', 10) };
    const auto timeSincePreviousLog { juce::String (curTime - lastTime).paddedLeft ('0', 4) };
    const juce::String logMsg { "[" + timeSinceStart + "]+" + timeSincePreviousLog + " [t:" + threadIdToDisplay + "]" + " (" + moduleName + ") -> " + logLine };
#if USE_DEFERRED_LOGGER
    debugLogger.logMsg (logMsg);
#else
    juce::Logger::writeToLog (logMsg);
#endif
    lastTime = curTime;
}
