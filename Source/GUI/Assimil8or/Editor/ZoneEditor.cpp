#include "ZoneEditor.h"
#include "FormatHelpers.h"
#include "ParameterToolTipData.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/DumpStack.h"
#include "../../../Utility/ErrorHelpers.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

ZoneEditor::ZoneEditor ()
{
    audioFormatManager.registerBasicFormats ();
    {
        PresetProperties minPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MinParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        ChannelProperties minChannelProperties (minPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        minZoneProperties.wrap (minChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }
    {
        PresetProperties maxPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        ChannelProperties maxChannelProperties (maxPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        maxZoneProperties.wrap (maxChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }

    auto setupLabel = [this] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        const auto textColor { juce::Colours::black };
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setColour (juce::Label::ColourIds::textColourId, textColor);
        label.setFont (label.getFont ().withHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };

    sampleNameSelectLabel.onFilesSelected = [this] (const juce::StringArray& files)
    {
        if (! handleSamplesInternal (zoneProperties.getId () - 1, files))
        {
            // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
        }
    };

    toolsButton.setButtonText ("TOOLS");
    toolsButton.onClick = [this] ()
    {
        if (displayToolsMenu != nullptr)
            displayToolsMenu (zoneProperties.getId () - 1);
    };
    addAndMakeVisible (toolsButton);

    addAndMakeVisible (loopPointsView);

    setupLabel (sourceLabel, "SOURCE", 14.0f, juce::Justification::centred);
    addAndMakeVisible (sourceLabel);
    auto setupSourceButton = [this] (juce::TextButton& sourceButton, juce::String text, bool initilalState, juce::Rectangle<int>* background,
                                     std::function<void ()> ifStoppedFunc)
    {
        jassert (ifStoppedFunc != nullptr);
        jassert (background != nullptr);
        sourceButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
        sourceButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, sourceSamplePointsButton.findColour (juce::TextButton::ColourIds::buttonColourId).brighter (0.3f));
        sourceButton.setToggleState (initilalState, juce::NotificationType::dontSendNotification);
        sourceButton.setButtonText (text);
        sourceButton.onClick = [this, ifStoppedFunc, background, &sourceButton] ()
        {
            activePointBackground = background;
            sourceSamplePointsButton.setToggleState (&sourceButton == &sourceSamplePointsButton, juce::NotificationType::dontSendNotification);
            sourceLoopPointsButton.setToggleState (&sourceButton == &sourceLoopPointsButton, juce::NotificationType::dontSendNotification);
            updateLoopPointsView ();
            repaint ();
            if (audioPlayerProperties.getPlayState () != AudioPlayerProperties::PlayState::stop)
                ifStoppedFunc ();
        };
        addAndMakeVisible (sourceButton);
    };
    setupSourceButton (sourceSamplePointsButton, "SMPL", true, &samplePointsBackground, [this] ()
    {
        const auto sampleStart { static_cast<int> (zoneProperties.getSampleStart ().value_or (0)) };
        audioPlayerProperties.setLoopStart (sampleStart, false);
        audioPlayerProperties.setLoopLength (static_cast<int> (zoneProperties.getSampleEnd ().value_or (sampleData.getLengthInSamples ()) - sampleStart), false);
    });

    setupSourceButton (sourceLoopPointsButton, "LOOP", false, &loopPointsBackground, [this] ()
    {
        const auto loopStart { static_cast<int> (zoneProperties.getLoopStart ().value_or (0)) };
        audioPlayerProperties.setLoopStart (loopStart, false);
        audioPlayerProperties.setLoopLength (static_cast<int> (zoneProperties.getLoopLength ().value_or (sampleData.getLengthInSamples ())), false);
    });

    setupLabel (playModeLabel, "PLAY", 14.0f, juce::Justification::centred);
    addAndMakeVisible (playModeLabel);
    auto setupPlayButton = [this] (juce::TextButton& playButton, juce::String text, bool initilalEnabledState, juce::String otherButtonText,
                                   juce::TextButton& sourceButton, AudioPlayerProperties::PlayState playState,
                                   std::function<void ()> startPlayFunction, std::function<void ()> switchPlayFunction)
    {
        playButton.setButtonText (text);
        playButton.setEnabled (initilalEnabledState);
        playButton.onClick = [this, text, &playButton, &sourceButton, startPlayFunction, switchPlayFunction, playState, otherButtonText] ()
        {
            if (playButton.getButtonText () == "STOP")
            {
                // stopping
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
                playButton.setButtonText (text);
            }
            else
            {
                audioPlayerProperties.setSourceFile (juce::File (appProperties.getMostRecentFolder ()).getChildFile (zoneProperties.getSample ()).getFullPathName (), false);
                if (sourceButton.getToggleState ())
                {
                    // starting
                    startPlayFunction ();
                }
                else
                {
                    // switching play states
                    switchPlayFunction ();
                }
                audioPlayerProperties.setPlayState (playState, false);
                playButton.setButtonText ("STOP");
                if (&playButton == &oneShotPlayButton)
                    loopPlayButton.setButtonText (otherButtonText);
                else
                    oneShotPlayButton.setButtonText (otherButtonText);
            }
        };
        addAndMakeVisible (playButton);
    };
    setupPlayButton (loopPlayButton, "LOOP", false, "ONCE", sourceLoopPointsButton, AudioPlayerProperties::PlayState::loop,
    [this] ()
    {
        const auto loopStart { static_cast<int> (zoneProperties.getLoopStart ().value_or (0)) };
        audioPlayerProperties.setLoopStart (loopStart, false);
        audioPlayerProperties.setLoopLength (static_cast<int> (zoneProperties.getLoopLength ().value_or (sampleData.getLengthInSamples ())), false);
    },
    [this] ()
    {
        const auto sampleStart { static_cast<int> (zoneProperties.getSampleStart ().value_or (0)) };
        audioPlayerProperties.setLoopStart (sampleStart, false);
        audioPlayerProperties.setLoopLength (static_cast<int> (zoneProperties.getSampleEnd ().value_or (sampleData.getLengthInSamples ()) - sampleStart), false);
    });

    setupPlayButton (oneShotPlayButton, "ONCE", false, "LOOP", sourceSamplePointsButton, AudioPlayerProperties::PlayState::play,
    [this] ()
    {
        const auto sampleStart { static_cast<int> (zoneProperties.getSampleStart ().value_or (0)) };
        audioPlayerProperties.setLoopStart (sampleStart, false);
        audioPlayerProperties.setLoopLength (static_cast<int> (zoneProperties.getSampleEnd ().value_or (sampleData.getLengthInSamples ()) - sampleStart), false);
    },
    [this] ()
    {
        const auto loopStart { static_cast<int> (zoneProperties.getLoopStart ().value_or (0)) };
        audioPlayerProperties.setLoopStart (loopStart, false);
        audioPlayerProperties.setLoopLength (static_cast<int> (zoneProperties.getLoopLength ().value_or (sampleData.getLengthInSamples ())), false);
    });
    setupZoneComponents ();
}

bool ZoneEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto file : files)
        if (! isSupportedAudioFile (file))
            return false;

    return true;
}

void ZoneEditor::setDropIndex (const juce::StringArray& files, int x, int y)
{
//       dropIndex = -1;
    if (files.size () == 1)
        dropIndex = -1;
    else if (getLocalBounds ().removeFromTop (getLocalBounds ().getHeight () / 2).contains (x, y))
        dropIndex = 0;
    else
        dropIndex = 1;
}

void ZoneEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    setDropIndex (files, x, y);
    draggingFiles = false;
    repaint ();
    if (! handleSamplesInternal (dropIndex == 0 ? 0 : zoneProperties.getId () - 1, files))
    {
        // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
    }
}

