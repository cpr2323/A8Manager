#include "FileTypeHelpers.h"

namespace FileTypeHelpers
{
    bool isAudioFile (juce::File file)
    {
        return file.getFileExtension ().toLowerCase () == kWaveFileExtension;
    }

    bool isFolderPrefsFile (juce::File file)
    {
        return file.getFileName ().toLowerCase () == kFolderPrefsFileName;
    }
    bool isLastFolderFile (juce::File file)
    {
        return file.getFileName ().toLowerCase () == kLastFolderFileName;
    }

    bool isLastPresetFile (juce::File file)
    {
        return file.getFileName ().toLowerCase () == kLastPresetFileName;
    }

    bool isSystemFile (juce::File file)
    {
        return isFolderPrefsFile (file) || isLastFolderFile (file) || isLastPresetFile (file);
    }

    bool isPresetFile (juce::File file)
    {
        return file.getFileExtension ().toLowerCase () == kYmlFileExtension &&
            file.getFileNameWithoutExtension ().length () == kPresetFileNameLen &&
            file.getFileNameWithoutExtension ().toLowerCase ().startsWith (kPresetFileNamePrefix) &&
            file.getFileNameWithoutExtension ().substring (kPresetFileNumberOffset).containsOnly ("0123456789");
    }

    int getPresetNumberFromName (juce::File file)
    {
        if (! isPresetFile (file))
            return kBadPresetNumber;
        return file.getFileNameWithoutExtension ().substring (kPresetFileNumberOffset).getIntValue ();
    }

    juce::String getPresetFileName (int presetIndex)
    {
        jassert (presetIndex < kMaxPresets);
        const auto rawPresetIndexString { juce::String (presetIndex) };
        const auto presetIndexString { juce::String ("000").substring (0, 3 - rawPresetIndexString.length ()) + rawPresetIndexString };
        return kPresetFileNamePrefix + presetIndexString;
    }
};
