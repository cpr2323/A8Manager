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

void AudioPlayer::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    sampleManagerProperties.wrap (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::owner, AppProperties::EnableCallbacks::yes);

    audioSettingsProperties.wrap (persistentRootProperties.getValueTree (), AudioSettingsProperties::WrapperType::owner, AudioSettingsProperties::EnableCallbacks::no);

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
    // Clients call these to play the voice, as the gate input of the module does
    audioPlayerProperties.onTrigger = [this] ()
    {
        LogAudioPlayer ("init: audioPlayerProperties.onTrigger");
        handleTrigger ();
    };
    audioPlayerProperties.onGateChange = [this] (bool newGateHigh)
    {
        LogAudioPlayer ("init: audioPlayerProperties.onGateChange");
        handleGate (newGateHigh);
    };
    audioDeviceManager.addChangeListener (this);
    configureAudioDevice (audioSettingsProperties.getConfig ());
}

void AudioPlayer::initFromZone (std::tuple<int, int> channelAndZoneIndecies)
{
    LogAudioPlayer ("initFromZone");
    auto [newChannelIndex, newZoneIndex] { channelAndZoneIndecies };
    jassert (newChannelIndex < 8);
    jassert (newZoneIndex < 8);
    // the voice belongs to the zone it was playing
    stopVoiceNow ();
    {
        juce::ScopedLock sl (dataCS);
        channelIndex = newChannelIndex;
        zoneIndex = newZoneIndex;
    }
    channelProperties.wrap (presetProperties.getChannelVT (newChannelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
    zoneProperties.wrap (channelProperties.getZoneVT (newZoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (newChannelIndex, newZoneIndex), SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::yes);

    if (newChannelIndex < 7)
    {
        const auto nextChannelIndex { newChannelIndex + 1 };
        nextChannelProperties.wrap (presetProperties.getChannelVT (nextChannelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::yes);
        nextZoneProperties.wrap (nextChannelProperties.getZoneVT (newZoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
        nextSampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (nextChannelIndex, newZoneIndex), SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::yes);
    }
    else
    {
        nextChannelProperties.wrap ({}, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        nextZoneProperties.wrap ({}, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        nextSampleProperties.wrap ({}, SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::no);
    }

    // a different sample, or a different pair of samples, has to be prepared again, and ends the voice
    auto preparePlayback = [this] ()
    {
        prepareSampleForPlayback ();
        updateWindow ();
    };
    channelProperties.onChannelModeChange = [this, preparePlayback] (int)
    {
        LogAudioPlayer ("channelProperties.onChannelModeChange");
        preparePlayback ();
    };
    zoneProperties.onSampleChange = [this, preparePlayback] (juce::String)
    {
        LogAudioPlayer ("zoneProperties.onSampleChange");
        preparePlayback ();
    };
    zoneProperties.onSideChange = [this, preparePlayback] (int)
    {
        LogAudioPlayer ("zoneProperties.onSideChange");
        preparePlayback ();
    };
    nextChannelProperties.onChannelModeChange = [this, preparePlayback] (int)
    {
        LogAudioPlayer ("nextChannelProperties.onChannelModeChange");
        preparePlayback ();
    };
    nextZoneProperties.onSampleChange = [this, preparePlayback] (juce::String)
    {
        LogAudioPlayer ("nextZoneProperties.onSampleChange");
        preparePlayback ();
    };
    nextZoneProperties.onSideChange = [this, preparePlayback] (int)
    {
        LogAudioPlayer ("nextZoneProperties.onSideChange");
        preparePlayback ();
    };

    // As on the module, the points and the modes can change while the voice plays: the playhead stays where it is,
    // and the rules carry on with the new values
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64>) { updateWindow (); };
    zoneProperties.onSampleEndChange = [this] (std::optional <juce::int64>) { updateWindow (); };
    zoneProperties.onLoopStartChange = [this] (std::optional <juce::int64>) { updateWindow (); };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double>) { updateWindow (); };
    channelProperties.onPlayModeChange = [this] (int) { updateModes (); };
    channelProperties.onLoopModeChange = [this] (int) { updateModes (); };

    sampleProperties.onStatusChange = [this, preparePlayback] (SampleStatus status)
    {
        LogAudioPlayer ("sampleProperties.onStatusChange");
        if (status == SampleStatus::exists)
        {
            preparePlayback ();
        }
        else
        {
            stopVoiceNow ();
            juce::ScopedLock sl (dataCS);
            window = {};
            sampleBuffer.reset ();
        }
    };
    nextSampleProperties.onStatusChange = [this, preparePlayback] (SampleStatus status)
    {
        LogAudioPlayer ("nextSampleProperties.onStatusChange");
        if (status == SampleStatus::exists)
        {
            preparePlayback ();
        }
        else
        {
            stopVoiceNow ();
            juce::ScopedLock sl (dataCS);
            window = {};
            sampleBuffer.reset ();
        }
    };

    // create local copy of audio data, with resampling if needed, and the points that go with it
    preparePlayback ();
    updateModes ();
}

void AudioPlayer::updateWindow ()
{
    Window newWindow;
    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        // As the module does, each point is clamped against the sample length, and never against the other points,
        // so the loop may lie before Sample Start, or past Sample End. The engine's shortest loop is 4 frames.
        constexpr auto kMinLoopLength { 4.0 };
        const auto frames { static_cast<double> (sampleProperties.getLengthInSamples ()) };
        newWindow.sampleStart = std::clamp (static_cast<double> (zoneProperties.getSampleStart ().value_or (0)), 0.0, frames);
        newWindow.sampleEnd = std::clamp (static_cast<double> (zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ())), 0.0, frames);
        if (frames >= kMinLoopLength)
        {
            const auto storedLoopStart { static_cast<double> (zoneProperties.getLoopStart ().value_or (0)) };
            const auto loopLength { std::min (zoneProperties.getLoopLength ().value_or (frames), frames - storedLoopStart) };
            const auto loopStart { std::min (std::clamp (storedLoopStart, 0.0, frames - kMinLoopLength), frames - loopLength) };
            newWindow.loopStart = loopStart;
            newWindow.loopEnd = std::clamp (loopStart + loopLength, loopStart + kMinLoopLength, frames);
        }
        else
        {
            newWindow.loopStart = 0.0;
            newWindow.loopEnd = frames;
        }
    }

    juce::ScopedLock sl (dataCS);
    // the points are in frames of the sample, and the playback buffer has been resampled to the device rate
    window.sampleStart = newWindow.sampleStart * sampleRateRatio;
    window.sampleEnd = newWindow.sampleEnd * sampleRateRatio;
    window.loopStart = newWindow.loopStart * sampleRateRatio;
    window.loopEnd = newWindow.loopEnd * sampleRateRatio;
    LogAudioPlayer ("updateWindow - sampleStart: " + juce::String (window.sampleStart) + ", sampleEnd: " + juce::String (window.sampleEnd) +
                    ", loopStart: " + juce::String (window.loopStart) + ", loopEnd: " + juce::String (window.loopEnd));
}

void AudioPlayer::updateModes ()
{
    const auto newPlayMode { channelProperties.isValid () ? channelProperties.getPlayMode () : 0 };
    const auto newLoopMode { channelProperties.isValid () ? channelProperties.getLoopMode () : 0 };
    juce::ScopedLock sl (dataCS);
    playMode = newPlayMode;
    loopMode = newLoopMode;
}

void AudioPlayer::prepareSampleForPlayback ()
{
    // the buffer is about to change under the voice
    stopVoiceNow ();
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

        // we use the master channel sample rate, since we use the sample points from that
        const auto newSampleRateRatio { sampleRate / sampleProperties.getSampleRate () };
        if (channelProperties.getChannelMode () == ChannelProperties::ChannelMode::master && nextChannelProperties.getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
        {
            LogAudioPlayer ("prepareSampleForPlayback: master channel only");
            leftReaderSource = std::make_unique<juce::MemoryAudioSource> (*sampleProperties.getAudioBufferPtr (), false, false);
            leftResamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (leftReaderSource.get (), false, 1);
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
            juce::ScopedLock sl (dataCS);
            sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (2, 0);
            return;
        }

        jassert (leftAudioSource != nullptr || rightAudioSource != nullptr);
        std::unique_ptr<LeftRightCombinerAudioSource> leftRightCombinerAudioSource { std::make_unique<LeftRightCombinerAudioSource> (leftAudioSource, leftAudioSourceChannel,
                                                                                                                                     rightAudioSource, rightAudioSourceChannel, false) };

        auto newSampleBuffer { std::make_unique<juce::AudioBuffer<float>> (2, static_cast<int> (sampleProperties.getLengthInSamples () * sampleRate / sampleProperties.getSampleRate ())) };
        leftRightCombinerAudioSource->getNextAudioBlock (juce::AudioSourceChannelInfo (*newSampleBuffer.get ()));

        juce::ScopedLock sl (dataCS);
        sampleBuffer = std::move (newSampleBuffer);
        sampleRateRatio = newSampleRateRatio;
    }
    else
    {
        LogAudioPlayer ("prepareSampleForPlayback: sample is NOT ready");
        juce::ScopedLock sl (dataCS);
        sampleBuffer.reset ();
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
    // a client stopping the voice, which is the only play state a client sets
    if (newPlayState == AudioPlayerProperties::PlayState::stop)
    {
        LogAudioPlayer ("AudioPlayer::handlePlayState: stop");
        juce::ScopedLock sl (dataCS);
        startFadeOut ();
    }
}

void AudioPlayer::handleTrigger ()
{
    LogAudioPlayer ("AudioPlayer::handleTrigger");
    {
        juce::ScopedLock sl (dataCS);
        if (sampleBuffer == nullptr || sampleBuffer->getNumSamples () == 0)
            return;
        // a gate rise always restarts the voice at Sample Start - Loop Start has no part in it
        playPosition = window.sampleStart;
        voiceState = VoiceState::playing;
        // a Sample Start at or past Loop End wraps before the first frame is heard
        wrapAndTestForEnd ();
        auditionPlayhead.channelIndex.store (channelIndex);
        auditionPlayhead.zoneIndex.store (zoneIndex);
    }
    audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::play, false);
}

void AudioPlayer::handleGate (bool newGateHigh)
{
    LogAudioPlayer ("AudioPlayer::handleGate: " + juce::String (newGateHigh ? "high" : "low"));
    // what the gate does is up to the modes, and the audio callback looks at it every frame
    juce::ScopedLock sl (dataCS);
    gateHigh = newGateHigh;
}

void AudioPlayer::stopVoiceNow ()
{
    juce::ScopedLock sl (dataCS);
    if (voiceState == VoiceState::idle)
        return;
    voiceState = VoiceState::idle;
    // published from the message thread, by handleAsyncUpdate
    triggerAsyncUpdate ();
}

bool AudioPlayer::isLoopingEnabled () const noexcept
{
    return loopMode == kLoopModeOn || (loopMode == kLoopModeGated && gateHigh);
}

void AudioPlayer::wrapAndTestForEnd () noexcept
{
    // the wrap is tested before the end, so a loop that ends on Sample End loops for ever
    if (isLoopingEnabled () && playPosition >= window.loopEnd)
    {
        const auto loopLength { window.loopEnd - window.loopStart };
        if (loopLength > 0.0)
            playPosition = window.loopStart + std::fmod (playPosition - window.loopEnd, loopLength);
    }
    if (playPosition >= window.sampleEnd)
    {
        voiceState = VoiceState::idle;
        triggerAsyncUpdate ();
    }
}

void AudioPlayer::startFadeOut () noexcept
{
    if (voiceState != VoiceState::playing)
        return;
    voiceState = VoiceState::fadingOut;
    fadeSamplesRemaining = fadeLength;
}

void AudioPlayer::handleAsyncUpdate ()
{
    {
        juce::ScopedLock sl (dataCS);
        // retriggered since the voice ended
        if (voiceState != VoiceState::idle)
            return;
    }
    LogAudioPlayer ("AudioPlayer::handleAsyncUpdate: voice ended");
    audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
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
    fadeLength = juce::jmax (1, juce::roundToInt (kDeclickFadeSeconds * sampleRate));
    prepareSampleForPlayback ();
    updateWindow ();
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

    // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
    juce::ScopedLock sl (dataCS);
    if (voiceState == VoiceState::idle || sampleBuffer == nullptr)
    {
        auditionPlayhead.position.store (-1.0);
        return;
    }

    auto& outputBuffer { *bufferToFill.buffer };
    const auto channels { juce::jmin (outputBuffer.getNumChannels (), sampleBuffer->getNumChannels ()) };
    const auto sampleBufferLength { sampleBuffer->getNumSamples () };
    for (auto frame { 0 }; frame < bufferToFill.numSamples && voiceState != VoiceState::idle; ++frame)
    {
        // Gated Play Mode ends the voice when the gate goes low. It is looked at every frame, as the module does,
        // so changing to Gated while the gate is low ends a 1 Shot voice that is still running.
        if (voiceState == VoiceState::playing && playMode == kPlayModeGated && ! gateHigh)
            startFadeOut ();

        const auto readIndex { static_cast<int> (playPosition) };
        if (readIndex >= 0 && readIndex < sampleBufferLength)
        {
            const auto fraction { static_cast<float> (playPosition - readIndex) };
            const auto nextReadIndex { juce::jmin (readIndex + 1, sampleBufferLength - 1) };
            const auto gain { voiceState == VoiceState::fadingOut ? static_cast<float> (fadeSamplesRemaining) / static_cast<float> (fadeLength) : 1.0f };
            for (auto ch { 0 }; ch < channels; ++ch)
            {
                const auto* sampleData { sampleBuffer->getReadPointer (ch) };
                const auto sampleValue { sampleData [readIndex] + fraction * (sampleData [nextReadIndex] - sampleData [readIndex]) };
                outputBuffer.setSample (ch, bufferToFill.startSample + frame, sampleValue * gain);
            }
        }

        if (voiceState == VoiceState::fadingOut && --fadeSamplesRemaining <= 0)
        {
            voiceState = VoiceState::idle;
            triggerAsyncUpdate ();
            break;
        }

        playPosition += 1.0;
        wrapAndTestForEnd ();
    }
    // the displays draw it in frames of the sample, as they do the markers
    auditionPlayhead.position.store (voiceState == VoiceState::idle || sampleRateRatio <= 0.0 ? -1.0 : playPosition / sampleRateRatio);
}
