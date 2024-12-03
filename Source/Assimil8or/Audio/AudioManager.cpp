#include "AudioManager.h"
#include "../../Utility/DebugLog.h"

AudioManager::AudioManager ()
{
    audioFormatManager.registerBasicFormats ();
    for (auto formatIndex { 0 }; formatIndex < audioFormatManager.getNumKnownFormats (); ++formatIndex)
    {
        const auto* format { audioFormatManager.getKnownFormat (formatIndex) };
        DebugLog ("Assimil8orValidator", "Format Name: " + format->getFormatName ());
        DebugLog ("Assimil8orValidator", "Format Extensions: " + format->getFileExtensions ().joinIntoString (", "));
        audioFileExtensions.addArray (format->getFileExtensions ());
    }
}

void AudioManager::init (juce::ValueTree rootPropertiesVT)
{
}

bool AudioManager::isAssimil8orSupportedAudioFile (const juce::File file)
{
    if (file.getFileExtension ().toLowerCase () != ".wav")
        return false;
    if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file)); reader != nullptr)
    {
        return reader->usesFloatingPointData == false &&
               (reader->bitsPerSample >= 8 && reader->bitsPerSample <= 32) &&
               (reader->numChannels >= 1 && reader->numChannels <= 2) &&
               reader->sampleRate <= 192000;
    }
    return false;
}

bool AudioManager::isA8ManagerSupportedAudioFile (const juce::File file)
{
    return audioFileExtensions.contains (file.getFileExtension (), true);
}

std::unique_ptr<juce::AudioFormatReader> AudioManager::getReaderFor (const juce::File file)
{
    return std::unique_ptr<juce::AudioFormatReader> (audioFormatManager.createReaderFor (file));
}
