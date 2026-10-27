#include "AudioManager.h"
#include "oolib/Debug/DebugLog.h"

constexpr float epsilon { 1e-6f };
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

AudioManager::StagedSample AudioManager::stageSampleForPreset (juce::File sourceFile, juce::File presetFolder, std::function<bool (juce::File)> removeOriginal)
{
    // juce::File comparisons follow the platform's case sensitivity rules, which a plain string comparison of the paths does not
    const auto sourceIsInPresetFolder { sourceFile.getParentDirectory () == presetFolder };
    auto collisionError = [] (juce::File existingFile)
    {
        return "A different file named '" + existingFile.getFileName () + "' is already in the preset folder. Rename one of them and try again.";
    };

    if (isAssimil8orSupportedAudioFile (sourceFile))
    {
        if (sourceIsInPresetFolder)
            return { sourceFile, {} };

        const auto destinationFile { presetFolder.getChildFile (sourceFile.getFileName ()) };
        if (destinationFile.exists ())
        {
            // the same sample being assigned again
            if (destinationFile.hasIdenticalContentTo (sourceFile))
                return { destinationFile, {} };
            return { {}, collisionError (destinationFile) };
        }

        // copy through a temporary file, so a failed copy does not leave a partial sample in the preset folder
        juce::TemporaryFile temporaryFile (destinationFile);
        if (! sourceFile.copyFileTo (temporaryFile.getFile ()) || ! temporaryFile.overwriteTargetFileWithTemporary ())
            return { {}, "Unable to copy '" + sourceFile.getFileName () + "' to the preset folder." };
        return { destinationFile, {} };
    }

    // the file needs to be converted to a format the Assimil8or supports
    const auto destinationFile { presetFolder.getChildFile (sourceFile.getFileNameWithoutExtension ()).withFileExtension ("wav") };
    // ie. a floating point .wav in the preset folder, which is replaced by the converted version
    const auto replacesSource { destinationFile == sourceFile };

    // the conversion is written to a temporary file next to the destination, and only moved into place once it has succeeded,
    // so neither the source, nor an existing file, is touched if something goes wrong
    juce::TemporaryFile temporaryFile (destinationFile);
    {
        auto reader { getReaderFor (sourceFile) };
        if (reader == nullptr)
            return { {}, "Unable to read '" + sourceFile.getFileName () + "'." };
        if (reader->numChannels == 0)
            return { {}, "'" + sourceFile.getFileName () + "' contains no audio channels." };
        if (reader->sampleRate > 192000)
            return { {}, "The sample rate of '" + sourceFile.getFileName () + "' exceeds 192 kHz." };

        std::unique_ptr<juce::OutputStream> outputStream { temporaryFile.getFile ().createOutputStream () };
        if (outputStream == nullptr)
            return { {}, "Unable to create a temporary file for '" + destinationFile.getFileName () + "'." };

        const auto bitsPerSample { juce::jlimit (8, 24, static_cast<int> (reader->bitsPerSample)) }; // the wave writer supports int 8/16/24
        const auto numChannels { juce::jmin (static_cast<int> (reader->numChannels), 2) };
        juce::WavAudioFormat wavAudioFormat;
        // on success, the writer takes ownership of the output stream, and will delete it when done
        auto writer { wavAudioFormat.createWriterFor (outputStream, juce::AudioFormatWriterOptions {}.withSampleRate (reader->sampleRate)
                                                                                                     .withNumChannels (numChannels)
                                                                                                     .withBitsPerSample (bitsPerSample)) };
        if (writer == nullptr)
            return { {}, "Unable to create a writer for '" + destinationFile.getFileName () + "'." };
        // TODO - this should be done in a thread, reading into a buffer, so progress can be displayed
        if (! writer->writeFromAudioReader (*reader.get (), 0, -1))
            return { {}, "Unable to convert '" + sourceFile.getFileName () + "'." };
        // the reader and writer go out of scope here, closing both files before they are moved
    }

    if (destinationFile.exists () && ! replacesSource)
    {
        // the same sample being assigned again
        if (destinationFile.hasIdenticalContentTo (temporaryFile.getFile ()))
            return { destinationFile, {} };
        return { {}, collisionError (destinationFile) };
    }

    if (! temporaryFile.overwriteTargetFileWithTemporary ())
        return { {}, "Unable to move the converted '" + destinationFile.getFileName () + "' into the preset folder." };

    if (replacesSource)
        return { sourceFile, {} };

    // a file that was converted to a new name in the preset folder is replaced by the new file. files from elsewhere are never removed
    if (sourceIsInPresetFolder && ! removeOriginal (sourceFile))
        return { destinationFile, "'" + sourceFile.getFileName () + "' was converted to '" + destinationFile.getFileName () + "', but the original could not be removed." };

    return { destinationFile, {} };
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
    for (juce::int64 i { startSampleOffset + 1 }; i < maxSampleOffset - 1; ++i)
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
    if (startSampleOffset <= minSampleOffset || startSampleOffset > buffer.getNumSamples ())
        return -1; // Invalid start position

    auto readPtr { buffer.getReadPointer (side) };
    for (juce::int64 i { startSampleOffset - 1 }; i > minSampleOffset; --i)
    {
        if ((readPtr [i] > epsilon && readPtr [i - 1] <= epsilon) || (readPtr [i] < epsilon && readPtr [i - 1] >= epsilon))
        {
            return i - 1; // Return the index of the zero crossing
        }
    }
    return -1; // No zero crossing found

}

