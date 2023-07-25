#pragma once

#include <JuceHeader.h>
#include "ValidatorProperties.h"

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
    void parse (juce::StringArray presetLines);

    juce::ValueTree getPresetVT () { return assimil8orData; }

private:
    enum class ParseState
    {
        ParsingGlobalSection,
        ParsingPresetSection,
        ParsingChannelSection,
        ParsingZoneSection,
    };
    ParseState parseState { ParseState::ParsingGlobalSection };

    juce::ValueTree assimil8orData { "Assimil8or" };

    juce::Identifier PresetSectionId  { "Preset" };
    juce::Identifier PresetNamePropertyId { "Name" };

    juce::Identifier ChannelSectionId { "Channel" };

    juce::Identifier ZoneSectionId    { "Zone" };

    juce::String getParseStateString (ParseState parseState);
    void setParseState (ParseState newParseState);
};