void ZoneEditor::fileDragEnter (const juce::StringArray& files, int x, int y)
{
    setDropIndex (files, x, y);
    draggingFiles = true;
    repaint ();
}

void ZoneEditor::fileDragMove (const juce::StringArray& files, int x, int y)
{
    setDropIndex (files, x, y);
    repaint ();
}

void ZoneEditor::fileDragExit (const juce::StringArray&)
{
    draggingFiles = false;
    repaint ();
}

bool ZoneEditor::handleSamplesInternal (int zoneIndex, juce::StringArray files)
{
    jassert (assignSamples != nullptr);
    audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
    files.sort (true);
    return assignSamples (zoneIndex, files);
}

void ZoneEditor::updateLoopPointsView ()
{
    juce::int64 startSample { 0 };
    juce::int64 numSamples { 0 };
    if (sourceSamplePointsButton.getToggleState ())
    {
        startSample = zoneProperties.getSampleStart ().value_or (0);
        numSamples = zoneProperties.getSampleEnd ().value_or (sampleData.getLengthInSamples ()) - startSample;
    }
    else
    {
        startSample = zoneProperties.getLoopStart ().value_or (0);
        numSamples = static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleData.getLengthInSamples ())));
    }
    loopPointsView.setAudioBuffer (sampleData.getAudioBuffer ());
    loopPointsView.setLoopPoints (startSample, numSamples);
    loopPointsView.repaint ();
}

