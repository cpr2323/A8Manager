#include "AudioPlayer.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

#define LOG_AUDIO_PLAYER 0
#if LOG_AUDIO_PLAYER
#define LogAudioPlayer(text) juce::Logger::outputDebugString (text);
#else
#define LogAudioPlayer(text) ;
#endif

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
    audioSettingsProperties.onConfigChange = [this] (juce::String config)
    {
        //configureAudioDevice (deviceName);
    };

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::owner, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onShowConfigDialog = [this] () { showConfigDialog (); };
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState newPlayState)
    {
        handlePlayState (newPlayState);
    };
    audioPlayerProperties.onSourceFileChanged = [this] (juce::String sourceFile)
    {
        jassert (playState == AudioPlayerProperties::PlayState::stop);
        LogAudioPlayer ("AudioPlayer - Source File: " + sourceFile);
        audioFile = juce::File (sourceFile);
        // TODO - maybe move this into a thread, as long files will block the UI
        prepareSampleForPlayback ();
    };
    audioPlayerProperties.onLoopStartChanged = [this] (int newLoopStart)
    {
        {
            juce::ScopedLock sl (dataCS);
            sampleStart = static_cast<int> (newLoopStart * sampleRateRatio);
        }
        LogAudioPlayer ("AudioPlayer - Loop Start: " + juce::String (sampleStart));
    };
    audioPlayerProperties.onLoopLengthChanged = [this] (int newLoopLength)
    {
        {
            juce::ScopedLock sl (dataCS);
            sampleLength = static_cast<int> (newLoopLength * sampleRateRatio);
            if (curSampleOffset > sampleLength)
                curSampleOffset = 0;
        }
        LogAudioPlayer ("AudioPlayer - Loop Length: " + juce::String (sampleLength));
    };
    audioDeviceManager.addChangeListener (this);
    configureAudioDevice (audioSettingsProperties.getConfig ());
}

void AudioPlayer::shutdownAudio ()
{
    audioSourcePlayer.setSource (nullptr);
    audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    audioDeviceManager.closeAudioDevice ();
}

void AudioPlayer::configureAudioDevice (juce::String config)
{
    juce::String audioConfigError;
    if (config.isEmpty ())
    {
        audioConfigError = audioDeviceManager.initialise (0, 2, nullptr, true);
    }
    else
    {
        auto audioConfigXml { juce::XmlDocument::parse (config) };
        audioConfigError = audioDeviceManager.initialise (0, 2, audioConfigXml.get (), true, {}, nullptr);
    }

    if (! audioConfigError.isEmpty ())
    {
        jassertfalse;
    }

    audioDeviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (this);
}

void AudioPlayer::handlePlayState (AudioPlayerProperties::PlayState newPlayState)
{
    juce::ScopedLock sl (dataCS);
    if (playState == AudioPlayerProperties::PlayState::stop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: stop");
    }
    else if (playState == AudioPlayerProperties::PlayState::loop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: play");
        curSampleOffset = 0;
    }
    else if (playState == AudioPlayerProperties::PlayState::play)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: play");
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
    LogAudioPlayer ("audio device settings changed");
    auto audioDeviceSettings { audioDeviceManager.createStateXml () };
    if(audioDeviceSettings != nullptr)
    {
        auto xmlString { audioDeviceSettings->toString () };
        audioSettingsProperties.setConfig (xmlString, false);
    }
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
    auto cachedSampleLength { 0 };
    auto cachedSampleStart { 0 };
    auto cachedSampleOffset { 0 };
    auto chachedPlayState { AudioPlayerProperties::PlayState::stop };
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        cachedSampleLength = sampleLength;
        cachedSampleStart = sampleStart;
        chachedPlayState = playState;
        cachedSampleOffset = curSampleOffset;
    }
    auto numSamplesToCopy { 0 };
    auto outputBufferWritePos { 0 };
    while (cachedSampleLength > 0 && outputBufferWritePos < numOutputSamples)
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
