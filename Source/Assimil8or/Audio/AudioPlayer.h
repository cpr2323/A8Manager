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
    ChannelProperties nextChannelProperties;
    ZoneProperties nextZoneProperties;
    SampleProperties nextSampleProperties;

    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    std::unique_ptr < juce::AudioBuffer<float>> sampleBuffer;
    juce::AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, false, false, true, false};

    juce::CriticalSection dataCS;
    AudioPlayerProperties::PlayState playState { AudioPlayerProperties::PlayState::stop };
    int curSampleOffset { 0 };
    int sampleStart { 0 };
    int sampleLength { 0 };

    double sampleRate { 44100.0 };
    int blockSize { 128 };
    double sampleRateRatio { 0.0 };

    class LeftRightCombinerAudioSource : public juce::AudioSource
    {
    public:
        LeftRightCombinerAudioSource (AudioSource* leftInputSource, int leftInputIndex, AudioSource* rightInputSource, int rightInputIndex, const bool deleteInputWhenDeleted)
            : leftInput { leftInputSource, deleteInputWhenDeleted },
              rightInput { rightInputSource, deleteInputWhenDeleted },
              leftChannelIndex { leftInputIndex },
              rightChannelIndex { rightInputIndex }
        {
            jassert (leftInput != nullptr);
            jassert (rightInput != nullptr);
        }

        void prepareToPlay (int, double) {}
        void releaseResources () {}
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
        {
            jassert (bufferToFill.buffer->getNumChannels () == 2);
            tempBuffer.setSize (2, bufferToFill.buffer->getNumSamples ());
            juce::AudioSourceChannelInfo tempBufferChannelInfo (&tempBuffer, 0, bufferToFill.numSamples);

            leftInput->getNextAudioBlock (tempBufferChannelInfo);
            bufferToFill.buffer->clear ();
            bufferToFill.buffer->copyFrom (0, bufferToFill.startSample, tempBufferChannelInfo.buffer->getReadPointer (leftChannelIndex, 0),
                                           juce::jmin(tempBufferChannelInfo.buffer->getNumSamples(), bufferToFill.numSamples));
            rightInput->getNextAudioBlock (tempBufferChannelInfo);
            bufferToFill.buffer->copyFrom (1, bufferToFill.startSample, tempBufferChannelInfo.buffer->getReadPointer (rightChannelIndex, 0),
                                           juce::jmin (tempBufferChannelInfo.buffer->getNumSamples (), bufferToFill.numSamples));
        }
    private:
        juce::AudioBuffer<float> tempBuffer;
        juce::OptionalScopedPointer<AudioSource> leftInput;
        juce::OptionalScopedPointer<AudioSource> rightInput;
        int leftChannelIndex { 0 };
        int rightChannelIndex { 0 };
    };

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