void ZoneEditor::loadSample (juce::String sampleFileName)
{
    if (sampleFileName == currentSampleFileName)
        return;

    currentSampleFileName = sampleFileName;

    if (sampleFileName.isNotEmpty ())
        sampleData = samplePool->open (sampleFileName);
    else
        sampleData = SampleData ();

    if (sampleData.getStatus () == SampleData::SampleDataStatus::exists)
    {
        updateLoopPointsView ();

        zoneProperties.setSampleStart (-1, true);
        zoneProperties.setSampleEnd (-1, true);
        zoneProperties.setLoopStart (-1, true);
        zoneProperties.setLoopLength (-1, true);
        updateSamplePositionInfo ();
        sampleUiChanged (sampleFileName);
        sampleNameSelectLabel.setText (sampleFileName, juce::NotificationType::dontSendNotification);
        sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);

        if (sampleData.getNumChannels () == 2)
        {
            ChannelProperties parentChannelProperties (zoneProperties.getValueTree ().getParent (), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            // if this zone not the last channel && the parent channel isn't set to Stereo/Right
            if (auto parentChannelId { parentChannelProperties.getId () }; parentChannelId < 8 && parentChannelProperties.getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
            {
                PresetProperties presetProperties (parentChannelProperties.getValueTree ().getParent (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
                // NOTE PresetProperties.getChannelVT takes a 0 based index, but Id's are 1 based. and since we want the NEXT channel, we can use the Id, because it is already +1 to the index
                ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelId), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                ZoneProperties nextChannelZone1Properties (nextChannelProperties.getZoneVT (zoneProperties.getId () - 1), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                // if next Channel does not have a sample
                if (nextChannelZone1Properties.getSample ().isEmpty ())
                {
                    nextChannelProperties.setChannelMode (ChannelProperties::ChannelMode::stereoRight, false);
                    nextChannelZone1Properties.setSide (1, false);
                    nextChannelZone1Properties.setSampleStart (-1, true);
                    nextChannelZone1Properties.setSampleEnd (-1, true);
                    nextChannelZone1Properties.setLoopStart (-1, true);
                    nextChannelZone1Properties.setLoopLength (-1, true);
                    nextChannelZone1Properties.setSample (sampleFileName, false); // when the other editor receives this update, it will also update the sample positions, so do it after setting them
                }
            }
        }
    }

    const auto sampleCanBePlayed { sampleData.getStatus () == SampleData::SampleDataStatus::exists };
    oneShotPlayButton.setEnabled (sampleCanBePlayed);
    loopPlayButton.setEnabled (sampleCanBePlayed);

    if (onSampleChange != nullptr)
        onSampleChange (sampleFileName);
}

void ZoneEditor::setupZoneComponents ()
{
    juce::XmlDocument xmlDoc { BinaryData::Assimil8orToolTips_xml };
    auto xmlElement { xmlDoc.getDocumentElement (false) };
//     if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
//         juce::Logger::outputDebugString ("XML Parsing Error for Assimil8orToolTips_xml: " + parseError);
    // NOTE: this is a hard failure, which indicates there is a problem in the file the parameterPresetXml passed in
    jassert (xmlDoc.getLastParseError () == "");
    auto toolTipsVT { juce::ValueTree::fromXml (*xmlElement) };
    ParameterToolTipData parameterToolTipData (toolTipsVT, ParameterToolTipData::WrapperType::owner, ParameterToolTipData::EnableCallbacks::no);

    auto setupLabel = [this] (juce::Label& label, juce::String text, float fontSize, juce::Justification justification)
    {
        const auto textColor { juce::Colours::black };
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setColour (juce::Label::ColourIds::textColourId, textColor);
        label.setFont (label.getFont ().withHeight (fontSize));
        label.setText (text, juce::NotificationType::dontSendNotification);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this, &parameterToolTipData] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters,
                                                          juce::String parameterName)
    {
        textEditor.setJustification (justification);
        textEditor.setIndents (2, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.setTooltip (parameterToolTipData.getToolTip ("Zone", parameterName));
        addAndMakeVisible (textEditor);
    };

    // SAMPLE FILE SELECTOR
    setupLabel (sampleNameLabel, "FILE", 15.0, juce::Justification::centredLeft);
    setupLabel (sampleNameSelectLabel, "", 15.0, juce::Justification::centredLeft);
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::textColourId));
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::backgroundColourId, levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::backgroundColourId));
    sampleNameSelectLabel.setOutline (levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::outlineColourId));
    sampleNameSelectLabel.setBorderSize ({ 0, 2, 0, 0 });

    // SAMPLE START
    setupLabel (sampleStartLabel, "SMPL START", 12.0, juce::Justification::centredRight);
    sampleStartTextEditor.getMinValueCallback = [this] { return minZoneProperties.getSampleStart ().value_or (0); };
    sampleStartTextEditor.getMaxValueCallback = [this] { return zoneProperties.getSampleEnd ().value_or (sampleData.getLengthInSamples ()); };
    sampleStartTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    sampleStartTextEditor.updateDataCallback = [this] (juce::int64 value)
    {
        sampleStartUiChanged (value);
        if (sourceSamplePointsButton.getToggleState ())
            audioPlayerProperties.setLoopStart (static_cast<int> (value), false);
    };
    sampleStartTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    sampleStartTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (sampleStartTextEditor, juce::Justification::centred, 0, "0123456789", "SampleStart");

    // SAMPLE END
    setupLabel (sampleEndLabel, "SMPL END", 12.0, juce::Justification::centredRight);
    sampleEndTextEditor.getMinValueCallback = [this] { return zoneProperties.getSampleStart ().value_or (0); };
    sampleEndTextEditor.getMaxValueCallback = [this] { return sampleData.getLengthInSamples (); };
    sampleEndTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    sampleEndTextEditor.updateDataCallback = [this] (juce::int64 value)
    {
        sampleEndUiChanged (value);
        if (sourceSamplePointsButton.getToggleState ())
            audioPlayerProperties.setLoopLength (static_cast<int> (value - zoneProperties.getSampleStart ().value_or (0)), false);
    };
    sampleEndTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    sampleEndTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (sampleEndTextEditor, juce::Justification::centred, 0, "0123456789", "SampleEnd");

    // LOOP START
    setupLabel (loopStartLabel, "LOOP START", 12.0, juce::Justification::centredRight);
    loopStartTextEditor.getMinValueCallback = [this] { return minZoneProperties.getLoopStart ().value_or (0); };
    loopStartTextEditor.getMaxValueCallback = [this]
    {
        if (! treatLoopLengthAsEndInUi)
        {
            const auto sampleLength { sampleData.getLengthInSamples () };
            const auto loopLength { zoneProperties.getLoopLength ().value_or (sampleLength)};
            //DebugLog ("loopStartTextEditor.getMaxValueCallback (loopLength)", "sampleLength: " + juce::String(sampleLength) + ", loopLength: " + juce::String (loopLength));
            return sampleData.getLengthInSamples () < 4 ? 0 : static_cast<juce::int64> (static_cast<double> (sampleData.getLengthInSamples ()) -
                                                                  zoneProperties.getLoopLength ().value_or (static_cast<double>(sampleData.getLengthInSamples ())));
        }
        else
        {
            const auto sampleLength { sampleData.getLengthInSamples () };
            const auto loopStart { loopStartTextEditor.getText().getLargeIntValue () };
            const auto loopLength { static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (minZoneProperties.getLoopLength ().value ())) };
            const auto loopEnd { std::min (loopStart + loopLength, sampleData.getLengthInSamples () - static_cast<juce::int64> (minZoneProperties.getLoopLength ().value ())) };
//             DebugLog ("loopStartTextEditor.getMaxValueCallback (loopEnd)", "sampleLength: " + juce::String (sampleLength) + ", loopStart: " + juce::String (loopStart) +
//                       ", loopLength: " + juce::String (loopLength) + ", loopEnd: " + juce::String (loopEnd));
            return loopEnd;
        }
    };
    loopStartTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    loopStartTextEditor.updateDataCallback = [this] (juce::int64 value)
    {
        loopStartUiChanged (value);
        if (sourceLoopPointsButton.getToggleState ())
            audioPlayerProperties.setLoopStart (static_cast<int> (value), false);
        if (treatLoopLengthAsEndInUi)
        {
            // Calculate Loop Length value and store it
            const auto loopLength { loopLengthTextEditor.getText ().getDoubleValue () - static_cast<double> (zoneProperties.getLoopStart ().value_or (0)) };
            loopLengthUiChanged (loopLength);
            if (sourceLoopPointsButton.getToggleState ())
                audioPlayerProperties.setLoopLength (static_cast<int> (loopLength), false);
        }
    };
    loopStartTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    loopStartTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (loopStartTextEditor, juce::Justification::centred, 0, "0123456789", "LoopStart");

    // LOOP LENGTH/END
    setupLabel (loopLengthLabel, "LOOP LENGTH", 12.0, juce::Justification::centredRight);
