#pragma once

#include <JuceHeader.h>

namespace FileTypeHelpers
{
    const juce::String kWaveFileExtension { ".wav" };
    const juce::String kYmlFileExtension { ".yml" };
    const juce::String kFolderPrefsFileName { "folderprefs.yml" };
    const juce::String kLastFolderFileName { "lastfolder.yml" };
    const juce::String kLastPresetFileName { "lastpreset.yml" };
    const juce::String kPresetFileNamePrefix { "prst" };
    const auto kPresetFileNameLen { 7 };
    const auto kPresetFileNumberOffset { 4 };
    const juce::String kMidiSetupFileNamePrefix { "midi" };
    const auto kMidiSetupFileNameLen { 5 };
    const auto kMidiSetupNumberOffset { 4 };

    const auto kMaxPresets { 199 };
    const auto kBadPresetNumber { 9999 };

    // the names A8Manager registers its file types under with DirectoryValueTree. clients turn one of
    // these into the id stored on a directory entry via DirectoryDataProperties::getFileTypeId
    const juce::String kSystemFileTypeName { "system" };
    const juce::String kPresetFileTypeName { "preset" };
    const juce::String kAudioFileTypeName { "audio" };

    juce::String getPresetFileName (int presetIndex);
    int  getPresetNumberFromName (juce::File file);
    bool isPresetFile (juce::File file);
    bool isFolderPrefsFile (juce::File file);
    bool isLastFolderFile (juce::File file);
    bool isLastPresetFile (juce::File file);
    bool isSystemFile (juce::File file);
    bool isMidiSetupFile (juce::File file);
    bool isAudioFile (juce::File file);
};
