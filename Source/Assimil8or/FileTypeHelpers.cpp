#include "FileTypeHelpers.h"

namespace FileTypeHelpers
{
    DirectoryDataProperties::TypeIndex getFileType (juce::File file)
    {
        if (FileTypeHelpers::isSystemFile (file))
            return DirectoryDataProperties::TypeIndex::systemFile;
        else if (FileTypeHelpers::isPresetFile (file))
            return DirectoryDataProperties::TypeIndex::presetFile;
        else if (FileTypeHelpers::isSupportedAudioFile (file))
            return DirectoryDataProperties::TypeIndex::audioFile;
        else // unknown file
            return DirectoryDataProperties::TypeIndex::unknownFile;
    }

    juce::String getPresetFileName (int presetIndex)
    {
        jassert (presetIndex > 0 && presetIndex <= kMaxPresets);
        const auto rawPresetIndexString { juce::String (presetIndex) };
        const auto presetIndexString { juce::String ("000").substring (0, 3 - rawPresetIndexString.length ()) + rawPresetIndexString };
        return kPresetFileNamePrefix + presetIndexString;
    }

    int getPresetNumberFromName (juce::File file)
    {
        if (! isPresetFile (file))
            return kBadPresetNumber;
        return file.getFileNameWithoutExtension ().substring (kPresetFileNumberOffset).getIntValue ();
    }

    bool isSupportedAudioFile (juce::File file)
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
        return isFolderPrefsFile (file) || isLastFolderFile (file) || isLastPresetFile (file) || isMidiSetupFile (file);
    }

    bool isPresetFile (juce::File file)
    {
        return file.getFileExtension ().toLowerCase () == kYmlFileExtension &&
               file.getFileNameWithoutExtension ().length () == kPresetFileNameLen &&
               file.getFileNameWithoutExtension ().toLowerCase ().startsWith (kPresetFileNamePrefix) &&
               file.getFileNameWithoutExtension ().substring (kPresetFileNumberOffset).containsOnly ("0123456789");
    }

    bool isMidiSetupFile (juce::File file)
    {
        return file.getFileExtension ().toLowerCase () == kYmlFileExtension &&
               file.getFileNameWithoutExtension ().length () == kMidiSetupFileNameLen &&
               file.getFileNameWithoutExtension ().toLowerCase ().startsWith (kMidiSetupFileNamePrefix) &&
               file.getFileNameWithoutExtension ().substring (kMidiSetupNumberOffset, kMidiSetupNumberOffset + 1).containsOnly ("123456789");
    }
};