// TODO - NEED TO FINISH THIS ONE. DOES IT REQUIRE MORE CHANGES TO THE CustomTextEditor base class?
    loopLengthTextEditor.getMinValueCallback = [this]
    {
        if (treatLoopLengthAsEndInUi)
            return sampleData.getLengthInSamples () == 0 ? 0.0 : zoneProperties.getLoopStart().value_or (0.0) + minZoneProperties.getLoopLength ().value ();
        else
            return sampleData.getLengthInSamples () == 0 ? 0.0 : minZoneProperties.getLoopLength ().value ();
    };
    loopLengthTextEditor.getMaxValueCallback = [this]
    {
        if (treatLoopLengthAsEndInUi)
            return static_cast<double> (sampleData.getLengthInSamples ());
        else
            return static_cast<double> (sampleData.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0));
    };
    loopLengthTextEditor.snapValueCallback = [this] (double value) { return snapLoopLength (value); };
    loopLengthTextEditor.toStringCallback = [this] (double value)
    {
        auto loopLengthInputValue = [this, value] ()
        {
            if (treatLoopLengthAsEndInUi)
                return value - zoneProperties.getLoopStart ().value_or (0.0);
            else
                return value;
        } ();
        return formatLoopLength (loopLengthInputValue);
    };
    loopLengthTextEditor.updateDataCallback = [this] (double value)
    {
        auto loopLengthInputValue = [this, value] ()
        {
            if (treatLoopLengthAsEndInUi)
                return value - zoneProperties.getLoopStart ().value_or (0.0);
            else
                return value;
        } ();
        loopLengthUiChanged (loopLengthInputValue);
        if (sourceLoopPointsButton.getToggleState ())
            audioPlayerProperties.setLoopLength (static_cast<int> (loopLengthInputValue), false);
    };
    loopLengthTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    loopLengthTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (loopLengthTextEditor, juce::Justification::centred, 0, ".0123456789", "LoopLength");

    // MIN VOLTAGE
    setupLabel (minVoltageLabel, "MIN VOLTAGE", 15.0, juce::Justification::centredRight);
    minVoltageTextEditor.getMinValueCallback = [this] { return minZoneProperties.getMinVoltage (); };
    minVoltageTextEditor.getMaxValueCallback = [this] { return maxZoneProperties.getMinVoltage (); };
    minVoltageTextEditor.highlightErrorCallback = [this] ()
    {
        jassert (isMinVoltageInRange != nullptr);
        ErrorHelpers::setColorIfError (minVoltageTextEditor, isMinVoltageInRange (minVoltageTextEditor.getText ().getDoubleValue ()));
    };
    minVoltageTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    minVoltageTextEditor.updateDataCallback = [this] (double value)
    {
        // TODO can this extra call to clampMinVoltage somehow be covered by the snapValueCallback in CustomTextEditor
        jassert (clampMinVoltage != nullptr);
        minVoltageUiChanged (clampMinVoltage (value));
    };
    minVoltageTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    minVoltageTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (minVoltageTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MinVoltage");

    // PITCH OFFSET
    pitchOffsetTextEditor.getMinValueCallback = [this] { return minZoneProperties.getPitchOffset (); };
    pitchOffsetTextEditor.getMaxValueCallback = [this] { return maxZoneProperties.getPitchOffset (); };
    pitchOffsetTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    pitchOffsetTextEditor.updateDataCallback = [this] (double value) { pitchOffsetUiChanged (value); };
    pitchOffsetTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    pitchOffsetTextEditor.onPopupMenuCallback = [this] () {};
    setupLabel (pitchOffsetLabel, "PITCH OFFSET", 15.0, juce::Justification::centredRight);
    setupTextEditor (pitchOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PitchOffset");

    // LEVEL OFFSET
    setupLabel (levelOffsetLabel, "LEVEL OFFSET", 15.0, juce::Justification::centredRight);
    levelOffsetTextEditor.getMinValueCallback = [this] { return minZoneProperties.getLevelOffset (); };
    levelOffsetTextEditor.getMaxValueCallback = [this] { return maxZoneProperties.getLevelOffset (); };
    levelOffsetTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, true); };
    levelOffsetTextEditor.updateDataCallback = [this] (double value) { levelOffsetUiChanged (value); };
    levelOffsetTextEditor.onDragCallback = [this] (DragSpeed dragSpeed, int direction) {};
    levelOffsetTextEditor.onPopupMenuCallback = [this] () {};
    setupTextEditor (levelOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LevelOffset");
}

