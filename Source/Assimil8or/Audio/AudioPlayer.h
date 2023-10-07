#pragma once

#include <JuceHeader.h>
#include "AudioConfigProperties.h"
#include "../../AppProperties.h"
#include "../Preset/PresetProperties.h"

class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener
{
public:
    void init (juce::ValueTree rootProperties);
    void shutdownAudio ();

private:
    AudioConfigProperties audioConfigProperties;
    AppProperties appProperties;
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    std::unique_ptr <juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource;
    juce::AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, false, false, true, false};

    juce::File audioFile;
    double sampleRate { 44100.0 };
    int blockSize { 128 };
    bool playing { false };

    void configureAudioDevice (juce::String deviceName);
    void handlePlayState (AudioConfigProperties::PlayState playState);
    void showConfigDialog ();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;
    void changeListenerCallback (juce::ChangeBroadcaster* source);
};