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

AudioPlayer::AudioPlayer ()
{
    audioFormatManager.registerBasicFormats ();
}

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
    audioPlayerProperties.onSamplePointsSelectorChanged = [this] (AudioPlayerProperties::SamplePointsSelector samplePointsSelector)
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
    channelProperties.wrap (presetProperties.getChannelVT (channelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);

    zoneProperties.wrap (channelProperties.getZoneVT (zoneIndex), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (channelIndex, zoneIndex), SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::yes);

    zoneProperties.onSampleChange = [this] (juce::String sampleName)
    {
        LogAudioPlayer ("zoneProperties.onSampleChange");
        initSamplePoints ();
        prepareSampleForPlayback ();
    };
    zoneProperties.onSampleStartChange = [this] (std::optional<juce::int64> newSampleStart)
    {
        LogAudioPlayer ("zoneProperties.onSampleStartChange");
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::LoopPoints)
            return;
        sampleStart = static_cast<int> (newSampleStart.value_or (0) * sampleRateRatio);
        sampleLength = static_cast<int> ((zoneProperties.getSampleEnd().value_or (sampleProperties.getLengthInSamples ()) - zoneProperties.getSampleStart ().value_or (0)) * sampleRateRatio);
        if (curSampleOffset > sampleLength)
            curSampleOffset = 0;
        };
    zoneProperties.onSampleEndChange = [this] (std::optional <juce::int64> newSampleEnd)
    {
        LogAudioPlayer ("zoneProperties.onSampleEndChange");
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::LoopPoints)
            return;
        sampleLength = static_cast<int> ((newSampleEnd.value_or (sampleProperties.getLengthInSamples ()) - zoneProperties.getSampleStart ().value_or (0)) * sampleRateRatio);
        if (curSampleOffset > sampleLength)
            curSampleOffset = 0;
    };
    zoneProperties.onLoopStartChange = [this] (std::optional <juce::int64> newLoopStart)
    {
        LogAudioPlayer ("zoneProperties.onLoopStartChange");
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
            return;
        sampleStart = static_cast<int> (newLoopStart.value_or (0) * sampleRateRatio);
    };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double> newLoopLength)
    {
        LogAudioPlayer ("zoneProperties.onLoopLengthChange");
        juce::ScopedLock sl (dataCS);
        if (audioPlayerProperties.getSSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
            return;
        sampleLength = static_cast<int> (newLoopLength.value_or (sampleProperties.getLengthInSamples ()) * sampleRateRatio);
        if (curSampleOffset > sampleLength)
            curSampleOffset = 0;
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
        }
    };

    // setup local sample start and sample length based on samplePointsSource
    initSamplePoints ();

    // create local copy of audio data, with resampling if needed
    prepareSampleForPlayback ();

}

void AudioPlayer::initSamplePoints ()
{
    LogAudioPlayer ("initSamplePoints");
    juce::ScopedLock sl (dataCS);
    if (audioPlayerProperties.getSSamplePointsSelector () == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
    {
        LogAudioPlayer (" using SamplePoints");
        sampleStart = static_cast<int> (zoneProperties.getSampleStart ().value_or (0) * sampleRateRatio);
        sampleLength = static_cast<int> ((zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) - zoneProperties.getSampleStart ().value_or (0)) * sampleRateRatio);
        if (curSampleOffset > sampleLength)
            curSampleOffset = 0;
    }
    else
    {
        LogAudioPlayer (" using LoopPoints");
        sampleStart = static_cast<int> (zoneProperties.getLoopStart ().value_or (0) * sampleRateRatio);
        sampleLength = static_cast<int> (zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()) * sampleRateRatio);
        if (curSampleOffset > sampleLength)
            curSampleOffset = 0;
    }
    LogAudioPlayer (" sampleStart: " + juce::String (sampleStart) + ", sampleLength: " + juce::String (sampleLength));
}

void AudioPlayer::prepareSampleForPlayback ()
{
    if (zoneProperties.isValid () && sampleProperties.isValid () && sampleProperties.getStatus () == SampleStatus::exists)
    {
        LogAudioPlayer ("prepareSampleForPlayback: sample is ready");
        // TODO - get data from SampleProperties instead of opening the file again
        std::unique_ptr <juce::MemoryAudioSource> readerSource = std::make_unique<juce::MemoryAudioSource> (*sampleProperties.getAudioBufferPtr (), false, false);
        std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource = std::make_unique<juce::ResamplingAudioSource> (readerSource.get (), false, 2);
        sampleRateRatio = sampleRate / sampleProperties.getSampleRate ();
        resamplingAudioSource->setResamplingRatio (sampleProperties.getSampleRate () / sampleRate);
        resamplingAudioSource->prepareToPlay (blockSize, sampleRate);
        sampleBuffer = std::make_unique<juce::AudioBuffer<float>> (sampleProperties.getNumChannels (), static_cast<int> (sampleProperties.getLengthInSamples () * sampleRate / sampleProperties.getSampleRate ()));
        resamplingAudioSource->getNextAudioBlock (juce::AudioSourceChannelInfo (*sampleBuffer.get ()));
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
        curSampleOffset = 0;
    }
    else if (newPlayState == AudioPlayerProperties::PlayState::play)
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
        jassert (numSamplesToCopy >= 0);

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
    {
        // NOTE: I am using a lock in the audio callback ONLY BECAUSE the audio play back is a simple audition feature, not recording or performance playback
        juce::ScopedLock sl (dataCS);
        if (originalSampleOffset == curSampleOffset)
            curSampleOffset = cachedSampleOffset;
    }
}
