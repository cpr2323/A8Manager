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

    const auto kMaxPresets { 199 };
    const auto kBadPresetNumber { 9999 };

    bool isAudioFile (juce::File file);
    bool isPresetFile (juce::File file);
    bool isFolderPrefsFile (juce::File file);
    bool isLastFolderFile (juce::File file);
    bool isLastPresetFile (juce::File file);
    bool isSystemFile (juce::File file);
    int  getPresetNumberFromName (juce::File file);
    juce::String getPresetFileName (int presetIndex);
};