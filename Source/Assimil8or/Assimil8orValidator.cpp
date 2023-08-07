#include "Assimil8orValidator.h"
#include "Assimil8orPreset.h"
#include "Validator/ValidatorResultProperties.h"

#define LOG_VALIDATION 0
#if LOG_VALIDATION
#define LogValidation(text) juce::Logger::outputDebugString (text);
#else
#define LogValidation(text) ;
#endif

const double oneK { 1024.0f };
const double oneMB { oneK * oneK };
const double oneGB { oneMB * oneK };
const auto maxMemory { static_cast<int> (422 * oneMB) };
const auto maxPresets { 199 };
const juce::String kPresetFileNamePrefix { "prst" };
const auto kPresetFileNameLen { 7 };
const auto kPresetFileNumberOffset { 4 };
const juce::String kWaveFileExtension { ".wav" };
const juce::String kYmlFileExtension { ".yml" };
const juce::String kFolderPrefsFileName { "folderprefs.yml" };
const juce::String kLastFolderFileName { "lastfolder.yml" };
const juce::String kLastPresetFileName { "lastpreset.yml" };

const auto kBadPresetNumber { 9999 };
const auto bytesPerSampleInAssimMemory { 4 };

juce::String getMemorySizeString (uint64_t memoryUsage)
{
    auto formatString = [] (double usage, juce::String postFix)
    {
        return (usage == 0 ? "0" : juce::String (usage, 2).trimCharactersAtEnd ("0.")) + postFix;
    };
    if (memoryUsage >= oneGB)
        return formatString (static_cast<float>(memoryUsage) / oneGB, "GB");
    else if (memoryUsage >= oneMB)
        return formatString (static_cast<float>(memoryUsage) / oneMB, "MB");
    else if (memoryUsage >= oneK)
        return formatString (static_cast<float>(memoryUsage) / oneK, "k");
    else
        return formatString (static_cast<float>(memoryUsage), " bytes");
};

Assimil8orValidator::Assimil8orValidator () : Thread ("Assimil8orValidator")
{
    audioFormatManager.registerBasicFormats ();
}

Assimil8orValidator::~Assimil8orValidator ()
{
    stopThread (500);
}

void Assimil8orValidator::init (juce::ValueTree vt)
{
    validatorProperties.wrap (vt, ValidatorProperties::WrapperType::owner, ValidatorProperties::EnableCallbacks::yes);
    validatorProperties.onStartScanAsync = [this] () { validate (); };
    validatorResultListProperties.wrap (validatorProperties.getValueTree (),
                                        ValidatorResultListProperties::WrapperType::client, ValidatorResultListProperties::EnableCallbacks::no);
}

void Assimil8orValidator::validate ()
{
    if (isThreadRunning ())
        return;
    validatorProperties.setScanStatus ("scanning", false);
    startThread ();
}

void Assimil8orValidator::addResult (juce::String statusType, juce::String statusText)
{
    ValidatorResultProperties validatorResultProperties;
    validatorResultProperties.update (statusType, statusText, false);
    addResult (validatorResultProperties.getValueTree ());
};

