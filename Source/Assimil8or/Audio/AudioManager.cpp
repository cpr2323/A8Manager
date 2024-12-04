#include "AudioManager.h"
#include "../../Utility/DebugLog.h"

constexpr float epsilon = 1e-6f;

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

juce::String AudioManager::getFileTypesList ()
{
    juce::String fileTypesList;
    for (auto fileExtension : audioFileExtensions)
        fileTypesList += juce::String (fileTypesList.length () == 0 ? "" : ";") + "*" + fileExtension;
    return fileTypesList;
}

juce::int64 AudioManager::findNextZeroCrossing (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side)
{
    if (startSampleOffset < 0 || startSampleOffset >= maxSampleOffset - 1)
        return -1; // Invalid start position

    auto readPtr { buffer.getReadPointer (side) };
    for (juce::int64 i = startSampleOffset; i < maxSampleOffset - 1; ++i)
    {
        if ((readPtr [i] > epsilon && readPtr [i + 1] <= epsilon) || (readPtr [i] < epsilon && readPtr [i + 1] >= epsilon))
        {
            return i; // Return the index of the zero crossing
        }
    }
    return -1; // No zero crossing found
}

juce::int64 AudioManager::findPreviousZeroCrossing (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side)
{
    if (startSampleOffset <= minSampleOffset || startSampleOffset >= buffer.getNumSamples ())
    {
        return -1; // Invalid start position
    }

    auto readPtr { buffer.getReadPointer (side) };
    for (juce::int64 i = startSampleOffset; i > minSampleOffset; --i)
    {
        if ((readPtr [i] > epsilon && readPtr [i - 1] <= epsilon) || (readPtr [i] < epsilon && readPtr [i - 1] >= epsilon))
        {
            return i - 1; // Return the index of the zero crossing
        }
    }
    return -1; // No zero crossing found

}