void ZoneEditor::init (juce::ValueTree zonePropertiesVT, juce::ValueTree rootPropertiesVT, SamplePool* theSamplePool)
{
    //DebugLog ("ZoneEditor[" + juce::String (zoneProperties.getId ()) + "]", "init");
    jassert (theSamplePool != nullptr);
    samplePool = theSamplePool;

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
    audioPlayerProperties.wrap (runtimeRootProperties.getValueTree (), AudioPlayerProperties::WrapperType::client, AudioPlayerProperties::EnableCallbacks::yes);
    audioPlayerProperties.onPlayStateChange = [this] (AudioPlayerProperties::PlayState playState)
    {
        if (playState == AudioPlayerProperties::PlayState::stop)
        {
            juce::MessageManager::callAsync ([this] ()
            {
                oneShotPlayButton.setButtonText ("ONCE");
                loopPlayButton.setButtonText ("LOOP");
            });
        }
        else if (playState == AudioPlayerProperties::PlayState::play)
        {
            juce::MessageManager::callAsync ([this] ()
            {
                oneShotPlayButton.setButtonText ("STOP");
                loopPlayButton.setButtonText ("LOOP");
            });
        }
        else if (playState == AudioPlayerProperties::PlayState::loop)
        {
            juce::MessageManager::callAsync ([this] ()
            {
                oneShotPlayButton.setButtonText ("ONCE");
                loopPlayButton.setButtonText ("STOP");
            });
        }
        else
        {
            jassertfalse;
        }
    };

    zoneProperties.wrap (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::yes);
    setupZonePropertiesCallbacks ();

    levelOffsetDataChanged (zoneProperties.getLevelOffset ());
    loopLengthDataChanged (zoneProperties.getLoopLength ());
    loopStartDataChanged (zoneProperties.getLoopStart ());
    minVoltageDataChanged (zoneProperties.getMinVoltage ());
    pitchOffsetDataChanged (zoneProperties.getPitchOffset ());
    sampleDataChanged (zoneProperties.getSample ());
    sampleStartDataChanged (zoneProperties.getSampleStart ());
    sampleEndDataChanged (zoneProperties.getSampleEnd ());

    ChannelProperties channelProperties (zoneProperties.getValueTree ().getParent (), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    setLoopLengthIsEnd (channelProperties.getLoopLengthIsEnd ());
}

void ZoneEditor::setLoopLengthIsEnd (bool newLoopLengthIsEnd)
{
    treatLoopLengthAsEndInUi = newLoopLengthIsEnd;
    if (treatLoopLengthAsEndInUi)
    {
        loopLengthLabel.setText ("LOOP END", juce::NotificationType::dontSendNotification);
        loopLengthTextEditor.setInputRestrictions (0, "0123456789");
    }
    else
    {
        loopLengthLabel.setText ("LOOP LENGTH", juce::NotificationType::dontSendNotification);
        loopLengthTextEditor.setInputRestrictions (0, ".0123456789");
    }
    // reformat the UI string
    loopLengthDataChanged (zoneProperties.getLoopLength ());
}

void ZoneEditor::receiveSampleLoadRequest (juce::File sampleFile)
{
    if (! handleSamplesInternal (zoneProperties.getId () - 1, { sampleFile.getFullPathName () }))
    {
        // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
    }
}

void ZoneEditor::setupZonePropertiesCallbacks ()
{
    zoneProperties.onIdChange = [this] ([[maybe_unused]] int id) { jassertfalse; /* I don't think this should change while we are editing */};
    zoneProperties.onLevelOffsetChange = [this] (double levelOffset) { levelOffsetDataChanged (levelOffset);  };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double> loopLength) { loopLengthDataChanged (loopLength);  };
    zoneProperties.onLoopStartChange = [this] (std::optional <juce::int64> loopStart) { loopStartDataChanged (loopStart);  };
    zoneProperties.onMinVoltageChange = [this] (double minVoltage) { minVoltageDataChanged (minVoltage);  };
    zoneProperties.onPitchOffsetChange = [this] (double pitchOffset) { pitchOffsetDataChanged (pitchOffset);  };
    zoneProperties.onSampleChange = [this] (juce::String sample) { sampleDataChanged (sample);  };
    zoneProperties.onSampleStartChange = [this] (std::optional <juce::int64> sampleStart) { sampleStartDataChanged (sampleStart);  };
    zoneProperties.onSampleEndChange = [this] (std::optional <juce::int64> sampleEnd) { sampleEndDataChanged (sampleEnd);  };
}