// From startSampleOffset, either way, to the next place its sample value matches the value at matchSampleOffset -
// where the waveform crosses that level, landing on whichever of the two samples either side is the closer match.
// With matchMovesWithStart, the match point is carried along with the search (a loop start moving its end with it),
// so each sample is matched against where the match point would be carried to rather than where it is now.
juce::int64 AudioManager::findMatchingLevel (juce::int64 startSampleOffset, juce::int64 limitSampleOffset, juce::int64 matchSampleOffset, bool matchMovesWithStart,
                                             juce::AudioBuffer<float>& buffer, int side, bool searchRight)
{
    const auto numSamples { static_cast<juce::int64> (buffer.getNumSamples ()) };
    if (numSamples == 0)
        return -1;

    const auto readPtr { buffer.getReadPointer (side) };
    auto valueAt = [readPtr, numSamples] (juce::int64 sample) { return readPtr [std::clamp (sample, juce::int64 { 0 }, numSamples - 1)]; };
    auto difference = [&] (juce::int64 sample)
    {
        return valueAt (sample) - valueAt (matchMovesWithStart ? matchSampleOffset + (sample - startSampleOffset) : matchSampleOffset);
    };

    const auto step { searchRight ? juce::int64 { 1 } : juce::int64 { -1 } };
    const auto limit { std::clamp (limitSampleOffset, juce::int64 { 0 }, numSamples - 1) };
    // starting a sample away from the start, so that it moves on from a match it is already sitting on
    for (auto sample { startSampleOffset + step }; searchRight ? sample + step <= limit : sample + step >= limit; sample += step)
    {
        const auto thisDifference { difference (sample) };
        const auto nextDifference { difference (sample + step) };
        if (thisDifference == 0.0f)
            return sample;
        if ((thisDifference < 0.0f) != (nextDifference < 0.0f) || nextDifference == 0.0f)
            return std::abs (thisDifference) <= std::abs (nextDifference) ? sample : sample + step;
    }
    return -1; // No match found
}

juce::int64 AudioManager::findNextMatchingLevel (juce::int64 startSampleOffset, juce::int64 maxSampleOffset, juce::int64 matchSampleOffset, bool matchMovesWithStart,
                                                 juce::AudioBuffer<float>& buffer, int side)
{
    return findMatchingLevel (startSampleOffset, maxSampleOffset, matchSampleOffset, matchMovesWithStart, buffer, side, true);
}

juce::int64 AudioManager::findPreviousMatchingLevel (juce::int64 startSampleOffset, juce::int64 minSampleOffset, juce::int64 matchSampleOffset, bool matchMovesWithStart,
                                                     juce::AudioBuffer<float>& buffer, int side)
{
    return findMatchingLevel (startSampleOffset, minSampleOffset, matchSampleOffset, matchMovesWithStart, buffer, side, false);
}

