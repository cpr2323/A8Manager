#pragma once

#include <JuceHeader.h>
#include "Preset/PresetProperties.h"
#include "Validator/ValidatorProperties.h"
#include "Validator/ValidatorResultListProperties.h"

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
    
    std::map<juce::String, std::function<void()>> actions;
//    std::map<juce::String, std::function<void(const juce::String & value)>> channelActions;
//    std::map<juce::String, std::function<void(const juce::String & value)>> zoneActions;
    juce::ValueTree curPresetSection;
    ChannelProperties channelProperties;
    juce::ValueTree curChannelSection;
    ZoneProperties zoneProperties;
    juce::ValueTree curZoneSection;
    juce::String key;
    juce::String value;

    juce::String getParseStateString (ParseState parseState);
    void setParseState (ParseState newParseState);
    void dispatch();
    void clearVars();
};
