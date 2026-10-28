#pragma once

#include <JuceHeader.h>
#include "AudioPlayerProperties.h"
#include "AudioSettingsProperties.h"
#include "AuditionPlayhead.h"
#include "../Preset/ChannelProperties.h"
#include "../Preset/PresetProperties.h"
#include "../Preset/ZoneProperties.h"
#include "../../AppProperties.h"
#include "../../Assimil8or/Assimil8orPreset.h"
#include "../../GUI/Assimil8or/Editor/SampleManager/SampleManagerProperties.h"
#include "../../GUI/Assimil8or/Editor/SampleManager/SampleProperties.h"

/*
    The audition voice. It moves its playhead through the zone as the Assimil8or does
    (see LOOP_PLAYBACK.md), every output frame:

        trigger  a gate rise, or a retrigger, puts the playhead on Sample Start
        advance  the playhead moves on one frame
        wrap     if looping is enabled right now - Loop Mode On, or Loop Mode Loop/Release
                 with the gate high - and the playhead is at or past Loop End, it goes back
                 by whole loop lengths
        end      then, if the playhead is at or past Sample End, the voice ends

    Nothing ties the loop to the Sample Start..End window, so a loop that is wholly, or
    partly, before Sample Start is played, and one that ends past Sample End is never
    reached. With Play Mode Gated the voice lasts while the gate is high; with 1 Shot gate
    low is ignored. The Release is not modelled: a gated voice, or one stopped by a client,
    fades out over a few milliseconds only so that it does not click.
*/
class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener,
                    private juce::AsyncUpdater
{
public:
    void init (juce::ValueTree rootProperties);
    void shutdownAudio ();

    // published through SystemServices so the waveform displays can draw the playhead
    AuditionPlayhead& getAuditionPlayhead () noexcept { return auditionPlayhead; }

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
    juce::AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, false, false, true, false };

    // the zone's points as the engine uses them, clamped against the sample length only, in frames of sampleBuffer
    struct Window
    {
        double sampleStart { 0.0 };
        double sampleEnd { 0.0 };
        double loopStart { 0.0 };
        double loopEnd { 0.0 };
    };
    enum class VoiceState { idle, playing, fadingOut };
    // the channel's Play Mode and Loop Mode values
    static constexpr int kPlayModeGated { 0 };
    static constexpr int kLoopModeOn { 1 };
    static constexpr int kLoopModeGated { 2 };
    // the fade that keeps a stop from clicking - it is not the Release
    static constexpr double kDeclickFadeSeconds { 0.003 };

    juce::CriticalSection dataCS;
    Window window;
    VoiceState voiceState { VoiceState::idle };
    double playPosition { 0.0 };
    bool gateHigh { false };
    int playMode { 0 };
    int loopMode { 0 };
    int fadeLength { 1 };
    int fadeSamplesRemaining { 0 };
    int channelIndex { -1 };
    int zoneIndex { -1 };
    AuditionPlayhead auditionPlayhead;

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
            jassert (leftInput != nullptr || rightInput != nullptr);
        }

        void prepareToPlay (int, double) {}
        void releaseResources () {}
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
        {
            jassert (bufferToFill.buffer->getNumChannels () == 2);
            tempBuffer.setSize (2, bufferToFill.buffer->getNumSamples ());
            juce::AudioSourceChannelInfo tempBufferChannelInfo (&tempBuffer, 0, bufferToFill.numSamples);

            bufferToFill.buffer->clear ();
            if (leftInput != nullptr)
            {
                leftInput->getNextAudioBlock (tempBufferChannelInfo);
                bufferToFill.buffer->copyFrom (0, bufferToFill.startSample, tempBufferChannelInfo.buffer->getReadPointer (leftChannelIndex, 0),
                    juce::jmin (tempBufferChannelInfo.buffer->getNumSamples (), bufferToFill.numSamples));
            }
            if (rightInput != nullptr)
            {
                rightInput->getNextAudioBlock (tempBufferChannelInfo);
                bufferToFill.buffer->copyFrom (1, bufferToFill.startSample, tempBufferChannelInfo.buffer->getReadPointer (rightChannelIndex, 0),
                    juce::jmin (tempBufferChannelInfo.buffer->getNumSamples (), bufferToFill.numSamples));
            }
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
    void handleTrigger ();
    void handleGate (bool newGateHigh);
    void initFromZone (std::tuple<int, int> channelAndZoneIndecies);
    void updateWindow ();
    void updateModes ();
    void prepareSampleForPlayback ();
    void showConfigDialog ();
    void stopVoiceNow ();

    // the engine, called with dataCS held
    bool isLoopingEnabled () const noexcept;
    void wrapAndTestForEnd () noexcept;
    void startFadeOut () noexcept;

    void handleAsyncUpdate () override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
};