#if INCLUDE_WAVE_MATCHING_LOOP_POINT_ALIGN
// Function to calculate similarity (e.g., RMSE) between two segments
float AudioManager::calculateSimilarity (const float* buffer, size_t index1, size_t index2, size_t window_size)
{
    float error { 0.0f };
    for (size_t i { 0 }; i < window_size; ++i)
    {
        float diff { buffer [index1 + i] - buffer [index2 + i] };
        error += diff * diff;
    }
    return std::sqrt (error / window_size);
}

// Function to move a marker left or right to match another point
size_t AudioManager::findWaveMatchingOffset (const float* buffer, juce::int64 size, size_t currentOffset, size_t targetOffset, size_t windowSize, SearchDirection searchDirection, size_t maxDistance)
{
    size_t bestOffset { currentOffset };
    float bestError { std::numeric_limits<float>::max () };

    // Define search range
    size_t searchStart { (searchDirection == SearchDirection::left) ? (currentOffset > maxDistance ? currentOffset - maxDistance : 0) : currentOffset };
    size_t searchEnd { (searchDirection == SearchDirection::left) ? currentOffset : std::min (currentOffset + maxDistance, size - windowSize) };

    // Search for the best match within the range
    for (size_t candidateOffset { searchStart }; candidateOffset < searchEnd; ++candidateOffset)
    {
        float error { calculateSimilarity (buffer, candidateOffset, targetOffset, windowSize) };
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
    return findWaveMatchingOffset (buffer.getReadPointer (side), buffer.getNumSamples (), startSampleOffset, maxSampleOffset, kWindowSize, SearchDirection::right, std::abs (maxSampleOffset - startSampleOffset));
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
        // setPosition () and truncate () are FileOutputStream only, so the setup has to happen while the pointer still has that type
        auto outputFileStream { monoOuputFile.createOutputStream () };
        outputFileStream->setPosition (0);
        outputFileStream->truncate ();
        // createWriterFor takes a unique_ptr<OutputStream>&, and a unique_ptr<FileOutputStream> cannot bind to a reference to a
        // different type, so the stream is moved into a base typed pointer to hand over. this is the same stream: outputFileStream is null from here on
        std::unique_ptr<juce::OutputStream> outputStream { std::move (outputFileStream) };
        juce::WavAudioFormat wavAudioFormat;
        // on success, the writer takes ownership of the output stream, and will delete it when done
        if (auto writer { wavAudioFormat.createWriterFor (outputStream, juce::AudioFormatWriterOptions {}.withSampleRate (sampleFileReader->sampleRate)
                                                                                                        .withNumChannels (1)
                                                                                                        .withBitsPerSample (sampleFileReader->bitsPerSample)) }; writer != nullptr)
        {
            writer->writeFromAudioSampleBuffer (monoAudioBuffer, 0, static_cast<int> (sampleFileReader->lengthInSamples));
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

            monoAudioBuffer.copyFrom (0, 0, monoReadPtr, static_cast<int> (sampleFileReader->lengthInSamples));

            // write mono audio buffer to new file
            const auto monoOutputFileName { inputFile.getFileNameWithoutExtension () + "-" + postFixChannelIndicator };
            juce::File monoOuputFile { inputFile.getParentDirectory ().getChildFile (monoOutputFileName).withFileExtension ("wav") };
            // setPosition () and truncate () are FileOutputStream only, so the setup has to happen while the pointer still has that type
            auto outputFileStream { monoOuputFile.createOutputStream () };
            outputFileStream->setPosition (0);
            outputFileStream->truncate ();
            // createWriterFor takes a unique_ptr<OutputStream>&, and a unique_ptr<FileOutputStream> cannot bind to a reference to a
            // different type, so the stream is moved into a base typed pointer to hand over. this is the same stream: outputFileStream is null from here on
            std::unique_ptr<juce::OutputStream> outputStream { std::move (outputFileStream) };
            juce::WavAudioFormat wavAudioFormat;
            // on success, the writer takes ownership of the output stream, and will delete it when done
            if (auto writer { wavAudioFormat.createWriterFor (outputStream, juce::AudioFormatWriterOptions {}.withSampleRate (sampleFileReader->sampleRate)
                                                                                                            .withNumChannels (1)
                                                                                                            .withBitsPerSample (sampleFileReader->bitsPerSample)) }; writer != nullptr)
            {
                writer->writeFromAudioSampleBuffer (monoAudioBuffer, 0, static_cast<int> (sampleFileReader->lengthInSamples));
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
