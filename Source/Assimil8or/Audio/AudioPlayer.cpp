#include "AudioPlayer.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);

    audioSettingsProperties.wrap (persistentRootProperties.getValueTree (), AudioSettingsProperties::WrapperType::owner, AudioSettingsProperties::EnableCallbacks::yes);
    audioSettingsProperties.onDeviceNameChange = [this] (juce::String deviceName)
    {
        //configureAudioDevice (deviceName);
    };

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::owner, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onShowConfigDialog = [this] () { showConfigDialog (); };
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState playState)
    {
        handlePlayState (playState);
    };
    audioPlayerProperties.onSourceFileChanged = [this] (juce::String sourceFile)
    {
        juce::Logger::outputDebugString ("AudioPlayer - Source File: " + sourceFile);
        audioFile = juce::File (sourceFile);
        juce::AudioFormatManager audioFormatManager;
        audioFormatManager.registerBasicFormats ();
        std::unique_ptr <juce::AudioFormatReaderSource> readerSource = std::make_unique<juce::AudioFormatReaderSource> (audioFormatManager.createReaderFor (audioFile), true);
        std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (readerSource.get (), false, 2);
        resamplingAudioSource->setResamplingRatio (readerSource->getAudioFormatReader ()->sampleRate / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
        sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (readerSource->getAudioFormatReader ()->numChannels,
                                                                   static_cast<int>(readerSource->getAudioFormatReader ()->lengthInSamples * sampleRate / readerSource->getAudioFormatReader ()->sampleRate));
        // resample into output buffer
        resamplingAudioSource->getNextAudioBlock (juce::AudioSourceChannelInfo (*sampleBuffer.get ()));
        savedSampleBufferReadPos = 0;
    };
    audioPlayerProperties.onLoopStartChanged = [this] (int newLoopStart)
    {
        loopStart = newLoopStart;
        juce::Logger::outputDebugString ("AudioPlayer - Loop Start: " + juce::String (loopStart));
    };
    audioPlayerProperties.onLoopLengthChanged = [this] (int newLoopLength)
    {
        loopLength = newLoopLength;
        juce::Logger::outputDebugString ("AudioPlayer - Loop Length: " + juce::String (loopLength));
    };
    audioDeviceManager.addChangeListener (this);
    configureAudioDevice (audioSettingsProperties.getDeviceName ());
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
        audioSettingsProperties.setDeviceName (setup.outputDeviceName, false);

    audioDeviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (this);
}

void AudioPlayer::handlePlayState (AudioPlayerProperties::PlayState playState)
{
    if (playState == AudioPlayerProperties::PlayState::stop)
    {
        juce::Logger::outputDebugString ("AudioPlayer::handlePlayState: stop");
        // TODO - stop playback, with quick fade out
        playing = false;
    }
    else if (playState == AudioPlayerProperties::PlayState::play)
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

    // TODO - need to resample here if sample rate changes
}

void AudioPlayer::releaseResources ()
{
}

void AudioPlayer::changeListenerCallback (juce::ChangeBroadcaster*)
{
    juce::Logger::outputDebugString ("audio device settings changed");
    auto currentAudioSetup { audioDeviceManager.getAudioDeviceSetup () };
    audioSettingsProperties.setDeviceName (currentAudioSetup.outputDeviceName, false);
}

void AudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion ();
    // fill buffer with data

    if (! playing)
        return;

    auto& dst = *bufferToFill.buffer;
    auto channels = juce::jmin (dst.getNumChannels (), sampleBuffer->getNumChannels ());
    int numSamplesToCopy = 0;
    int outputBufferWritePos = 0;
    const auto numInputSamples { loopLength };
    const auto numOutputSamples { bufferToFill.numSamples };

    int curSampleBufferReadPos = savedSampleBufferReadPos;
    for (; outputBufferWritePos < numOutputSamples; curSampleBufferReadPos += numSamplesToCopy)
    {
        numSamplesToCopy = juce::jmin (numOutputSamples - outputBufferWritePos, numInputSamples - (curSampleBufferReadPos % numInputSamples));

        int ch = 0;
        for (; ch < channels; ++ch)
            dst.copyFrom (ch, bufferToFill.startSample + outputBufferWritePos, *sampleBuffer, ch, loopStart + (curSampleBufferReadPos % numInputSamples), numSamplesToCopy);

        for (; ch < dst.getNumChannels (); ++ch)
            dst.clear (ch, bufferToFill.startSample + outputBufferWritePos, numSamplesToCopy);

        outputBufferWritePos += numSamplesToCopy;
    }

    if (outputBufferWritePos < numOutputSamples)
        dst.clear (bufferToFill.startSample + outputBufferWritePos, numOutputSamples - outputBufferWritePos);

    savedSampleBufferReadPos = curSampleBufferReadPos;
}
