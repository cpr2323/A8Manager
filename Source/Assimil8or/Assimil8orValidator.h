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
    AppProperties appProperties;
    ValidatorProperties validatorProperties;
    ValidatorResultListProperties validatorResultListProperties;
    juce::AudioFormatManager audioFormatManager;
    int64_t lastScanInProgressUpdate {};
    juce::ValueTree rootFolderVT;
    DirectoryValueTree directoryValueTree;

    void addResult (juce::String statusType, juce::String statusText);
    void addResult (juce::ValueTree validatorResultsVT);
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void processFolder (juce::ValueTree folder);
    void validate ();
    std::tuple<uint64_t, std::optional<uint64_t>> validateFile (juce::File file, juce::ValueTree validatorResultsVT);
    void validateFolder (juce::File folder, juce::ValueTree validatorResultsVT);
    void validateRootFolder ();

    void run () override;
};
