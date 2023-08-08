#pragma once

#include <JuceHeader.h>

class DirectoryValueTree : public juce::Thread
{
public:
    DirectoryValueTree () : Thread ("Assimil8orValidator") {}
    DirectoryValueTree (juce::String theRootFolderName);
    ~DirectoryValueTree ();

    void setRootFolder (juce::String theRootFolderName);
    juce::ValueTree getDirectoryVT ();
    void startAsyncRead ();
    bool getReadStatus ();
    void clear ();

    std::function<void (bool readStatus)> onReadStatusChange;

private:
    juce::String rootFolderName;
    int64_t lastScanInProgressUpdate {};
    juce::ValueTree rootFolderVT;

    juce::ValueTree getContentsOfFolder (juce::File folder);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    void validateRootFolder ();

    void run () override;
};
