#include "AudioManager.h"
#include "../../Utility/DebugLog.h"

constexpr float epsilon = 1e-6f;
#define INCLUDE_WAVE_MATCHING_LOOP_POINT_ALIGN 0
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

#if INCLUDE_WAVE_MATCHING_LOOP_POINT_ALIGN
// Function to calculate similarity (e.g., RMSE) between two segments
float AudioManager::calculateSimilarity (const float* buffer, size_t index1, size_t index2, size_t window_size)
{
    float error = 0.0f;
    for (size_t i = 0; i < window_size; ++i)
    {
        float diff = buffer [index1 + i] - buffer [index2 + i];
        error += diff * diff;
    }
    return std::sqrt (error / window_size);
}

// Function to move a marker left or right to match another point
size_t AudioManager::findWaveMatchingOffset (const float* buffer, juce::int64 size, size_t currentOffset, size_t targetOffset, size_t windowSize, SearchDirection searchDirection, size_t maxDistance)
{
    size_t bestOffset = currentOffset;
    float bestError = std::numeric_limits<float>::max ();

    // Define search range
    size_t searchStart = (searchDirection == SearchDirection::left) ? (currentOffset > maxDistance ? currentOffset - maxDistance : 0) : currentOffset;
    size_t searchEnd = (searchDirection == SearchDirection::left) ? currentOffset : std::min (currentOffset + maxDistance, size - windowSize);

    // Search for the best match within the range
    for (size_t candidateOffset = searchStart; candidateOffset < searchEnd; ++candidateOffset)
    {
        float error = calculateSimilarity (buffer, candidateOffset, targetOffset, windowSize);
        if (error < bestError)
        {
            bestError = error;
            bestOffset = candidateOffset;
        }
    }

    return bestOffset;
}

constexpr size_t kWindowSize { 60 };
juce::int64 AudioManager::findNextZeroWaveMatching (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::AudioBuffer<float>& buffer, int side)
{
    return findWaveMatchingOffset (buffer.getReadPointer(side), buffer.getNumSamples(), startSampleOffset, maxSampleOffset, kWindowSize, SearchDirection::right, std::abs(maxSampleOffset - startSampleOffset));
}

juce::int64 AudioManager::findPreviousWaveMatching (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::AudioBuffer<float>& buffer, int side)
{
    return findWaveMatchingOffset (buffer.getReadPointer (side), buffer.getNumSamples (), startSampleOffset, minSampleOffset, kWindowSize, SearchDirection::left, std::abs (startSampleOffset - minSampleOffset));
}
#endif

