#include "AudioPlayer.h"
#include "../../Utility/PersistentRootProperties.h"

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);

    audioConfigProperties.wrap (persistentRootProperties.getValueTree (), AudioConfigProperties::WrapperType::owner, AudioConfigProperties::EnableCallbacks::yes);
    audioConfigProperties.onDeviceNameChange = [this] (juce::String deviceName)
    {
        //configureAudioDevice (deviceName);
    };
    audioConfigProperties.onPlayStateChange = [this] (AudioConfigProperties::PlayState playState)
    {
        handlePlayState (playState);
    };
    audioConfigProperties.onSourceFileChanged = [this] (juce::String sourceFile)
    {
        juce::Logger::outputDebugString ("AudioPlayer - Source File: " + sourceFile);
        audioFile = juce::File (sourceFile);
        juce::AudioFormatManager audioFormatManager;
        audioFormatManager.registerBasicFormats ();
        readerSource = std::make_unique<juce::AudioFormatReaderSource> (audioFormatManager.createReaderFor (audioFile), true);
        resamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (readerSource.get (), false, 2);

        resamplingAudioSource->setResamplingRatio (readerSource->getAudioFormatReader ()->sampleRate / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
    };
    audioConfigProperties.onLoopStartChanged = [this] (int loopStart)
    {
        juce::Logger::outputDebugString ("AudioPlayer - Loop Start: " + juce::String(loopStart));
    };
    audioConfigProperties.onLoopEndChanged = [this] (int loopEnd)
    {
        juce::Logger::outputDebugString ("AudioPlayer - Loop End: " + juce::String (loopEnd));
    };
    audioConfigProperties.onShowConfigDialog = [this] () { showConfigDialog (); };
    audioDeviceManager.addChangeListener (this);
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
        juce::Logger::outputDebugString ("AudioPlayer::handlePlayState: stop");
        // TODO - stop playback, with quick fade out
        playing = false;
    }
    else if (playState == AudioConfigProperties::play)
    {
        juce::Logger::outputDebugString ("AudioPlayer::handlePlayState: play");
        // TODO - start playback, with quick fade in
        playing = true;
    }
}

void AudioPlayer::showConfigDialog ()
{
    juce::DialogWindow::LaunchOptions o;
    o.escapeKeyTriggersCloseButton = true;
    o.dialogBackgroundColour = juce::Colours::grey;
    o.dialogTitle = "AUDIO SETTTINGS";
    audioSetupComp.setBounds (0, 0, 400, 600);
    o.content.set (&audioSetupComp, false);
    o.launchAsync ();
}

void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    sampleRate = newSampleRate;
    blockSize = samplesPerBlockExpected;

    if (resamplingAudioSource != nullptr)
    {
        resamplingAudioSource->setResamplingRatio (readerSource->getAudioFormatReader ()->sampleRate / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
    }
}

void AudioPlayer::releaseResources ()
{
    if (resamplingAudioSource != nullptr)
        resamplingAudioSource->releaseResources ();
}

void AudioPlayer::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    juce::Logger::outputDebugString ("audio device settings changed");
    auto currentAudioSetup { audioDeviceManager.getAudioDeviceSetup () };
    audioConfigProperties.setDeviceName (currentAudioSetup.outputDeviceName, false);
}

void AudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion ();
    // fill buffer with data

    if (! playing)
        return;

    if (readerSource->getNextReadPosition () > readerSource->getTotalLength ())
        readerSource->setNextReadPosition (0);

    resamplingAudioSource->getNextAudioBlock (bufferToFill);
}
