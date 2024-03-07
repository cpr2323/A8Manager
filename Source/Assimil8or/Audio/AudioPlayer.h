#pragma once

#include <JuceHeader.h>
#include "AudioPlayerProperties.h"
#include "AudioSettingsProperties.h"
#include "../Preset/ChannelProperties.h"
#include "../Preset/PresetProperties.h"
#include "../Preset/ZoneProperties.h"
#include "../../AppProperties.h"
#include "../../Assimil8or/Assimil8orPreset.h"
#include "../../GUI/Assimil8or/Editor/SampleManager/SampleManagerProperties.h"
#include "../../GUI/Assimil8or/Editor/SampleManager/SampleProperties.h"

class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener
{
public:
    AudioPlayer ();

    void init (juce::ValueTree rootProperties);
    void shutdownAudio ();

private:
    AudioSettingsProperties audioSettingsProperties;
    AudioPlayerProperties audioPlayerProperties;
    AppProperties appProperties;
    PresetProperties presetProperties;
    SampleManagerProperties sampleManagerProperties;
    ChannelProperties channelProperties;
    ZoneProperties zoneProperties;
    SampleProperties sampleProperties;
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioFormatManager audioFormatManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    std::unique_ptr < juce::AudioBuffer<float>> sampleBuffer;
    juce::AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, false, false, true, false};

    juce::CriticalSection dataCS;
    AudioPlayerProperties::PlayState playState { AudioPlayerProperties::PlayState::stop };
    int curSampleOffset { 0 };
    int sampleStart { 0 };
    int sampleLength { 0 };

//    juce::File audioFile;
    double sampleRate { 44100.0 };
    int blockSize { 128 };
    double sampleRateRatio { 0.0 };

    void configureAudioDevice (juce::String deviceName);
    void handlePlayState (AudioPlayerProperties::PlayState playState);
    void initFromZone (std::tuple<int, int> channelAndZoneIndecies);
    void initSamplePoints ();
    void prepareSampleForPlayback ();
    void showConfigDialog ();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
};