void ZoneEditor::paint ([[maybe_unused]] juce::Graphics& g)
{
//     g.setColour (juce::Colours::magenta);
//     g.drawRect (getLocalBounds ());
    g.setColour (juce::Colours::grey.withAlpha (0.3f));
    g.fillRoundedRectangle (activePointBackground->toFloat (), 0.5f);
}

void ZoneEditor::paintOverChildren (juce::Graphics& g)
{
    juce::Colour fillColor { juce::Colours::white };
    float activeAlpha { 0.7f };
    float nonActiveAlpha { 0.2f };
    if (draggingFiles)
    {
        auto localBounds { getLocalBounds () };
        if (dropIndex == -1 || zoneProperties.getId () == 1)
        {
            g.setColour (fillColor.withAlpha (activeAlpha));
            g.fillRect (localBounds);
            g.setFont (20.0f);
            g.setColour (juce::Colours::black);
            g.drawText ("Start on Zone " + juce::String (zoneProperties.getId ()), localBounds, juce::Justification::centred, false);
        }
        else
        {
            g.setColour (fillColor.withAlpha (dropIndex == 0 ? activeAlpha : nonActiveAlpha));
            const auto topHalfBounds { localBounds.removeFromTop (localBounds.getHeight () / 2) };
            g.fillRect (topHalfBounds);
            g.setColour (fillColor.withAlpha (dropIndex == 1 ? activeAlpha : nonActiveAlpha));
            g.fillRect (localBounds);

            g.setFont (20.0f);
            g.setColour (juce::Colours::black);
            if (dropIndex == 0)
                g.drawText ("Start on Zone 1", topHalfBounds, juce::Justification::centred, false);
            else
                g.drawText ("Start on Zone " + juce::String (zoneProperties.getId ()), localBounds, juce::Justification::centred, false);
        }
    }
}

