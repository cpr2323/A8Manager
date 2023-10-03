#include "AudioPlayer.h"
#include "../../Utility/PersistentRootProperties.h"

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    audioConfigProperties.wrap (persistentRootProperties.getValueTree (), AudioConfigProperties::WrapperType::owner, AudioConfigProperties::EnableCallbacks::yes);
    audioConfigProperties.onDeviceNameChange = [this] (juce::String deviceName)
    {
        configureAudioDevice (deviceName);
    };
    audioConfigProperties.onPlayStateChange= [this] (AudioConfigProperties::PlayState playState)
    {
        handlePlayState (playState);
    };
    configureAudioDevice (audioConfigProperties.getDeviceName ());
}

void AudioPlayer::shutdownAudio ()
{
    audioSourcePlayer.setSource (nullptr);
    audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    audioDeviceManager.closeAudioDevice ();
}

void AudioPlayer::configureAudioDevice (juce::String deviceName)
{
    juce::String audioConfigError;
    if (deviceName.isEmpty ())
    {
        audioConfigError = audioDeviceManager.initialise (0, 2, nullptr, true);
    }
    else
    {
        juce::AudioDeviceManager::AudioDeviceSetup audioDeviceSetup;
        audioDeviceSetup.outputDeviceName = deviceName;
        audioConfigError = audioDeviceManager.initialise (0, 2, nullptr, true, {}, &audioDeviceSetup);
    }

    if (! audioConfigError.isEmpty ())
    {
        jassertfalse;
    }

    // if the configuration device differs from the requested device, update AudioConfig
    const auto setup = audioDeviceManager.getAudioDeviceSetup ();
    if (setup.outputDeviceName != deviceName)
        audioConfigProperties.setDeviceName (setup.outputDeviceName, false);

    audioDeviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (this);
}

void AudioPlayer::handlePlayState (AudioConfigProperties::PlayState playState)
{
    if (playState == AudioConfigProperties::stop)
    {
        // TODO - stop playback
    }
    else if (playState == AudioConfigProperties::play)
    {
        // TODO - start playback
    }
}

void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    sampleRate = newSampleRate;
    blockSize = samplesPerBlockExpected;
}

void AudioPlayer::releaseResources ()
{
}

void AudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion ();
    // fill buffer with data
}
