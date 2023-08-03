#pragma once

#include <JuceHeader.h>
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

    void init (juce::ValueTree vt);

    static bool isAudioFile (juce::File file);
    static bool isPresetFile (juce::File file);
    static bool isFolderPrefsFile (juce::File file);
    static int  getPresetNumberFromName (juce::File file);

private:
    juce::AudioFormatManager audioFormatManager;
    ValidatorProperties validatorProperties;
    ValidatorResultListProperties validatorResultListProperties;
    int64_t lastScanInProgressUpdate {};
    juce::ValueTree rootFolderVT { "AssimilatorFileList" };

    void addResult (juce::String statusType, juce::String statusText);
    void addResult (juce::ValueTree validatorResultsVT);
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    juce::ValueTree getContentsOfFolder (juce::File folder);
    void processFolder (juce::ValueTree folder);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    void validate ();
    std::optional<uint64_t> validateFile (juce::File file, juce::ValueTree validatorResultsVT);
    void validateFolder (juce::File folder, juce::ValueTree validatorResultsVT);
    void validateRootFolder ();

    void run () override;
};
