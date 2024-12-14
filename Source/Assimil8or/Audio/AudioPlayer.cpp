#include "AudioPlayer.h"
#include "../../Assimil8or/PresetManagerProperties.h"
#include "../../Utility/DebugLog.h"
#include "../../Utility/PersistentRootProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

#define LOG_AUDIO_PLAYER 0
#if LOG_AUDIO_PLAYER
#define LogAudioPlayer(text) DebugLog ("AudioPlayer", text);
#else
#define LogAudioPlayer(text) ;
#endif

#define LOG_AUDIO_PLAY_BACK 0
#if LOG_AUDIO_PLAY_BACK
#define LogAudioPlayback(text) DebugLog ("AudioPlayer", text);
#else
#define LogAudioPlayback(text) ;
#endif

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);

    audioSettingsProperties.wrap (persistentRootProperties.getValueTree (), AudioSettingsProperties::WrapperType::owner, AudioSettingsProperties::EnableCallbacks::yes);
    audioSettingsProperties.onConfigChange = [this] (juce::String config)
    {
            // TODO - do we need this callback?
        //configureAudioDevice (deviceName);
    };

    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::owner, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onShowConfigDialog = [this] () { showConfigDialog (); };
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState newPlayState)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onPlayStateChange");
        handlePlayState (newPlayState);
    };
    // Clients call this to setup the sample source
    audioPlayerProperties.onSampleSourceChanged = [this] (std::tuple<int, int> channelAndZoneIndecies)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onSampleSourceChanged");
        initFromZone (channelAndZoneIndecies);
    };
    // Clients call this to change which sample points are used, Sample or Loop
    audioPlayerProperties.onSamplePointsSelectorChanged = [this] (AudioPlayerProperties::SamplePointsSelector)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onSamplePointsSelectorChanged");
        initSamplePoints ();
    };
    audioDeviceManager.addChangeListener (this);
    configureAudioDevice (audioSettingsProperties.getConfig ());
}

