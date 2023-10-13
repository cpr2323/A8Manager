#include "AudioPlayer.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

AudioPlayer::AudioPlayer ()
{
    audioFormatManager.registerBasicFormats ();
}

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
        // TODO - maybe move this into a thread, as long files will block the UI
        prepareSampleForPlayback ();
    };
    audioPlayerProperties.onLoopStartChanged = [this] (int newLoopStart)
    {
        sampleStart = static_cast<int> (newLoopStart * sampleRateRatio);
        if (curSampleOffset > sampleStart || curSampleOffset > sampleStart + sampleLength)
            curSampleOffset = 0;
        juce::Logger::outputDebugString ("AudioPlayer - Loop Start: " + juce::String (sampleStart));
    };
    audioPlayerProperties.onLoopLengthChanged = [this] (int newLoopLength)
    {
        sampleLength = static_cast<int> (newLoopLength * sampleRateRatio);
        if (curSampleOffset > sampleStart + sampleLength)
            curSampleOffset = 0;
        juce::Logger::outputDebugString ("AudioPlayer - Loop Length: " + juce::String (sampleLength));
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

void AudioPlayer::handlePlayState (AudioPlayerProperties::PlayState newPlayState)
{
    if (playState == AudioPlayerProperties::PlayState::stop)
    {
        juce::Logger::outputDebugString ("AudioPlayer::handlePlayState: stop");
    }
    else if (playState == AudioPlayerProperties::PlayState::loop)
    {
        juce::Logger::outputDebugString ("AudioPlayer::handlePlayState: play");
        curSampleOffset = 0;
    }
    else if (playState == AudioPlayerProperties::PlayState::play)
    {
        juce::Logger::outputDebugString ("AudioPlayer::handlePlayState: play");
        curSampleOffset = 0;
    }
    playState = newPlayState;
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

void AudioPlayer::prepareSampleForPlayback ()
{
    if (audioFile.exists ())
    {
        std::unique_ptr <juce::AudioFormatReaderSource> readerSource = std::make_unique<juce::AudioFormatReaderSource> (audioFormatManager.createReaderFor (audioFile), true);
        std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (readerSource.get (), false, 2);
        sampleRateRatio = sampleRate / readerSource->getAudioFormatReader ()->sampleRate;
        resamplingAudioSource->setResamplingRatio (readerSource->getAudioFormatReader ()->sampleRate / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
        sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (readerSource->getAudioFormatReader ()->numChannels,
                                                                   static_cast<int> (readerSource->getAudioFormatReader ()->lengthInSamples * sampleRate / readerSource->getAudioFormatReader ()->sampleRate));
        resamplingAudioSource->getNextAudioBlock (juce::AudioSourceChannelInfo (*sampleBuffer.get ()));
        curSampleOffset = 0;
    }

}
void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    sampleRate = newSampleRate;
    blockSize = samplesPerBlockExpected;
    prepareSampleForPlayback ();
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

    if (playState == AudioPlayerProperties::PlayState::stop)
        return;

    const auto numOutputSamples { bufferToFill.numSamples };
    auto& outputBuffer { *bufferToFill.buffer };
    const auto channels { juce::jmin (outputBuffer.getNumChannels (), sampleBuffer->getNumChannels ()) };

    const auto originalSampleOffset { curSampleOffset };
    const auto cachedSampleLength { sampleLength };
    const auto cachedSampleStart { sampleStart };
    const auto chachedPlayState { playState };
    auto cachedSampleOffset { curSampleOffset };
    auto numSamplesToCopy { 0 };
    auto outputBufferWritePos { 0 };
    while (outputBufferWritePos < numOutputSamples)
    {
        numSamplesToCopy = juce::jmin (numOutputSamples - outputBufferWritePos, cachedSampleLength - cachedSampleOffset);

        // copy data from sample buffer to output buffer, this may, or may not, fill the entire output buffer
        auto ch { 0 };
        for (; ch < channels; ++ch)
            outputBuffer.copyFrom (ch, bufferToFill.startSample + outputBufferWritePos, *sampleBuffer, ch, cachedSampleStart + cachedSampleOffset, numSamplesToCopy);

        // clear any unused channels
        for (; ch < outputBuffer.getNumChannels (); ++ch)
            outputBuffer.clear (ch, bufferToFill.startSample + outputBufferWritePos, numSamplesToCopy);

        outputBufferWritePos += numSamplesToCopy;
        cachedSampleOffset += numSamplesToCopy;
        if (chachedPlayState == AudioPlayerProperties::PlayState::loop)
        {
            if (cachedSampleOffset >= cachedSampleLength)
                cachedSampleOffset = 0;
        }
        else
        {
            if (outputBufferWritePos < numOutputSamples)
            {
                outputBuffer.clear (bufferToFill.startSample + outputBufferWritePos, numOutputSamples - outputBufferWritePos);
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
                break;
            }
        }
    }
    if (originalSampleOffset == curSampleOffset)
        curSampleOffset = cachedSampleOffset;
}
