#pragma once

#include <JuceHeader.h>
#include "ValidatorProperties.h"
#include "Preset/PresetProperties.h"

// validate
//  SD Card (max folders unknown)
//      Folder (31 character len) (max 199 presets) (no nested folders)
//          Preset (prst000.yml)
//          Preset
//          Audio (47 character len) (arbitrary name)
//          Audio
class Assimil8orSDCardValidator : public juce::Thread
{
public:
    Assimil8orSDCardValidator ();

    void init (juce::ValueTree vt);

private:
    juce::AudioFormatManager audioFormatManager;
    ValidatorProperties validatorProperties;
    int64_t lastScanInProgressUpdate {};
    juce::ValueTree rootFolderVT { "AssimilatorSdCardFileList" };

    void addStatus (juce::String statusType, juce::String statusText);
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    juce::ValueTree getContentsOfFolder (juce::File folder);
    bool isAudioFile (juce::File file);
    bool isPresetFile (juce::File file);
    void processFolder (juce::ValueTree folder);
    void sortContentsOfFolder (juce::ValueTree rootFolderVT);
    void validate ();
    std::tuple<juce::String, juce::String, std::optional<uint64_t>> validateFile (juce::File file);
    std::tuple<juce::String, juce::String> validateFolder (juce::File folder);
    void validateRootFolder ();

    void run () override;
};

// File Contents
//      Preset 1 (1-8 channels)
//          Channel 1 (? zones)
//              Zone 1
class Assimil8orPreset
{
public:
    Assimil8orPreset ();
    void write (juce::File presetFile);
    void write (juce::File presetFile, juce::ValueTree presetProperties);
    void parse (juce::StringArray presetLines);

    juce::ValueTree getPresetVT () { return presetProperties.getValueTree (); }

private:
    enum class ParseState
    {
        ParsingGlobalSection,
        ParsingPresetSection,
        ParsingChannelSection,
        ParsingZoneSection,
    };
    ParseState parseState { ParseState::ParsingGlobalSection };
    PresetProperties presetProperties;

    juce::String getParseStateString (ParseState parseState);
    void setParseState (ParseState newParseState);
};