void Assimil8orValidator::addResult (juce::ValueTree validatorResultsVT)
{
    ValidatorResultProperties validatorResultProperties (validatorResultsVT,
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
    jassert (validatorResultProperties.getType () != ValidatorResultProperties::ResultTypeNone);
    if (validatorResultProperties.getType () != ValidatorResultProperties::ResultTypeNone)
        validatorResultListProperties.addResult (validatorResultsVT);
};

void Assimil8orValidator::doIfProgressTimeElapsed (std::function<void ()> functionToDo)
{
    jassert (functionToDo != nullptr);
    if (juce::Time::currentTimeMillis () - lastScanInProgressUpdate > 250)
    {
        lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
        functionToDo ();
    }
}

bool Assimil8orValidator::isAudioFile (juce::File file)
{
    return file.getFileExtension ().toLowerCase () == kWaveFileExtension;
}

bool Assimil8orValidator::isFolderPrefsFile (juce::File file)
{
    return file.getFileName ().toLowerCase () == kFolderPrefsFileName;
}
bool Assimil8orValidator::isLastFolderFile (juce::File file)
{
    return file.getFileName ().toLowerCase () == kLastFolderFileName;
}

bool Assimil8orValidator::isLastPresetFile (juce::File file)
{
    return file.getFileName ().toLowerCase () == kLastPresetFileName;
}

bool Assimil8orValidator::isPresetFile (juce::File file)
{
    return file.getFileExtension ().toLowerCase () == kYmlFileExtension &&
           file.getFileNameWithoutExtension ().length () == kPresetFileNameLen &&
           file.getFileNameWithoutExtension ().toLowerCase().startsWith (kPresetFileNamePrefix) &&
           file.getFileNameWithoutExtension ().substring (kPresetFileNumberOffset).containsOnly ("0123456789");
}

int Assimil8orValidator::getPresetNumberFromName (juce::File file)
{
    if (! isPresetFile (file))
        return kBadPresetNumber;
    return file.getFileNameWithoutExtension ().substring (kPresetFileNumberOffset).getIntValue ();
}

std::tuple<uint64_t, std::optional<uint64_t>> Assimil8orValidator::validateFile (juce::File file, juce::ValueTree validatorResultsVT)
{
    const auto kMaxFileNameLength { 47 };
    ValidatorResultProperties validatorResultProperties (validatorResultsVT,
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);

    std::optional<uint64_t> optionalPresetInfo;
    LogValidation ("File: " + file.getFileName ());
    if (file.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  File (ignored)");
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(ignored)", false);
        return {};
    }
    else if (isFolderPrefsFile (file))
    {
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "System, Folder Preferences", false);
        return {};
    }
    else if (isLastFolderFile (file))
    {
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "System, Last Folder", false);
        return {};
    }
    else if (isLastPresetFile(file))
    {
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "System, Last Preset", false);
        return {};
    }
    else if (isPresetFile (file))
    {
        bool isIgnored { false };

        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "Preset", false);

        const auto presetNumber { getPresetNumberFromName (file) };
        if (presetNumber > maxPresets)
        {
            isIgnored = true;
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "(Preset number (" + juce::String (presetNumber) + ") above max of 199. Preset will be ignored)", false);
            validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeRenameFile, file.getFullPathName ());
        }

        if (file.getFileNameWithoutExtension ().startsWith (kPresetFileNamePrefix) == false)
        {
            isIgnored = true;
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "(Preset file must begin with lowercase '" + kPresetFileNamePrefix + "'. Preset will be ignored)", false);
            validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeRenameFile, file.getFullPathName ());
        }

        juce::StringArray fileContents;
        file.readLines (fileContents);

        Assimil8orPreset assimil8orPreset;
        assimil8orPreset.parse (fileContents);

        if (auto presetErrorList { assimil8orPreset.getParseErrorsVT () }; presetErrorList.getNumChildren () > 0)
        {
            ValueTreeHelpers::forEachChildOfType (presetErrorList, "ParseError", [this, &validatorResultProperties] (juce::ValueTree childVT)
            {
                const auto parseErrorType { childVT.getProperty ("type").toString ()};
                const auto parseErrorDescription { childVT.getProperty ("description").toString () };
                validatorResultProperties.update (ValidatorResultProperties::ResultTypeError,
                                                  "[Parse error '" + parseErrorDescription + "']", false);
                return true;
            });
        }
        uint64_t sizeRequiredForSamples {};
        PresetProperties presetProperties (assimil8orPreset.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        if (presetProperties.isValid ())
        {
            presetProperties.forEachChannel ([this, &file, &validatorResultProperties, &sizeRequiredForSamples] (juce::ValueTree channelVT)
            {
                if (threadShouldExit ())
                    return false;
                ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                channelProperties.forEachZone ([this, &file, &validatorResultProperties, &sizeRequiredForSamples] (juce::ValueTree zoneVT)
                {
                    if (threadShouldExit ())
                        return false;
                    ZoneProperties zoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    // TODO - should we do doIfProgressTimeElapsed () here?
                    const auto sampleFileName { zoneProperties.getSample () };
                    const auto sampleFile { file.getParentDirectory ().getChildFile (sampleFileName) };
                    if (!sampleFile.exists ())
                    {
                        // report error
                        validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "['" + sampleFileName + "' does not exist]", false);
                        validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeNotFound, sampleFile.getFullPathName ());
                    }
                    else
                    {
                        // open as audio file, calculate memory requirements
                        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile));
                        if (reader != nullptr)
                        {
                            sizeRequiredForSamples += reader->numChannels * reader->lengthInSamples * bytesPerSampleInAssimMemory;
                        }
                        else
                        {
                            LogValidation ("    [ Warning : unknown audio format ]");
                            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "['" + sampleFileName + "' unknown audio format. size = " + juce::String (file.getSize ()) + "]", false);
                        }
                    }
                    return true;
                });
                return true;
            });
        }
        else
        {
            validatorResultProperties.update ("error", "[missing Preset section]", false);
        }
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "RAM: " + getMemorySizeString (sizeRequiredForSamples), false);
        if (! isIgnored)
            optionalPresetInfo = sizeRequiredForSamples;
        LogValidation ("  File (preset)");
        return { 0, optionalPresetInfo };
    }
    else if (isAudioFile (file))
    {
        validatorResultProperties.updateType (ValidatorResultProperties::ResultTypeInfo, false);
        if (file.getFileName ().length () > kMaxFileNameLength)
        {
            LogValidation ("  [ Warning : file name too long ]");
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError,
                "[name too long. " + juce::String (file.getFileName ().length ()) + "(length) vs " +
                juce::String (kMaxFileNameLength) + "(max)]", false);
            validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeRenameFile, file.getFullPathName ());
        }

        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
        uint64_t sizeRequiredForSamples { 0 };
        if (reader != nullptr)
        {
            LogValidation ("    Format: " + reader->getFormatName ());
            LogValidation ("    Sample data: " + juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer"));
            LogValidation ("    Bit Depth: " + juce::String (reader->bitsPerSample));
            LogValidation ("    Sample Rate: " + juce::String (reader->sampleRate));
            LogValidation ("    Channels: " + juce::String (reader->numChannels));
            LogValidation ("    Length/Samples: " + juce::String (reader->lengthInSamples));
            LogValidation ("    Length/Time: " + juce::String (reader->lengthInSamples / reader->sampleRate));

            const auto memoryUsage { reader->numChannels * reader->lengthInSamples * 4 };
            const auto sampleRateString { juce::String (reader->sampleRate / 1000.0f, 2).trimCharactersAtEnd ("0.") };
            validatorResultProperties.updateText (juce::String (" {") +
                                                 juce::String (reader->usesFloatingPointData == true ? "floating point" : "integer") + ", " +
                                                 juce::String (reader->bitsPerSample) + "bits/" + sampleRateString + "k, " +
                                                 juce::String (reader->numChannels == 1 ? "mono" : (reader->numChannels == 2 ? "stereo" : juce::String (reader->numChannels) + " channels")) + "}, " +
                                                 juce::String (reader->lengthInSamples / reader->sampleRate, 2) + " seconds, " +
                                                 "RAM: " + getMemorySizeString (memoryUsage), false);
            sizeRequiredForSamples = reader->numChannels * reader->lengthInSamples * bytesPerSampleInAssimMemory;
            auto reportErrorIfTrue = [&validatorResultProperties, &file] (bool conditionalResult, juce::String newText)
            {
                if (conditionalResult)
                {
                    validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, newText, false);
                    // TODO - this will create multiple fixer entries for multiple format mismatches. there should only be one
                    //        I think the fix should be in addFixerEntry, as there should never be more than one type
                    validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeConvert, file.getFullPathName ());
                }
            };
            reportErrorIfTrue (reader->usesFloatingPointData == true, "[sample format must be integer]");
            reportErrorIfTrue (reader->bitsPerSample < 8 || reader->bitsPerSample > 32, "[bit depth must be between 8 and 32]");
            if (reader->numChannels == 0)
                validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "[no channels in file]", false);
            else
