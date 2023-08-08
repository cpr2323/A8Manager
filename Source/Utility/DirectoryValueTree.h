#pragma once

#include <JuceHeader.h>

class DirectoryValueTree : public juce::Thread
{
public:
    DirectoryValueTree () : Thread ("Assimil8orValidator") {}
    DirectoryValueTree (juce::String theRootFolderName);
    ~DirectoryValueTree ();

    void setRootFolder (juce::String theRootFolderName);
    void setScanDepth (int theScanDepth);
    void startAsyncScan ();
    bool getScanStatus ();
    void clear ();
    juce::ValueTree getDirectoryVT ();

    std::function<void ()> onComplete;
    std::function<void (juce::String operation, juce::String fileName)> onStatusChange;

private:
    juce::String rootFolderName;
    int scanDepth { 0 };
    int64_t lastScanInProgressUpdate {};
    juce::ValueTree rootFolderVT;

    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void doScan ();
    void doStatusUpdate (juce::String operation, juce::String fileName);
    juce::ValueTree getContentsOfFolder (juce::File folder);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);

    void run () override;
};
