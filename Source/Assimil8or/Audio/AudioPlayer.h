#pragma once

#include <JuceHeader.h>
#include "AudioConfigProperties.h"

class AudioPlayer : public juce::AudioSource
{
public:
    void init (juce::ValueTree rootProperties);
    void shutdownAudio ();

private:
    AudioConfigProperties audioConfigProperties;
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    double sampleRate { 44100.0 };
    int blockSize { 128 };

    void configureAudioDevice (juce::String deviceName);

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;
};