#define ONLY_MONO_TEST 0
#if ONLY_MONO_TEST
                reportErrorIfTrue (reader->numChannels > 1, "[only mono supported]");
#else
                reportErrorIfTrue (reader->numChannels > 2, "[only mono and stereo supported]");
#endif
            reportErrorIfTrue (reader->sampleRate > 192000, "[sample rate must not exceed 192k]");
        }
        else
        {
            LogValidation ("    [ Warning : unknown audio format ]");
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "[unknown audio format. size = " + juce::String (file.getSize ()) + "]", false);
        }
        return { sizeRequiredForSamples, {} };
    }
    else
    {
        // possibly an audio file of a format not supported on the Assimil8or
        if (auto* format { audioFormatManager.findFormatForFileExtension (file.getFileExtension ()) }; format != nullptr)
        {
            // this is a type we can read, so we can offer to convert it
            LogValidation ("  File (unsupported audio format)");
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(unsupported audio format)", false);
        }
        else
        {
            LogValidation ("  File (unknown)");
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(unknown file type)", false);
        }
    }

    return {};
}

void Assimil8orValidator::validateFolder (juce::File folder, juce::ValueTree validatorResultsVT)
{
    const auto kMaxFolderNameLength { 31 };
    ValidatorResultProperties validatorResultProperties (validatorResultsVT,
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);

    LogValidation ("Folder: " + folder.getFileName ());
    if (folder.getFileName ().startsWithChar ('.'))
    {
        // ignore file
        LogValidation ("  Folder (ignored) : " + folder.getFileName ());
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeWarning, "(ignored)", false);
    }
    else if (folder.getFileName ().length () > kMaxFolderNameLength)
    {
        LogValidation ("  [ Warning : folder name too long ]");
        validatorResultProperties.update (ValidatorResultProperties::ResultTypeError, "[name too long. " +
                                                                                      juce::String (folder.getFileName ().length ()) + "(length) vs " +
                                                                                      juce::String (kMaxFolderNameLength) + "(max)]", false);
        validatorResultProperties.addFixerEntry (FixerEntryProperties::FixerTypeRenameFolder, folder.getFullPathName ());
    }
}