void AudioPlayer::initFromZone (std::tuple<int, int> channelAndZoneIndecies)
{
    LogAudioPlayer ("initFromZone");
    auto [channelIndex, zoneIndex] {channelAndZoneIndecies};
    jassert (channelIndex < 8);
    jassert (zoneIndex < 8);
    channelProperties.wrap (presetProperties.getChannelVT (channelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    zoneProperties.wrap (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelIndex, zoneIndex), SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::yes);

    if (channelIndex < 7)
    {
        const auto nextChannelIndex { channelIndex + 1 };
        nextChannelProperties.wrap (presetProperties.getChannelVT (nextChannelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
        nextZoneProperties.wrap (nextChannelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
        nextSampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (nextChannelIndex, zoneIndex), SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::yes);
    }
    else
    {
        nextChannelProperties.wrap ({}, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        nextZoneProperties.wrap ({}, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        nextSampleProperties.wrap ({}, SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::no);
    }

    channelProperties.onChannelModeChange = [this] (int)
    {
        LogAudioPlayer ("channelProperties.onChannelModeChange");
        initSamplePoints ();
        prepareSampleForPlayback ();
    };
    zoneProperties.onSampleChange = [this] (juce::String)
    {
        LogAudioPlayer ("zoneProperties.onSampleChange");
        initSamplePoints ();
        prepareSampleForPlayback ();
    };
    zoneProperties.onSideChange = [this] (int)
    {
        LogAudioPlayer ("zoneProperties.onSideChange");
        initSamplePoints ();
        prepareSampleForPlayback ();
    };
    nextChannelProperties.onChannelModeChange = [this] (int)
    {
        LogAudioPlayer ("nextChannelProperties.onChannelModeChange");
        prepareSampleForPlayback ();
    };
    nextZoneProperties.onSampleChange = [this] (juce::String)
    {
        LogAudioPlayer ("nextZoneProperties.onSampleChange");
        prepareSampleForPlayback ();
    };
    nextZoneProperties.onSideChange = [this] (int)
    {
        LogAudioPlayer ("nextZoneProperties.onSideChange");
        prepareSampleForPlayback ();
    };

    // TODO - can I refactor these callback?
    //  issues:
    //      onSampleStartChange recalculates sampleLength, but onLoopStartChange does NOT change sampleLength
    //      onLoopLengthChange a double parameter, the other three have int64
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64> newSampleStart)
    {
        LogAudioPlayer ("zoneProperties.onSampleStartChange");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::LoopPoints)
            return;
        sampleStart = static_cast<int> (newSampleStart.value_or (0) * sampleRateRatio);
        sampleLength = static_cast<int> ((zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) - zoneProperties.getSampleStart ().value_or (0)) * sampleRateRatio);
        if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("zoneProperties.onSampleStartChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    zoneProperties.onSampleEndChange = [this] (std::optional <juce::int64> newSampleEnd)
    {
        LogAudioPlayer ("zoneProperties.onSampleEndChange");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::LoopPoints)
            return;
        sampleLength = static_cast<int> ((newSampleEnd.value_or (sampleProperties.getLengthInSamples ()) - zoneProperties.getSampleStart ().value_or (0)) * sampleRateRatio);
        if (curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("zoneProperties.onSampleEndChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    zoneProperties.onLoopStartChange = [this] (std::optional <juce::int64> newLoopStart)
    {
        LogAudioPlayer ("zoneProperties.onLoopStartChange");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
            return;
        sampleStart = static_cast<int> (newLoopStart.value_or (0) * sampleRateRatio);
        if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("zoneProperties.onLoopStartChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double> newLoopLength)
    {
        LogAudioPlayer ("zoneProperties.onLoopLengthChange");
        jassert (sampleRateRatio > 0.0);
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
            return;
        sampleLength = static_cast<int> (newLoopLength.value_or (sampleProperties.getLengthInSamples ()) * sampleRateRatio);
        if (curSampleOffset >= sampleStart + sampleLength)
            curSampleOffset = sampleStart;
        LogAudioPlayer ("zoneProperties.onLoopLengthChange - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
    };
    sampleProperties.onStatusChange = [this] (SampleStatus status)
    {
        if (status == SampleStatus::exists)
        {
            LogAudioPlayer ("sampleProperties.onStatusChange: SampleStatus::exists");
            initSamplePoints ();
            prepareSampleForPlayback ();
        }
        else
        {
            // TODO - reset somethings?
            LogAudioPlayer ("sampleProperties.onStatusChange: NOT SampleStatus::exists");
            audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
            {
                juce::ScopedLock sl (dataCS);
                sampleStart = 0;
                sampleLength = 0;
                sampleBuffer.reset ();
            }
        }
    };
    nextSampleProperties.onStatusChange = [this] (SampleStatus status)
    {
        if (status == SampleStatus::exists)
        {
            LogAudioPlayer ("nextSampleProperties.onStatusChange: SampleStatus::exists");
            initSamplePoints ();
            prepareSampleForPlayback ();
        }
        else
        {
            // TODO - reset somethings?
            LogAudioPlayer ("nextSampleProperties.onStatusChange: NOT SampleStatus::exists");
            audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
            {
                juce::ScopedLock sl (dataCS);
                sampleStart = 0;
                sampleLength = 0;
                sampleBuffer.reset ();
            }
        }
    };

    // create local copy of audio data, with resampling if needed
    prepareSampleForPlayback ();

    // setup local sample start and sample length based on samplePointsSource
    initSamplePoints ();
}

void AudioPlayer::initSamplePoints ()
{
    LogAudioPlayer ("initSamplePoints");
    jassert (sampleRateRatio > 0.0);
    juce::ScopedLock sl (dataCS);
    if (audioPlayerProperties.getSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
    {
        LogAudioPlayer (" using SamplePoints");
        sampleStart = static_cast<int> (zoneProperties.getSampleStart ().value_or (0) * sampleRateRatio);
        sampleLength = static_cast<int> ((zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) - zoneProperties.getSampleStart ().value_or (0)) * sampleRateRatio);
    }
    else
    {
        LogAudioPlayer (" using LoopPoints");
        sampleStart = static_cast<int> (zoneProperties.getLoopStart ().value_or (0) * sampleRateRatio);
        sampleLength = static_cast<int> (zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()) * sampleRateRatio);
    }
    if (curSampleOffset < sampleStart || curSampleOffset >= sampleStart + sampleLength)
        curSampleOffset = sampleStart;
    LogAudioPlayer ("AudioPlayer::initSamplePoints - sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength) + ", curSampleOffset: " + juce::String (curSampleOffset));
}

void AudioPlayer::prepareSampleForPlayback ()
{
    jassert (playState == AudioPlayerProperties::PlayState::stop);
    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        juce::AudioSource* leftAudioSource { nullptr };
        juce::AudioSource* rightAudioSource { nullptr };
        int leftAudioSourceChannel { 0 };
        int rightAudioSourceChannel { 0 };

        LogAudioPlayer ("prepareSampleForPlayback: sample is ready");
        LogAudioPlayer ("prepareSampleForPlayback: num channels: " + juce::String (sampleProperties.getAudioBufferPtr ()->getNumChannels ()));
        std::unique_ptr <juce::MemoryAudioSource> leftReaderSource;
        std::unique_ptr<juce::ResamplingAudioSource> leftResamplingAudioSource;
        std::unique_ptr <juce::MemoryAudioSource> rightReaderSource;
        std::unique_ptr<juce::ResamplingAudioSource> rightResamplingAudioSource;

        if (channelProperties.getChannelMode () == ChannelProperties::ChannelMode::master && nextChannelProperties.getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
        {
            LogAudioPlayer ("prepareSampleForPlayback: master channel only");
            leftReaderSource = std::make_unique<juce::MemoryAudioSource> (*sampleProperties.getAudioBufferPtr (), false, false);
            leftResamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (leftReaderSource.get (), false, 1);
            sampleRateRatio = sampleRate / sampleProperties.getSampleRate (); // we use the master channel sample rate, since we use the sample points from that
            leftResamplingAudioSource->setResamplingRatio (sampleProperties.getSampleRate () / sampleRate);
            leftResamplingAudioSource->prepareToPlay (blockSize, sampleRate);
            leftAudioSource = leftResamplingAudioSource.get ();
            leftAudioSourceChannel = zoneProperties.getSide ();

            rightReaderSource = std::make_unique<juce::MemoryAudioSource> (*sampleProperties.getAudioBufferPtr (), false, false);
            rightResamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (rightReaderSource.get (), false, 1);
            rightResamplingAudioSource->setResamplingRatio (sampleProperties.getSampleRate () / sampleRate);
            rightResamplingAudioSource->prepareToPlay (blockSize, sampleRate);
            rightAudioSource = rightResamplingAudioSource.get ();
            rightAudioSourceChannel = zoneProperties.getSide ();
        }
        else if (channelProperties.getChannelMode () == ChannelProperties::ChannelMode::master && nextChannelProperties.getChannelMode () == ChannelProperties::ChannelMode::stereoRight)
        {
            LogAudioPlayer ("prepareSampleForPlayback: master and stereo/right");
            leftReaderSource = std::make_unique<juce::MemoryAudioSource> (*sampleProperties.getAudioBufferPtr (), false, false);
            leftResamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (leftReaderSource.get (), false, 2);
            sampleRateRatio = sampleRate / sampleProperties.getSampleRate (); // we use the master channel sample rate, since we use the sample points from that
            leftResamplingAudioSource->setResamplingRatio (sampleProperties.getSampleRate () / sampleRate);
            leftResamplingAudioSource->prepareToPlay (blockSize, sampleRate);
            leftAudioSource = leftResamplingAudioSource.get ();
            leftAudioSourceChannel = zoneProperties.getSide ();

            if (nextSampleProperties.getStatus () == SampleStatus::exists)
            {
                rightReaderSource = std::make_unique<juce::MemoryAudioSource> (*nextSampleProperties.getAudioBufferPtr (), false, false);
                rightResamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (rightReaderSource.get (), false, 2);
                rightResamplingAudioSource->setResamplingRatio (nextSampleProperties.getSampleRate () / sampleRate);
                rightResamplingAudioSource->prepareToPlay (blockSize, sampleRate);
                rightAudioSource = rightResamplingAudioSource.get ();
                rightAudioSourceChannel = nextZoneProperties.getSide ();
            }
        }
        else
        {
            jassertfalse;
            sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (2, 0);
            curSampleOffset = 0;
            return;
        }

        jassert (leftAudioSource != nullptr || rightAudioSource != nullptr);
        std::unique_ptr<LeftRightCombinerAudioSource> leftRightCombinerAudioSource { std::make_unique<LeftRightCombinerAudioSource> (leftAudioSource, leftAudioSourceChannel,
                                                                                                                                     rightAudioSource, rightAudioSourceChannel, false) };

        sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (2, static_cast<int> (sampleProperties.getLengthInSamples () * sampleRate / sampleProperties.getSampleRate ()));

        leftRightCombinerAudioSource->getNextAudioBlock (juce::AudioSourceChannelInfo (*sampleBuffer.get ()));
        curSampleOffset = 0;
    }
    else
    {
        LogAudioPlayer ("prepareSampleForPlayback: sample is NOT ready");
    }
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
    if (newPlayState == AudioPlayerProperties::PlayState::stop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: stop");
    }
    else if (newPlayState == AudioPlayerProperties::PlayState::loop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: loop");
        curSampleOffset = sampleStart;
    }
    else if (newPlayState == AudioPlayerProperties::PlayState::play)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: play");
        curSampleOffset = sampleStart;
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

void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    LogAudioPlayer ("prepareToPlay");
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
    if (audioDeviceSettings != nullptr)
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

    auto originalSampleOffset { 0 };
    auto cachedSampleLength { 0 };
    auto cachedSampleStart { 0 };
    auto cachedLocalSampleOffset { 0 };
    auto chachedPlayState { AudioPlayerProperties::PlayState::stop };
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        jassert (curSampleOffset >= sampleStart);
        jassert (curSampleOffset < sampleStart + sampleLength);
        originalSampleOffset =  curSampleOffset; // should be >= sampleStart and < sampleStart + sampleLength
        cachedSampleLength = sampleLength;
        cachedSampleStart = sampleStart;
        chachedPlayState = playState;
        cachedLocalSampleOffset = curSampleOffset - sampleStart;
        LogAudioPlayback ("AudioPlayer::getNextAudioBlock - cachedSampleStart: " + juce::String (cachedSampleStart) + ", chachedSampleLength: " + juce::String (cachedSampleLength) +
                        ", curSampleOffset: " + juce::String (curSampleOffset) + ", cachedLocalSampleOffset: " + juce::String (cachedLocalSampleOffset));
    }
    auto numSamplesToCopy { 0 };
    auto outputBufferWritePos { 0 };
    while (cachedSampleLength > 0 && outputBufferWritePos < numOutputSamples)
    {
        numSamplesToCopy = juce::jmin (numOutputSamples - outputBufferWritePos, cachedSampleLength - cachedLocalSampleOffset);
        LogAudioPlayback ("AudioPlayer::getNextAudioBlock - numSamplesToCopy: " + juce::String (numSamplesToCopy));
        jassert (numSamplesToCopy >= 0);

        // copy data from sample buffer to output buffer, this may, or may not, fill the entire output buffer
        auto ch { 0 };
        for (; ch < channels; ++ch)
            outputBuffer.copyFrom (ch, bufferToFill.startSample + outputBufferWritePos, *sampleBuffer, ch, cachedSampleStart + cachedLocalSampleOffset, numSamplesToCopy);

        // clear any unused channels
        for (; ch < outputBuffer.getNumChannels (); ++ch)
            outputBuffer.clear (ch, bufferToFill.startSample + outputBufferWritePos, numSamplesToCopy);

        outputBufferWritePos += numSamplesToCopy;
        cachedLocalSampleOffset += numSamplesToCopy;
        if (chachedPlayState == AudioPlayerProperties::PlayState::loop)
        {
            if (cachedLocalSampleOffset >= cachedSampleLength)
                cachedLocalSampleOffset = 0;
        }
        else
        {
            LogAudioPlayback ("AudioPlayer::getNextAudioBlock - outputBufferWritePos : " + juce::String (outputBufferWritePos) + ", numOutputSamples: " + juce::String (numOutputSamples));
            if (outputBufferWritePos < numOutputSamples)
            {
                outputBuffer.clear (bufferToFill.startSample + outputBufferWritePos, numOutputSamples - outputBufferWritePos);
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
                break;
            }
        }
    }
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        if (originalSampleOffset == curSampleOffset && cachedSampleStart == sampleStart && cachedSampleLength == sampleLength) // if the offset has not changed externally
        {
            curSampleOffset = cachedSampleStart + cachedLocalSampleOffset;
            LogAudioPlayback ("AudioPlayer::getNextAudioBlock setting new curSampleOffset: " + juce::String (curSampleOffset) + ", cachedSampleStart: " + juce::String (cachedSampleStart) + ", chachedSampleLength: " + juce::String (cachedSampleLength) +
                            ", cachedLocalSampleOffset: " + juce::String (cachedLocalSampleOffset));
        }
        else
        {
            LogAudioPlayback ("AudioPlayer::getNextAudioBlock - curSampleOffset: " + juce::String (cachedSampleStart) + " != originalSampleOffset: " + juce::String (originalSampleOffset));
        }
    }
}
