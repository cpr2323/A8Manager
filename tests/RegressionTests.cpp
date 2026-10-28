#include "Assimil8or/Assimil8orPreset.h"
#include "Assimil8or/Audio/AudioManager.h"
#include "Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "Utility/DebugLog.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

// The probes do not need the application's file logger or GUI startup.
void DebugLog (juce::String, juce::String) {}
void FlushDebugLog () {}

namespace
{
    void require (bool condition, const juce::String& message)
    {
        // Unlike jassert, these checks also run in Release builds.
        if (! condition)
            throw std::runtime_error (message.toStdString ());
    }

    void testParserAndCv ()
    {
        Assimil8orPreset parser;
        for (auto repetition { 0 }; repetition < 100; ++repetition)
        {
            parser.parse ({ "UnknownGlobal: 1", "Preset 1:", "UnknownPreset: 2", "Name: ScopeTest",
                            "Channel 1:", "UnknownChannel: 3", "Level: -6", "Zone 1:",
                            "UnknownZone: 4", "Sample: example.wav", "Channel 2:", "Pitch: 2" });
            const auto errors { parser.getParseErrorsVT () };
            require (errors.getNumChildren () == 4, "Expected four unknown-parameter diagnostics");
            for (const auto error : errors)
                require (error.getProperty ("type").toString () == "UnknownParameterError", "Unexpected diagnostic type");

            PresetProperties preset (parser.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
            ChannelProperties first (preset.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            ChannelProperties second (preset.getChannelVT (1), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            ZoneProperties zone (first.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
            require (preset.getName () == "ScopeTest", "Preset scope lost after unknown parameter");
            require (std::abs (first.getLevel () + 6.0) < 1e-9, "Channel scope lost after unknown parameter");
            require (zone.getSample () == "example.wav", "Zone scope lost after unknown parameter");
            require (std::abs (second.getPitch () - 2.0) < 1e-9, "Failed to transition from zone to next channel");

            first.setZonesCV ("CV A", false);
            require (first.getZonesCV () == "0A", "Zones CV normalization failed");
            require (ChannelProperties::getCvInputAndValueString ("CV B", 0.5, 2) == "0B 0.50", "CV amount normalization failed");
        }

        parser.parse ({ "Preset 2:", "Channel 1:", "PitchCV: 1A" });
        require (parser.getParseErrorsVT ().getNumChildren () == 1, "Missing CV delimiter was not reported");
        require (parser.getParseErrorsVT ().getChild (0).getProperty ("type").toString () == "ParameterFormatError", "Expected a CV format diagnostic");
        parser.parse ({ "Preset 3:", "Name: Clean" });
        require (parser.getParseErrorsVT ().getNumChildren () == 0, "Parse errors did not reset");
        PresetProperties cleanPreset (parser.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        require (cleanPreset.getId () == 3 && cleanPreset.getName () == "Clean", "Parsing did not recover after malformed CV");
        std::cout << "PASS: parser/CV (all scopes, 100 repeated parses, malformed CV, normalization)\n";
    }

    struct StereoFixture
    {
        // JUCE generates a unique path in the system temporary directory.
        juce::TemporaryFile input { ".wav" };
        juce::File left { input.getFile ().getSiblingFile (input.getFile ().getFileNameWithoutExtension () + "-L.wav") };
        juce::File right { input.getFile ().getSiblingFile (input.getFile ().getFileNameWithoutExtension () + "-R.wav") };

        ~StereoFixture ()
        {
            // Only generated outputs are removed; TemporaryFile removes the input.
            left.deleteFile ();
            right.deleteFile ();
        }
    };

    void testStereoSplit ()
    {
        AudioManager audio;
        constexpr auto sampleCount { 8193 };
        for (const auto bits : { 16, 24 })
        {
            StereoFixture fixture;
            const auto input { fixture.input.getFile () };
            juce::AudioBuffer<float> source (2, sampleCount);
            for (auto channel { 0 }; channel < 2; ++channel)
                for (auto sample { 0 }; sample < sampleCount; ++sample)
                    source.setSample (channel, sample, 0.7f * std::sin (static_cast<float> (sample + channel * 37) * 0.07f));

            {
                auto stream { input.createOutputStream () };
                require (stream != nullptr, "Failed to open stereo fixture");
                juce::WavAudioFormat format;
                std::unique_ptr<juce::AudioFormatWriter> writer { format.createWriterFor (stream.get (), 48000, 2, bits, {}, 0) };
                // the writer takes ownership of the stream
                if (writer != nullptr)
                    stream.release ();
                require (writer != nullptr && writer->writeFromAudioSampleBuffer (source, 0, sampleCount), "Failed to write stereo fixture");
            }

            // Compare against the quantized WAV data, not the pre-encoding floats.
            auto original { audio.getReaderFor (input) };
            require (original != nullptr && original->read (&source, 0, sampleCount, 0, true, true), "Failed to read stereo fixture");
            audio.splitStereoIntoTwoMono (input);
            for (auto channel { 0 }; channel < 2; ++channel)
            {
                auto reader { audio.getReaderFor (channel == 0 ? fixture.left : fixture.right) };
                require (reader != nullptr, "Mono output missing or unreadable");
                require (reader->numChannels == 1, "Output is not mono");
                require (reader->lengthInSamples == sampleCount, "Mono output length mismatch");
                require (reader->bitsPerSample == static_cast<unsigned int> (bits), "Mono output bit depth mismatch");
                require (std::abs (reader->sampleRate - 48000.0) < 1e-9, "Mono output sample rate mismatch");
                juce::AudioBuffer<float> mono (1, sampleCount);
                require (reader->read (&mono, 0, sampleCount, 0, true, false), "Failed to read mono output");

                // Float decoding/re-encoding may round by one PCM quantization step.
                const auto tolerance { 1.1f / static_cast<float> (1 << (bits - 1)) };
                auto maxError { 0.0f };
                for (auto sample { 0 }; sample < sampleCount; ++sample)
                {
                    const auto actual { mono.getSample (0, sample) };
                    const auto error { std::abs (source.getSample (channel, sample) - actual) };
                    require (std::isfinite (actual) && error <= tolerance,
                             juce::String (bits) + "-bit channel " + juce::String (channel) + ", sample " + juce::String (sample) + ": content mismatch");
                    maxError = std::max (maxError, error);
                }
                std::cout << bits << "-bit channel " << channel << ": max error " << maxError << '\n';
            }
            std::cout << "PASS: " << bits << "-bit stereo split (both channels, all " << sampleCount << " samples, lengths and format)\n";
        }
    }

    // A source folder (somewhere else on the computer) and a preset folder, both removed when the fixture goes away
    struct StagingFixture
    {
        juce::File root { juce::File::getSpecialLocation (juce::File::tempDirectory).getNonexistentChildFile ("A8ManagerStagingTest", {}, false) };
        juce::File sourceFolder { root.getChildFile ("source") };
        juce::File presetFolder { root.getChildFile ("preset") };
        juce::Array<juce::File> removedOriginals;

        StagingFixture ()
        {
            require (sourceFolder.createDirectory () && presetFolder.createDirectory (), "Failed to create staging folders");
        }

        ~StagingFixture ()
        {
            root.deleteRecursively ();
        }

        // stands in for moveToTrash, so the tests do not put files in the real Recycle Bin
        std::function<bool (juce::File)> recordRemoval ()
        {
            return [this] (juce::File file)
            {
                removedOriginals.add (file);
                return file.deleteFile ();
            };
        }

        juce::StringArray presetFolderContents () const
        {
            juce::StringArray names;
            for (const auto& file : presetFolder.findChildFiles (juce::File::findFilesAndDirectories, false))
                names.add (file.getFileName ());
            names.sort (true);
            return names;
        }
    };

    constexpr auto kStagingSampleCount { 4097 };

    juce::AudioBuffer<float> makeSignal (int numChannels, float frequency)
    {
        juce::AudioBuffer<float> signal (numChannels, kStagingSampleCount);
        for (auto channel { 0 }; channel < numChannels; ++channel)
            for (auto sample { 0 }; sample < kStagingSampleCount; ++sample)
                signal.setSample (channel, sample, 0.6f * std::sin (static_cast<float> (sample + channel * 11) * frequency));
        return signal;
    }

    // 32 bit WAV is written as floating point, which the Assimil8or does not support, so it has to be converted
    void writeAudioFile (juce::File file, juce::AudioFormat& format, int bitsPerSample, int numChannels, float frequency)
    {
        auto stream { file.createOutputStream () };
        require (stream != nullptr, "Failed to open fixture '" + file.getFileName () + "'");
        std::unique_ptr<juce::AudioFormatWriter> writer { format.createWriterFor (stream.get (), 48000, static_cast<unsigned int> (numChannels), bitsPerSample, {}, 0) };
        // the writer takes ownership of the stream
        if (writer != nullptr)
            stream.release ();
        const auto signal { makeSignal (numChannels, frequency) };
        require (writer != nullptr && writer->writeFromAudioSampleBuffer (signal, 0, kStagingSampleCount), "Failed to write fixture '" + file.getFileName () + "'");
    }

    void writeWav (juce::File file, int bitsPerSample, int numChannels = 2, float frequency = 0.05f)
    {
        juce::WavAudioFormat format;
        writeAudioFile (file, format, bitsPerSample, numChannels, frequency);
    }

    void writeAiff (juce::File file, int bitsPerSample, int numChannels = 2, float frequency = 0.05f)
    {
        juce::AiffAudioFormat format;
        writeAudioFile (file, format, bitsPerSample, numChannels, frequency);
    }

    juce::MemoryBlock contentsOf (juce::File file)
    {
        juce::MemoryBlock data;
        require (file.loadFileAsData (data), "Unable to read '" + file.getFullPathName () + "'");
        return data;
    }

    juce::AudioBuffer<float> readAudio (AudioManager& audio, juce::File file)
    {
        auto reader { audio.getReaderFor (file) };
        require (reader != nullptr, "'" + file.getFileName () + "' is missing or unreadable");
        juce::AudioBuffer<float> buffer (static_cast<int> (reader->numChannels), static_cast<int> (reader->lengthInSamples));
        require (reader->read (&buffer, 0, buffer.getNumSamples (), 0, true, true), "Failed to read '" + file.getFileName () + "'");
        return buffer;
    }

    void requireSameAudio (const juce::AudioBuffer<float>& expected, const juce::AudioBuffer<float>& actual, int bitsPerSample, const juce::String& context)
    {
        require (expected.getNumChannels () == actual.getNumChannels (), context + ": channel count changed");
        require (expected.getNumSamples () == actual.getNumSamples (), context + ": length changed");
        const auto tolerance { 1.1f / static_cast<float> (1 << (bitsPerSample - 1)) };
        for (auto channel { 0 }; channel < expected.getNumChannels (); ++channel)
            for (auto sample { 0 }; sample < expected.getNumSamples (); ++sample)
                require (std::abs (expected.getSample (channel, sample) - actual.getSample (channel, sample)) <= tolerance,
                         context + ": audio differs at channel " + juce::String (channel) + ", sample " + juce::String (sample));
    }

    void requireAssimil8orWav (AudioManager& audio, juce::File file, int expectedBitsPerSample, const juce::String& context)
    {
        require (file.getFileExtension () == ".wav", context + ": result is not a .wav file");
        require (audio.isAssimil8orSupportedAudioFile (file), context + ": result is not usable by the Assimil8or");
        auto reader { audio.getReaderFor (file) };
        require (reader->bitsPerSample == static_cast<unsigned int> (expectedBitsPerSample), context + ": unexpected bit depth");
    }

    void requireStaged (const AudioManager::StagedSample& staged, juce::File expectedFile, const juce::String& context)
    {
        require (staged.file != juce::File (), context + ": staging failed (" + staged.errorMessage + ")");
        require (staged.file == expectedFile, context + ": staged '" + staged.file.getFullPathName () + "' instead of '" + expectedFile.getFullPathName () + "'");
    }

    void requireRefused (const AudioManager::StagedSample& staged, const juce::String& context)
    {
        require (staged.file == juce::File (), context + ": expected staging to be refused, but it returned '" + staged.file.getFullPathName () + "'");
        require (staged.errorMessage.isNotEmpty (), context + ": refused without an error message");
    }

    // dropping a file from elsewhere must never touch the original (reported bug: originals were deleted)
    void testStagingLeavesOutsideSourcesAlone ()
    {
        AudioManager audio;
        struct Case { juce::String name; std::function<void (juce::File)> write; int expectedBits; };
        const std::vector<Case> cases {
            { "drum.aif", [] (juce::File f) { writeAiff (f, 16); }, 16 },
            { "pad.wav",  [] (juce::File f) { writeWav (f, 32); },  24 },  // float wav needs conversion
            { "kick.wav", [] (juce::File f) { writeWav (f, 16); },  16 },  // already usable, plain copy
        };
        for (const auto& testCase : cases)
        {
            StagingFixture fixture;
            const auto source { fixture.sourceFolder.getChildFile (testCase.name) };
            testCase.write (source);
            const auto originalBytes { contentsOf (source) };
            const auto originalAudio { readAudio (audio, source) };

            const auto staged { audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()) };

            const auto context { "outside " + testCase.name };
            require (source.existsAsFile (), context + ": the original source file was deleted");
            require (contentsOf (source) == originalBytes, context + ": the original source file was modified");
            require (fixture.removedOriginals.isEmpty (), context + ": tried to remove a file outside the preset folder");
            const auto expected { fixture.presetFolder.getChildFile (source.getFileNameWithoutExtension () + ".wav") };
            requireStaged (staged, expected, context);
            requireAssimil8orWav (audio, expected, testCase.expectedBits, context);
            requireSameAudio (originalAudio, readAudio (audio, expected), testCase.expectedBits, context);
            require (fixture.presetFolderContents () == juce::StringArray { expected.getFileName () }, context + ": unexpected files left in the preset folder: " + fixture.presetFolderContents ().joinIntoString (", "));
        }
        std::cout << "PASS: staging from outside the preset folder never modifies or removes the source\n";
    }

    // reported bug: a file in the preset folder that needed converting in place was truncated to nothing
    void testStagingConvertsInPlace ()
    {
        AudioManager audio;
        StagingFixture fixture;
        const auto source { fixture.presetFolder.getChildFile ("pad.wav") };
        writeWav (source, 32);
        const auto originalAudio { readAudio (audio, source) };

        const auto staged { audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()) };

        requireStaged (staged, source, "in place float wav");
        requireAssimil8orWav (audio, source, 24, "in place float wav");
        requireSameAudio (originalAudio, readAudio (audio, source), 24, "in place float wav");
        require (fixture.removedOriginals.isEmpty (), "in place float wav: removeOriginal should not be used when the name does not change");
        require (fixture.presetFolderContents () == juce::StringArray { "pad.wav" }, "in place float wav: unexpected files left in the preset folder: " + fixture.presetFolderContents ().joinIntoString (", "));
        std::cout << "PASS: a float wav in the preset folder is converted in place without losing its audio\n";
    }

    void testStagingConvertsRenamedFileInPresetFolder ()
    {
        AudioManager audio;
        StagingFixture fixture;
        const auto source { fixture.presetFolder.getChildFile ("drum.aif") };
        writeAiff (source, 24);
        const auto originalAudio { readAudio (audio, source) };

        const auto staged { audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()) };

        const auto expected { fixture.presetFolder.getChildFile ("drum.wav") };
        requireStaged (staged, expected, "preset folder aif");
        requireAssimil8orWav (audio, expected, 24, "preset folder aif");
        requireSameAudio (originalAudio, readAudio (audio, expected), 24, "preset folder aif");
        require (fixture.removedOriginals.size () == 1 && fixture.removedOriginals [0] == source, "preset folder aif: the original should be removed through removeOriginal exactly once");
        require (fixture.presetFolderContents () == juce::StringArray { "drum.wav" }, "preset folder aif: unexpected files left in the preset folder: " + fixture.presetFolderContents ().joinIntoString (", "));
        std::cout << "PASS: an aif in the preset folder is converted to wav, and the original goes through removeOriginal\n";
    }

    // a different file with the same name in the preset folder must never be replaced
    void testStagingRefusesToOverwrite ()
    {
        AudioManager audio;
        struct Case { juce::String description; bool sourceInPresetFolder; juce::String name; std::function<void (juce::File)> write; };
        const std::vector<Case> cases {
            { "copy from outside",        false, "drum.wav", [] (juce::File f) { writeWav (f, 16, 2, 0.11f); } },
            { "convert from outside",     false, "drum.aif", [] (juce::File f) { writeAiff (f, 16, 2, 0.11f); } },
            { "convert in preset folder", true,  "drum.aif", [] (juce::File f) { writeAiff (f, 16, 2, 0.11f); } },
        };
        for (const auto& testCase : cases)
        {
            StagingFixture fixture;
            const auto existing { fixture.presetFolder.getChildFile ("drum.wav") };
            writeWav (existing, 16, 1, 0.02f);
            const auto existingBytes { contentsOf (existing) };
            const auto source { (testCase.sourceInPresetFolder ? fixture.presetFolder : fixture.sourceFolder).getChildFile (testCase.name) };
            testCase.write (source);
            const auto sourceBytes { contentsOf (source) };

            const auto staged { audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()) };

            const auto context { "name collision, " + testCase.description };
            require (existing.existsAsFile () && contentsOf (existing) == existingBytes, context + ": the existing file in the preset folder was overwritten");
            require (source.existsAsFile () && contentsOf (source) == sourceBytes, context + ": the source file was modified or removed");
            require (fixture.removedOriginals.isEmpty (), context + ": tried to remove a file");
            requireRefused (staged, context);
            const auto expectedContents { testCase.sourceInPresetFolder ? juce::StringArray { "drum.aif", "drum.wav" } : juce::StringArray { "drum.wav" } };
            require (fixture.presetFolderContents () == expectedContents, context + ": unexpected files left in the preset folder: " + fixture.presetFolderContents ().joinIntoString (", "));
        }
        std::cout << "PASS: staging refuses to replace a different file of the same name\n";
    }

    // dropping the same sample again (ie. onto another zone) is not a collision
    void testStagingReusesIdenticalFile ()
    {
        AudioManager audio;
        for (const auto name : { "kick.wav", "drum.aif" })
        {
            StagingFixture fixture;
            const auto source { fixture.sourceFolder.getChildFile (name) };
            if (source.hasFileExtension ("wav"))
                writeWav (source, 16);
            else
                writeAiff (source, 16);
            const auto expected { fixture.presetFolder.getChildFile (source.getFileNameWithoutExtension () + ".wav") };

            const auto context { juce::String ("drop twice ") + name };
            requireStaged (audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()), expected, context + " (first)");
            const auto firstBytes { contentsOf (expected) };
            requireStaged (audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()), expected, context + " (second)");
            require (contentsOf (expected) == firstBytes, context + ": the staged file changed");
            require (fixture.presetFolderContents () == juce::StringArray { expected.getFileName () }, context + ": unexpected files left in the preset folder: " + fixture.presetFolderContents ().joinIntoString (", "));
        }
        std::cout << "PASS: staging the same sample twice reuses the identical file\n";
    }

    void testStagingReportsUnreadableFiles ()
    {
        AudioManager audio;
        StagingFixture fixture;
        const auto source { fixture.sourceFolder.getChildFile ("broken.aif") };
        require (source.replaceWithText ("this is not audio"), "Failed to write broken fixture");

        const auto staged { audio.stageSampleForPreset (source, fixture.presetFolder, fixture.recordRemoval ()) };

        requireRefused (staged, "unreadable file");
        require (source.loadFileAsString () == "this is not audio", "unreadable file: the source was modified");
        require (fixture.presetFolderContents ().isEmpty (), "unreadable file: files left in the preset folder: " + fixture.presetFolderContents ().joinIntoString (", "));
        std::cout << "PASS: an unreadable file is reported, and nothing is left behind\n";
    }

   #if JUCE_WINDOWS
    // file names are not case sensitive on Windows, so a preset folder path spelled with different case is still the preset folder
    void testStagingPresetFolderSpelling ()
    {
        AudioManager audio;
        StagingFixture fixture;
        const auto source { fixture.presetFolder.getChildFile ("drum.aif") };
        writeAiff (source, 16);
        const juce::File respelledPresetFolder { fixture.presetFolder.getFullPathName ().toUpperCase () };

        const auto staged { audio.stageSampleForPreset (source, respelledPresetFolder, fixture.recordRemoval ()) };

        requireStaged (staged, fixture.presetFolder.getChildFile ("drum.wav"), "preset folder spelling");
        require (fixture.removedOriginals.size () == 1, "preset folder spelling: the source was not recognised as being in the preset folder");
        std::cout << "PASS: the preset folder is recognised regardless of the case of its path\n";
    }
   #endif

    void testSampleStaging ()
    {
        // run every case, so one run shows all of the failures
        const std::vector<std::pair<const char*, std::function<void ()>>> tests {
            { "outside sources", testStagingLeavesOutsideSourcesAlone },
            { "convert in place", testStagingConvertsInPlace },
            { "convert renamed", testStagingConvertsRenamedFileInPresetFolder },
            { "overwrite", testStagingRefusesToOverwrite },
            { "reuse identical", testStagingReusesIdenticalFile },
            { "unreadable", testStagingReportsUnreadableFiles },
           #if JUCE_WINDOWS
            { "folder spelling", testStagingPresetFolderSpelling },
           #endif
        };
        auto failures { 0 };
        for (const auto& [name, test] : tests)
        {
            try
            {
                test ();
            }
            catch (const std::exception& error)
            {
                std::cout << "FAIL (" << name << "): " << error.what () << '\n';
                ++failures;
            }
        }
        require (failures == 0, juce::String (failures) + " sample staging test(s) failed");
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialise;
    auto result { 0 };
    try
    {
        require (argc == 2, "Usage: A8ManagerRegressionTests --parser-cv | --stereo-split | --sample-staging");
        const juce::String selection { argv[1] };
        if (selection == "--parser-cv")
            testParserAndCv ();
        else if (selection == "--stereo-split")
            testStereoSplit ();
        else if (selection == "--sample-staging")
            testSampleStaging ();
        else
            require (false, "Unknown test selection: " + selection);
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what () << '\n';
        result = 1;
    }
    ParameterPresetsSingleton::deleteInstance ();
    return result;
}