void Assimil8orValidator::processFolder (juce::ValueTree folderVT)
{
    // iterate over file system
    // for directories
    //      report if name over 31 characters
    //      (TODO) report if folder is 2 or more deep in hierarchy
    // for preset files (to be done when the preset file is fully defined)
    //      report if channels * samples * 4 > Assimil8or memory
    //      (TODO) validate preset file data (really only important if it has been edited)
    // for audio files
    //      report if name over 47 characters
    //      report if it does not match supported formats
    // report any other files as unused by assimil8or
    jassert (folderVT.getType ().toString () == "Folder");
    uint64_t totalSizeOfRamRequiredPresets {};
    uint64_t totalSizeOfRamRequiredAllAudioFiles {};
    int numberOfPresets {};
    for (auto folderEntryVT : folderVT)
    {
        if (threadShouldExit ())
            break;

        const auto curEntry { juce::File (folderEntryVT.getProperty ("name")) };
        doIfProgressTimeElapsed ([this, fileName = curEntry.getFileName ()] ()
        {
            juce::MessageManager::callAsync ([this, fileName] ()
            {
                validatorProperties.setProgressUpdate (fileName, false);
            });
        });
        if (curEntry.isDirectory ())
        {
            ValidatorResultProperties validatorResultProperties;
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "Folder: " + curEntry.getFileName (), false);
            validateFolder (curEntry, validatorResultProperties.getValueTree ());
            addResult (validatorResultProperties.getValueTree ());
            processFolder (folderEntryVT);
        }
        else // curEntry is a file
        {
            ValidatorResultProperties validatorResultProperties;
            validatorResultProperties.update (ValidatorResultProperties::ResultTypeInfo, "File: " + curEntry.getFileName (), false);
            validatorResultProperties.getValueTree ().setProperty ("fullFileName", curEntry.getFullPathName (), nullptr);
            const auto [ramRequired, presetUpdate] { validateFile (curEntry, validatorResultProperties.getValueTree ()) };
            totalSizeOfRamRequiredAllAudioFiles += ramRequired;
            if (presetUpdate.has_value ())
            {
                totalSizeOfRamRequiredPresets += presetUpdate.value ();
                ++numberOfPresets;
            }
            addResult (validatorResultProperties.getValueTree ());
        }
    }
    const auto folderName { juce::File (folderVT.getProperty ("name")).getFileName () };
    addResult ("info", "Total RAM Usage for Presets in`" + folderName + "': " + getMemorySizeString (totalSizeOfRamRequiredPresets));
    addResult ("info", "Total RAM Usage for all audio files in `" + folderName + "': " + getMemorySizeString (totalSizeOfRamRequiredAllAudioFiles));
    if (totalSizeOfRamRequiredPresets > maxMemory)
        addResult ("error", "[Preset RAM Usage exceeds memory capacity by " + getMemorySizeString (totalSizeOfRamRequiredPresets - maxMemory) + "]");
    if (numberOfPresets > maxPresets)
        addResult ("error", "[Number of Presets (" + juce::String (numberOfPresets) + ") exceeds maximum allowed (" + juce::String (maxPresets) + ")]");
}

juce::ValueTree Assimil8orValidator::getContentsOfFolder (juce::File folder)
{
    juce::ValueTree folderVT {"Folder"};
    folderVT.setProperty ("name", folder.getFullPathName (), nullptr);
    for (const auto& entry : juce::RangedDirectoryIterator (folder, false, "*", juce::File::findFilesAndDirectories))
    {
        if (threadShouldExit ())
            break;

        doIfProgressTimeElapsed ([this, fileName = entry.getFile ().getFileName ()] ()
        {
            juce::MessageManager::callAsync ([this, fileName] ()
                {
                    validatorProperties.setProgressUpdate (fileName, false);
                });
        });
        if (const auto& curFile { entry.getFile () }; curFile.isDirectory ())
        {
            folderVT.addChild (getContentsOfFolder (curFile), -1, nullptr);
        }
        else
        {
            juce::ValueTree fileVT {"File"};
            fileVT.setProperty ("name", curFile.getFullPathName (), nullptr);
            folderVT.addChild (fileVT, -1, nullptr);
        }
    }
    return folderVT;
}

