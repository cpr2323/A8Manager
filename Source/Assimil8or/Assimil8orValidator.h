#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"
#include "../Utility/DirectoryValueTree.h"
#include "Validator/ValidatorProperties.h"

// validate
//  SD Card (max folders unknown)
//      Folder (31 character len) (max 199 presets) (no nested folders)
//          Preset (prst000.yml)
//          Preset
//          Audio (47 character len) (arbitrary name)
//          Audio
class Assimil8orValidator : public juce::Thread
{
public:
    Assimil8orValidator ();
    ~Assimil8orValidator ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    enum class ValdatationState
    {
        idle,
        readingFolder,
        validating
    };
    AppProperties appProperties;
    ValidatorProperties validatorProperties;
    ValidatorResultListProperties validatorResultListProperties;
    juce::AudioFormatManager audioFormatManager;
    DirectoryValueTree directoryValueTree;
    juce::ValueTree rootFolderVT;
    int64_t lastScanInProgressUpdate {};
    ValdatationState valdatationState { ValdatationState::readingFolder };
    juce::File rootFolderToScan;
    juce::File queuedFolderToScan;
    juce::CriticalSection queuedFolderLock;
    std::atomic<bool> newItemQueued { false };

    void addResult (juce::String statusType, juce::String statusText);
    void addResult (juce::ValueTree validatorResultsVT);
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void processFolder (juce::ValueTree folder);
    bool shouldCancelOperation ();
    void startValidation (juce::String folderToScan);
    std::tuple<uint64_t, std::optional<uint64_t>> validateFile (juce::File file, juce::ValueTree validatorResultsVT);
    void validateFolder (juce::File folder, juce::ValueTree validatorResultsVT);
    void validateRootFolder ();

    void run () override;
};
