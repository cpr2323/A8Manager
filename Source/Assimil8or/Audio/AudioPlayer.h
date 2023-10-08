#pragma once

#include <JuceHeader.h>
#include "AudioPlayerProperties.h"
#include "AudioSettingsProperties.h"
#include "../Preset/PresetProperties.h"
#include "../../AppProperties.h"

class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener
{
public:
    void init (juce::ValueTree rootProperties);
    void shutdownAudio ();

private:
    AudioSettingsProperties audioSettingsProperties;
    AudioPlayerProperties audioPlayerProperties;
    AppProperties appProperties;
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    std::unique_ptr < juce::AudioBuffer<float>> sampleBuffer;
    juce::AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, false, false, true, false};

    int position { 0 };
    int loopStart { 0 };
    int loopEnd { 0 };

    juce::File audioFile;
    double sampleRate { 44100.0 };
    int blockSize { 128 };
    bool playing { false };

    void configureAudioDevice (juce::String deviceName);
    void handlePlayState (AudioPlayerProperties::PlayState playState);
    void showConfigDialog ();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
};