void Assimil8orValidator::validateRootFolder ()
{
    validatorResultListProperties.clear ();
    lastScanInProgressUpdate = juce::Time::currentTimeMillis ();
    const auto rootFolderName { validatorProperties.getRootFolder () };

    const auto rootEntry { juce::File (rootFolderName) };
    addResult (ValidatorResultProperties::ResultTypeInfo, "Root Folder: " + rootEntry.getFileName ());
    // do one initial progress update to fill in the first one
    juce::MessageManager::callAsync ([this, folderName = rootEntry.getFileName ()] ()
    {
        validatorProperties.setProgressUpdate (folderName, false);
    });

    if (! threadShouldExit ())
        rootFolderVT = getContentsOfFolder (rootFolderName);
    if (! threadShouldExit ())
        sortContentsOfFolder (rootFolderVT);
    if (! threadShouldExit ())
        processFolder (rootFolderVT);
    rootFolderVT = {};

    juce::MessageManager::callAsync ([this] ()
    {
        validatorProperties.setProgressUpdate ("", false);
        validatorProperties.setScanStatus ("idle", false);
    });
}

void Assimil8orValidator::run ()
{
    validateRootFolder ();
}

void Assimil8orValidator::sortContentsOfFolder (juce::ValueTree folderVT)
{
    jassert (folderVT.getType ().toString () == "Folder");

    //   Folders
    //   Preset files
    //   Audio files
    //   unknown files
    struct SectionInfo
    {
        int startIndex { 0 };
        int length { 0 };
    };
    enum SectionIndex
    {
        folders,
        presetFiles,
        audioFiles,
        unknownFiles,
        size
    };
    std::array<SectionInfo, SectionIndex::size> sections;
    const auto numFolderEntries { folderVT.getNumChildren () };

    auto insertSorted = [&sections, &folderVT] (int itemIndex, int sectionIndex)
    {
        jassert (sectionIndex < SectionIndex::size);
        auto& section { sections [sectionIndex] };
        jassert (itemIndex >= section.startIndex + section.length);
        auto startingSectionLength { section.length };
        auto insertItem = [&folderVT, &section, &sections] (int itemIndex, int insertIndex)
        {
            auto tempVT { folderVT.getChild (itemIndex) };
            folderVT.removeChild (itemIndex, nullptr);
            folderVT.addChild (tempVT, insertIndex, nullptr);
            ++section.length;
            for (auto curSectionIndex { 1 }; curSectionIndex < SectionIndex::size; ++curSectionIndex)
                sections [curSectionIndex].startIndex = sections [curSectionIndex - 1].startIndex + sections [curSectionIndex - 1].length;
        };
        for (auto sectionEntryIndex { section.startIndex }; sectionEntryIndex < section.startIndex + section.length; ++sectionEntryIndex)
        {
            if (folderVT.getChild (itemIndex).getProperty ("name").toString () < folderVT.getChild (sectionEntryIndex).getProperty ("name").toString ())
            {
                insertItem (itemIndex, sectionEntryIndex);
                break;
            }
        }
        if (section.length == startingSectionLength)
            insertItem (itemIndex, section.startIndex + section.length);
    };
    for (auto folderIndex { 0 }; folderIndex < numFolderEntries; ++folderIndex)
    {
        auto folderEntryVT { folderVT.getChild (folderIndex) };
        if (folderEntryVT.getType ().toString () == "Folder")
        {
            insertSorted (folderIndex, SectionIndex::folders);
            sortContentsOfFolder (folderEntryVT);
        }
        else if (folderEntryVT.getType ().toString () == "File")
        {
            // alphabetize in these lists
            //   Preset files
            //   Audio files
            //   unknown files
            auto curFile { juce::File (folderEntryVT.getProperty ("name").toString ()) };
            jassert (curFile.exists ());
            if (isPresetFile (curFile))
                insertSorted (folderIndex, SectionIndex::presetFiles);
            else if (isAudioFile (curFile))
                insertSorted (folderIndex, SectionIndex::audioFiles);
            else // unknown file
                insertSorted (folderIndex, SectionIndex::unknownFiles);
        }
        else
        {
            jassertfalse;
        }
    }
}
