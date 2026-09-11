#include "Assimil8or/Assimil8orPreset.h"
#include "Assimil8or/Audio/AudioManager.h"
#include "Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "oolib/Debug/DebugLog.h"
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
                std::unique_ptr<juce::OutputStream> stream { input.createOutputStream () };
                require (stream != nullptr, "Failed to open stereo fixture");
                juce::WavAudioFormat format;
                auto writer { format.createWriterFor (stream, juce::AudioFormatWriterOptions {}.withSampleRate (48000)
                                                                                                .withNumChannels (2)
                                                                                                .withBitsPerSample (bits)) };
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
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialise;
    auto result { 0 };
    try
    {
        require (argc == 2, "Usage: A8ManagerRegressionTests --parser-cv | --stereo-split");
        const juce::String selection { argv[1] };
        if (selection == "--parser-cv")
            testParserAndCv ();
        else if (selection == "--stereo-split")
            testStereoSplit ();
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