void ZoneEditor::resized ()
{
    const auto xOffset { 3 };
    const auto width { 160 };
    const auto interParameterYOffset { 1 };
    const auto spaceBetweenLabelAndInput { 3 };
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    if (displayToolsMenu != nullptr)
        toolsButton.setBounds (getWidth () - 5 - 40, getHeight () - 5 - 20, 40, 20);

    const auto sampleNameLabelScale { 0.156f };
    const auto sampleNameInputScale { 1.f - sampleNameLabelScale };
    sampleNameLabel.setBounds (xOffset, 5, scaleWidth (sampleNameLabelScale), 20);
    sampleNameSelectLabel.setBounds (sampleNameLabel.getRight () + spaceBetweenLabelAndInput, 5, scaleWidth (sampleNameInputScale) - spaceBetweenLabelAndInput + 1, 20);

    const auto loopPointsViewHeight { 40 };
    const auto samplePointLabelScale { 0.45f };
    const auto samplePointInputScale { 1.f - samplePointLabelScale };
    sampleStartLabel.setBounds (xOffset, sampleNameSelectLabel.getBottom () + 5, scaleWidth (samplePointLabelScale), 20);
    sampleStartTextEditor.setBounds (sampleStartLabel.getRight () + spaceBetweenLabelAndInput, sampleStartLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    sampleEndLabel.setBounds (xOffset, sampleStartLabel.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    sampleEndTextEditor.setBounds (sampleEndLabel.getRight () + spaceBetweenLabelAndInput, sampleEndLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    samplePointsBackground = { sampleStartLabel.getX (), sampleStartLabel.getY (),
                               sampleEndTextEditor.getRight () - sampleStartLabel.getX (),
                               sampleEndTextEditor.getBottom () - sampleStartLabel.getY () + loopPointsViewHeight };

    auto loopPointsViewBounds { juce::Rectangle<int> {0, sampleEndTextEditor.getBottom () + interParameterYOffset, getWidth (), loopPointsViewHeight } };
    loopPointsView.setBounds (loopPointsViewBounds.reduced (3, 0));

    loopStartLabel.setBounds (xOffset, loopPointsView.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    loopStartTextEditor.setBounds (loopStartLabel.getRight () + spaceBetweenLabelAndInput, loopStartLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    loopLengthLabel.setBounds (xOffset, loopStartLabel.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    loopLengthTextEditor.setBounds (loopLengthLabel.getRight () + spaceBetweenLabelAndInput, loopLengthLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    loopPointsBackground = { loopStartLabel.getX (), loopStartLabel.getY () - loopPointsViewHeight,
                             loopLengthTextEditor.getRight () - loopStartLabel.getX (),
                             loopLengthTextEditor.getBottom () - loopStartLabel.getY () + loopPointsViewHeight };

    auto labelBounds { juce::Rectangle<int> {0, loopLengthTextEditor.getBottom () + interParameterYOffset, getWidth (), 14} };
    sourceLabel.setBounds (labelBounds.removeFromLeft (labelBounds.getWidth () / 2));
    playModeLabel.setBounds (labelBounds);
    auto controlsBounds { juce::Rectangle<int> {0, labelBounds.getBottom () + interParameterYOffset, getWidth (), 20} };
    controlsBounds.removeFromLeft (3);
    sourceSamplePointsButton.setBounds (controlsBounds.removeFromLeft (35));
    controlsBounds.removeFromLeft (3);
    sourceLoopPointsButton.setBounds (controlsBounds.removeFromLeft (35));
    controlsBounds.removeFromRight (3);
    loopPlayButton.setBounds (controlsBounds.removeFromRight (35));
    controlsBounds.removeFromRight (3);
    oneShotPlayButton.setBounds (controlsBounds.removeFromRight (35));

    const auto otherLabelScale { 0.66f };
    const auto otherInputScale { 1.f - otherLabelScale };
    minVoltageLabel.setBounds (xOffset, oneShotPlayButton.getBottom () + 5, scaleWidth (otherLabelScale), 20);
    minVoltageTextEditor.setBounds (minVoltageLabel.getRight () + spaceBetweenLabelAndInput, minVoltageLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);

    pitchOffsetLabel.setBounds (xOffset, minVoltageTextEditor.getBottom () + 3, scaleWidth (otherLabelScale), 20);
    pitchOffsetTextEditor.setBounds (pitchOffsetLabel.getRight () + spaceBetweenLabelAndInput, pitchOffsetLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);

    levelOffsetLabel.setBounds (xOffset, pitchOffsetLabel.getBottom () + 3, scaleWidth (otherLabelScale), 20);
    levelOffsetTextEditor.setBounds (levelOffsetLabel.getRight () + spaceBetweenLabelAndInput, levelOffsetLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);
}

void ZoneEditor::checkSampleExistence ()
{
    if (zoneProperties.getSample ().isNotEmpty ())
        updateSampleFileInfo (zoneProperties.getSample ());
}

double ZoneEditor::snapLoopLength (double rawValue)
{
    if (rawValue < 2048)
    {
        if (rawValue < 4.0)
            return 4.0;

        auto snapToResolution = [] (double number, double resolution) { return std::round (number / resolution) * resolution; };
        const auto wholeValue { static_cast<uint32_t> (rawValue) };
        const auto fractionalValue { rawValue - static_cast<double> (wholeValue) };

        auto getFractionalSize = [] (uint32_t wholeValue)
        {
            auto calculateFractionalSize = [] (int numberOfBits) { return 1.0 / (1 << numberOfBits); };
            if (wholeValue > 1024)
                return calculateFractionalSize (1);
            else if (wholeValue > 512)
                return calculateFractionalSize (2);
            else if (wholeValue > 256)
                return calculateFractionalSize (3);
            else if (wholeValue > 128)
                return calculateFractionalSize (4);
            else if (wholeValue >= 4)
                return calculateFractionalSize (5);
            else
            {
                jassertfalse;
                return 0.0;
            }
        };
        const auto snappedFractionalValue { snapToResolution (fractionalValue, getFractionalSize (wholeValue)) };
        return static_cast<double> (wholeValue) + snappedFractionalValue;
    }
    else
    {
        return static_cast<uint32_t> (rawValue);
    }
}

juce::String ZoneEditor::formatLoopLength (double loopLength)
{
    if (treatLoopLengthAsEndInUi)
        loopLength = static_cast<int> (static_cast<double> (zoneProperties.getLoopStart ().value_or (0)) + loopLength);

    // value >= 2048 - no decimals
    // value < 2048 - 3 decimal places
    // 1024.000 < value < 2047.000 - 0.500 increment
    //  512.000 < value < 1024.000 - 0.250 increment (0.25, 0.5, 0.75, 1.000)
    //  256.000 < value <  512.000 - 0.125 increment (0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.000)
    //  128.000 < value <  256.000 - 0.063 increment (0.063, 0.125, 0.188, 0.250, 0.313, 0.375, 0.438, 0.500, 0.563, 0.625, 0.688, 0.750, 0.813, 0.875, 0.938, 1.000)
    //    0.000 < value <  128.000 - 0.031 increment  (0.031, 0.063, 0.094, 0.125, 0.156, 0.188, 0.219, 0.250, 0.281, 0.313, 0.344, 0.375, 0.406, 0.438, 0.469, 0.500,
    //                                                 0.531, 0.563, 0.594, 0.625, 0.656, 0.688, 0.719, 0.750, 0.781, 0.813, 0.844, 0.875, 0.906, 0.938, 0.969, 1.000)

    if (loopLength < 2048.0)
        return FormatHelpers::formatDouble (loopLength, 3, false);
    else
        return juce::String (static_cast<int> (loopLength));
}

// TODO - this should be done in the Directory scan, and stored in a property
bool ZoneEditor::isSupportedAudioFile (juce::File file)
{
    if (file.isDirectory () || file.getFileExtension ().toLowerCase() != ".wav")
        return false;
    std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;
    // check for any format settings that are unsupported
    if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample < 8 || reader->bitsPerSample > 32) || (reader->numChannels == 0 || reader->numChannels > 2) || (reader->sampleRate > 192000))
        return false;

    return true;
}

void ZoneEditor::levelOffsetDataChanged (double levelOffset)
{
    levelOffsetTextEditor.setText (FormatHelpers::formatDouble (levelOffset, 1, true));
}

void ZoneEditor::levelOffsetUiChanged (double levelOffset)
{
    zoneProperties.setLevelOffset (levelOffset, false);
}

void ZoneEditor::loopLengthDataChanged (std::optional<double> loopLength)
{
    loopLengthTextEditor.setText (formatLoopLength (loopLength.value_or (static_cast<double> (sampleData.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0)))));
}

void ZoneEditor::loopLengthUiChanged (double loopLength)
{
    zoneProperties.setLoopLength (loopLength, false);
}

void ZoneEditor::loopStartDataChanged (std::optional<juce::int64> loopStart)
{
    loopStartTextEditor.setText (juce::String (loopStart.value_or (0)));
    loopLengthDataChanged (zoneProperties.getLoopLength ());
}

void ZoneEditor::loopStartUiChanged (juce::int64 loopStart)
{
    zoneProperties.setLoopStart (loopStart, false);
}

void ZoneEditor::minVoltageDataChanged (double minVoltage)
{
    minVoltageTextEditor.setText (FormatHelpers::formatDouble (minVoltage, 2, true));
}

void ZoneEditor::minVoltageUiChanged (double minVoltage)
{
    zoneProperties.setMinVoltage (minVoltage, false);
}

void ZoneEditor::pitchOffsetDataChanged (double pitchOffset)
{
    pitchOffsetTextEditor.setText (FormatHelpers::formatDouble (pitchOffset, 2, true));
}

void ZoneEditor::pitchOffsetUiChanged (double pitchOffset)
{
    zoneProperties.setPitchOffset (pitchOffset, false);
}

void ZoneEditor::updateSampleFileInfo (juce::String sample)
{
    jassert (! sample.isEmpty ());
    auto textColor {juce::Colours::white};
    if (sampleData.getStatus () == SampleData::SampleDataStatus::exists)
    {
        if (! zoneProperties.getSampleEnd ().has_value ())
            sampleEndTextEditor.setText (juce::String (sampleData.getLengthInSamples ()));
        if (! zoneProperties.getLoopLength ().has_value ())
            loopLengthTextEditor.setText (formatLoopLength (static_cast<double> (sampleData.getLengthInSamples ())));
    }
    else
    {
        textColor = juce::Colours::red;
    }
    loopPointsView.repaint ();
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, textColor);
}

void ZoneEditor::updateSamplePositionInfo ()
{
    loopLengthDataChanged (zoneProperties.getLoopLength ());
    loopStartDataChanged (zoneProperties.getLoopStart ());
    sampleStartDataChanged (zoneProperties.getSampleStart ());
    sampleEndDataChanged (zoneProperties.getSampleEnd ());
}

void ZoneEditor::sampleDataChanged (juce::String sample)
{
    //DebugLog ("ZoneEditor", "ZoneEditor[" + juce::String (zoneProperties.getId ()) + "]::sampleDataChanged: '" + sample + "'");

    if (sampleNameSelectLabel.getText ().isNotEmpty ())
        samplePool->close (sampleNameSelectLabel.getText ());
    loadSample (sample);

    const auto sampleCanBePlayed { sampleData.getStatus () == SampleData::SampleDataStatus::exists };
    oneShotPlayButton.setEnabled (sampleCanBePlayed);
    loopPlayButton.setEnabled (sampleCanBePlayed);

    if (sample != sampleNameSelectLabel.getText ())
    {
        updateLoopPointsView ();
        updateSamplePositionInfo ();
        if (sample.isNotEmpty ())
            updateSampleFileInfo (sample);
        sampleNameSelectLabel.setText (sample, juce::NotificationType::dontSendNotification);
    }
}

void ZoneEditor::sampleUiChanged (juce::String sample)
{
    zoneProperties.setSample (sample, false);
}

void ZoneEditor::sampleStartDataChanged (std::optional<juce::int64> sampleStart)
{
    sampleStartTextEditor.setText (juce::String (sampleStart.value_or (0)));
    updateLoopPointsView ();
}

void ZoneEditor::sampleStartUiChanged (juce::int64 sampleStart)
{
    zoneProperties.setSampleStart (sampleStart, false);
    updateLoopPointsView ();
}

void ZoneEditor::sampleEndDataChanged (std::optional<juce::int64> sampleEnd)
{
    sampleEndTextEditor.setText (juce::String (sampleEnd.value_or (sampleData.getLengthInSamples ())));
    updateLoopPointsView ();
}

void ZoneEditor::sampleEndUiChanged (juce::int64 sampleEnd)
{
    zoneProperties.setSampleEnd (sampleEnd, false);
    updateLoopPointsView ();
}