void AudioManager::mixStereoToMono (juce::File inputFile)
{
    if (auto sampleFileReader { getReaderFor (inputFile) }; sampleFileReader != nullptr)
    {
        if (sampleFileReader->numChannels != 2)
        {
            jassertfalse;
            return;
        }

        // read in stereo data into stereo audio buffer
        juce::AudioBuffer<float> stereoAudioBuffer;
        stereoAudioBuffer.setSize (2, static_cast<int> (sampleFileReader->lengthInSamples), false, true, false);
        sampleFileReader->read (&stereoAudioBuffer, 0, static_cast<int> (sampleFileReader->lengthInSamples), 0, true, true);

        // prepare mono audio buffer
        juce::AudioBuffer<float> monoAudioBuffer;
        monoAudioBuffer.setSize (1, static_cast<int> (sampleFileReader->lengthInSamples), false, true, false);

        // mix to mono
        constexpr float sqrRootOfTwo { 1.41421356237f };
        auto leftChannelReadPtr { stereoAudioBuffer.getReadPointer (0) };
        auto rightChannelReadPtr { stereoAudioBuffer.getReadPointer (1) };
        auto monoWritePtr { monoAudioBuffer.getWritePointer (0) };
        for (uint32_t sampleCounter { 0 }; sampleCounter < sampleFileReader->lengthInSamples; ++sampleCounter)
        {
            *monoWritePtr = (*leftChannelReadPtr + *rightChannelReadPtr) / sqrRootOfTwo;
            ++monoWritePtr;
            ++leftChannelReadPtr;
            ++rightChannelReadPtr;
        }

        // write mono audio buffer to new file
        const auto monoOutputFileName { inputFile.getFileNameWithoutExtension () + "-mono" };
        juce::File monoOuputFile { inputFile.getParentDirectory ().getChildFile (monoOutputFileName).withFileExtension ("wav") };
        auto outputStream { monoOuputFile.createOutputStream () };
        outputStream->setPosition (0);
        outputStream->truncate ();
        juce::WavAudioFormat wavAudioFormat;
        if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (outputStream.get (), sampleFileReader->sampleRate, 1, sampleFileReader->bitsPerSample, {}, 0) }; writer != nullptr)
        {
            // audioFormatWriter will delete the file stream when done
            outputStream.release ();
            writer->writeFromAudioSampleBuffer (monoAudioBuffer, 0, static_cast<int>(sampleFileReader->lengthInSamples));
        }
        else
        {
            // TODO - handle error for not being able to create the writer
            jassertfalse;
        }
    }
    else
    {
        // TODO - handle error for not being able to create the reader
        jassertfalse;
    }
}

void AudioManager::splitStereoIntoTwoMono (juce::File inputFile)
{
    if (auto sampleFileReader { getReaderFor (inputFile) }; sampleFileReader != nullptr)
    {
        if (sampleFileReader->numChannels != 2)
        {
            jassertfalse;
            return;
        }

        // read in stereo data into stereo audio buffer
        juce::AudioBuffer<float> stereoAudioBuffer;
        stereoAudioBuffer.setSize (2, static_cast<int> (sampleFileReader->lengthInSamples), false, true, false);
        sampleFileReader->read (&stereoAudioBuffer, 0, static_cast<int> (sampleFileReader->lengthInSamples), 0, true, true);

        auto readWriteOneChannel = [&inputFile, &sampleFileReader] (const float* monoReadPtr, juce::String postFixChannelIndicator)
        {
            // prepare mono audio buffer
            juce::AudioBuffer<float> monoAudioBuffer;
            monoAudioBuffer.setSize (1, static_cast<int> (sampleFileReader->lengthInSamples), false, true, false);

            auto monoWritePtr { monoAudioBuffer.getWritePointer (0) };
            std::memcpy (monoWritePtr, monoReadPtr, sampleFileReader->lengthInSamples * (sampleFileReader->bitsPerSample / 8));

            // write mono audio buffer to new file
            const auto monoOutputFileName { inputFile.getFileNameWithoutExtension () + "-" + postFixChannelIndicator };
            juce::File monoOuputFile { inputFile.getParentDirectory ().getChildFile (monoOutputFileName).withFileExtension ("wav") };
            auto outputStream { monoOuputFile.createOutputStream () };
            outputStream->setPosition (0);
            outputStream->truncate ();
            juce::WavAudioFormat wavAudioFormat;
            if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (outputStream.get (), sampleFileReader->sampleRate, 1, sampleFileReader->bitsPerSample, {}, 0) }; writer != nullptr)
            {
                // audioFormatWriter will delete the file stream when done
                outputStream.release ();
                writer->writeFromAudioSampleBuffer (monoAudioBuffer, 0, static_cast<int>(sampleFileReader->lengthInSamples));
            }
            else
            {
                // TODO - handle error for not being able to create the writer
                jassertfalse;
            }
        };
        // mix to mono
        readWriteOneChannel (stereoAudioBuffer.getReadPointer (0), "L");
        readWriteOneChannel (stereoAudioBuffer.getReadPointer (1), "R");
    }
    else
    {
        // TODO - handle error for not being able to create the reader
        jassertfalse;
    }

}
