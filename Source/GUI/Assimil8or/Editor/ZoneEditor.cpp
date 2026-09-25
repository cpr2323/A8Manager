#include "ZoneEditor.h"
#include "FormatHelpers.h"
#include "../../Theme/A8ColourIds.h"
#include "ParameterToolTipData.h"
#include "SampleManager/SampleManagerProperties.h"
#include "../../../SystemServices.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "oolib/Debug/DebugLog.h"
#include "oolib/Debug/DumpStack.h"
#include "oolib/GUI/ErrorHelpers.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

#define INCLUDE_WAVE_MATCHING_LOOP_POINT_ALIGN 0
ZoneEditor::ZoneEditor ()
{
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

    auto setupLabel = [this] (juce::Label& label, juce::String text, [[maybe_unused]] float fontSize, juce::Justification justification)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setFont (A8Type::parameterLabel ());
        label.setText (text, juce::NotificationType::dontSendNotification);
        parameterLabels.push_back (&label);
        addAndMakeVisible (label);
    };

    toolsButton.setTooltip ("Zone Tools");
    toolsButton.onClick = [this] ()
    {
        jassert (displayToolsMenu != nullptr);
        displayToolsMenu (zoneProperties.getId () - 1);
    };
    addAndMakeVisible (toolsButton);

    loopPointsView.onPopupMenu = [this] (bool isStartSide) { showLoopPointsViewMenu (isStartSide); };
    addAndMakeVisible (loopPointsView);

    setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector::SamplePoints, true);

    auto setupPlayButton = [this] (juce::TextButton& playButton, juce::String text, juce::String otherButtonText,
                                   AudioPlayerProperties::PlayState playState)
    {
        playButton.setButtonText (text);
        playButton.setEnabled (false);
        playButton.onClick = [this, text, &playButton, playState, otherButtonText] ()
        {
            if (playButton.getButtonText () == "STOP")
            {
                // stopping
                audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, false);
                playButton.setButtonText (text);
            }
            else
            {
                audioPlayerProperties.setSampleSource (parentChannelIndex, zoneIndex, false);
                // starting
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
    loopPlayButton.setTooltip ("Plays the currently selected SOURCE in looping mode");
    setupPlayButton (loopPlayButton, "LOOP", "ONCE", AudioPlayerProperties::PlayState::loop);

    oneShotPlayButton.setTooltip ("Plays the currently selected SOURCE in one shot mode");
    setupPlayButton (oneShotPlayButton, "ONCE", "LOOP", AudioPlayerProperties::PlayState::play);
    setupZoneComponents ();
    setEditComponentsEnabled (false);
}

bool ZoneEditor::isInterestedInFileDrag ([[maybe_unused]] const juce::StringArray& files)
{
    // we do this check in the fileDragEnter and fileDragMove handlers, presenting more info regarding the drop operation
    return true;
}

void ZoneEditor::setDropIndex (const juce::StringArray& files, int x, int y)
{
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
    if (supportedFile && ! handleSamplesInternal (dropIndex == 0 ? 0 : zoneIndex, files))
    {
        // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
    }
    resetDropInfo ();
    repaint ();
}

void ZoneEditor::resetDropInfo ()
{
    draggingFilesCount = 0;
    dropMsg = {};
    dropDetails = {};
}

void ZoneEditor::updateDropInfo (const juce::StringArray& files)
{
    supportedFile = true;
    for (auto& fileName : files)
    {
        auto draggedFile { juce::File (fileName) };
        if (! audioManager->isA8ManagerSupportedAudioFile (draggedFile))
            supportedFile = false;
    }
}

void ZoneEditor::fileDragEnter (const juce::StringArray& files, int x, int y)
{
    draggingFilesCount = files.size ();
    updateDropInfo (files);
    setDropIndex (files, x, y);
    repaint ();
}

void ZoneEditor::fileDragMove (const juce::StringArray& files, int x, int y)
{
    const auto prevDropIndex= dropIndex;
    setDropIndex (files, x, y);
    if (prevDropIndex != dropIndex)
    {
        updateDropInfo (files);
        repaint ();
    }
    repaint ();
}

void ZoneEditor::fileDragExit (const juce::StringArray&)
{
    resetDropInfo ();
    repaint ();
}

// TODO - can we move this to the EditManager, as it just calls editManager->assignSamples (parentChannelIndex, startingZoneIndex, files); in the ZoneEditor
bool ZoneEditor::handleSamplesInternal (int startingZoneIndex, juce::StringArray files)
{
    audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
    files.sort (true);
    return editManager->assignSamples (parentChannelIndex, startingZoneIndex, files); // will end up calling ZoneProperties::setSample ()
}

void ZoneEditor::setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector newSamplePointsSelector, bool forceSetup)
{
    if (samplePointsSelector != newSamplePointsSelector || forceSetup)
    {
        samplePointsSelector = newSamplePointsSelector;
        activePointBackground = (samplePointsSelector == AudioPlayerProperties::SamplePointsSelector::SamplePoints ? &samplePointsBackground : &loopPointsBackground);
        updateLoopPointsView ();
        repaint ();
        audioPlayerProperties.setPlayState (AudioPlayerProperties::PlayState::stop, true);
        audioPlayerProperties.setSamplePointsSelector (samplePointsSelector, false);
        if (onActiveSamplePointsChanged != nullptr)
            onActiveSamplePointsChanged (samplePointsSelector);
    }
}

void ZoneEditor::selectSamplePoints (AudioPlayerProperties::SamplePointsSelector newSamplePointsSelector)
{
    setActiveSamplePoints (newSamplePointsSelector, false);
}

void ZoneEditor::updateLoopPointsView ()
{
    juce::int64 startSample { 0 };
    juce::int64 numSamples { 0 };
    int side { 0 };
    if (sampleProperties.getStatus () == SampleStatus::exists)
    {
        if (samplePointsSelector == AudioPlayerProperties::SamplePointsSelector::SamplePoints)
        {
            startSample = zoneProperties.getSampleStart ().value_or (0);
            numSamples = zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) - startSample;
        }
        else
        {
            startSample = zoneProperties.getLoopStart ().value_or (0);
            numSamples = static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleProperties.getLengthInSamples ())));
        }
        loopPointsView.setAudioBuffer (sampleProperties.getAudioBufferPtr ());
        side = zoneProperties.getSide ();
    }
    else
    {
        loopPointsView.setAudioBuffer (nullptr);
    }
    loopPointsView.setLoopPoints (startSample, numSamples, side);
    loopPointsView.repaint ();
}

juce::PopupMenu ZoneEditor::getSampleAdjustMenu (juce::PopupMenu adjustMenu, std::function<juce::int64 ()> getSampleOffset, std::function<juce::int64 ()> getMinSampleOffset,
                                                 std::function<juce::int64 ()>getMaxSampleOffset, std::function<void (juce::int64)> setSampleOffset)
{
    {
#if INCLUDE_WAVE_MATCHING_LOOP_POINT_ALIGN
        juce::PopupMenu adjustMenuOptions;
        {
            juce::PopupMenu zeroCrossingMenuOptions;
            zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this, getSampleOffset, getMinSampleOffset, setSampleOffset] ()
            {
                auto newSampleStart { audioManager->findPreviousZeroCrossing (getSampleOffset (), getMinSampleOffset (),
                                                                              *sampleProperties.getAudioBufferPtr (), zoneProperties.getSide ()) };
                if (newSampleStart != -1)
                    setSampleOffset (newSampleStart);
            });
            zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this, getSampleOffset, getMaxSampleOffset, setSampleOffset] ()
            {
                auto newSampleStart { audioManager->findNextZeroCrossing (getSampleOffset (), getMaxSampleOffset (),
                                                                          *sampleProperties.getAudioBufferPtr (), zoneProperties.getSide ()) };
                if (newSampleStart != -1)
                    setSampleOffset (newSampleStart);
            });
            adjustMenuOptions.addSubMenu ("Zero Crossing", zeroCrossingMenuOptions);
        }
        {
            juce::PopupMenu matchOtherMenuOptions;
            matchOtherMenuOptions.addItem ("Left  <<", true, false, [this, getSampleOffset, getMinSampleOffset, setSampleOffset] ()
            {
                auto newSampleStart { audioManager->findPreviousWaveMatching (getSampleOffset (), getMinSampleOffset (),
                                                                              *sampleProperties.getAudioBufferPtr (), zoneProperties.getSide ()) };
                if (newSampleStart != -1)
                    setSampleOffset (newSampleStart);
            });
            matchOtherMenuOptions.addItem ("Right >>", true, false, [this, getSampleOffset, getMaxSampleOffset, setSampleOffset] ()
            {
                auto newSampleStart { audioManager->findNextZeroWaveMatching (getSampleOffset (), getMaxSampleOffset (),
                                                                          *sampleProperties.getAudioBufferPtr (), zoneProperties.getSide ()) };
                if (newSampleStart != -1)
                    setSampleOffset (newSampleStart);
            });
            adjustMenuOptions.addSubMenu ("Match Other", matchOtherMenuOptions);
        }

        adjustMenu.addSubMenu ("Adjust", adjustMenuOptions);
#else
        juce::PopupMenu zeroCrossingMenuOptions;
        zeroCrossingMenuOptions.addItem ("Left  <<", true, false, [this, getSampleOffset, getMinSampleOffset, setSampleOffset] ()
                                         {
                                             auto newSampleStart { audioManager->findPreviousZeroCrossing (getSampleOffset (), getMinSampleOffset (),
                                                                                                           *sampleProperties.getAudioBufferPtr (), zoneProperties.getSide ()) };
                                             if (newSampleStart != -1)
                                                 setSampleOffset (newSampleStart);
                                         });
        zeroCrossingMenuOptions.addItem ("Right >>", true, false, [this, getSampleOffset, getMaxSampleOffset, setSampleOffset] ()
                                         {
                                             auto newSampleStart { audioManager->findNextZeroCrossing (getSampleOffset (), getMaxSampleOffset (),
                                                                                                       *sampleProperties.getAudioBufferPtr (), zoneProperties.getSide ()) };
                                             if (newSampleStart != -1)
                                                 setSampleOffset (newSampleStart);
                                         });
        adjustMenu.addSubMenu ("Zero Crossing", zeroCrossingMenuOptions);
#endif
    }
    return adjustMenu;
}

juce::PopupMenu ZoneEditor::createSamplePointAdjustMenu (SamplePoint samplePoint, juce::PopupMenu adjustMenu)
{
    switch (samplePoint)
    {
        case SamplePoint::sampleStart:
            return getSampleAdjustMenu (adjustMenu,
                                        [this] () { return zoneProperties.getSampleStart ().value_or (0); },
                                        [this] () { return juce::int64 { 0 }; },
                                        [this] () { return zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()); },
                                        [this] (juce::int64 sampleOffset) { zoneProperties.setSampleStart (sampleOffset, true); });
        case SamplePoint::sampleEnd:
            return getSampleAdjustMenu (adjustMenu,
                                        [this] () { return zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()); },
                                        [this] () { return zoneProperties.getSampleStart ().value_or (0); },
                                        [this] () { return sampleProperties.getLengthInSamples (); },
                                        [this] (juce::int64 sampleOffset) { zoneProperties.setSampleEnd (sampleOffset, true); });
        case SamplePoint::loopStart:
            return getSampleAdjustMenu (adjustMenu,
                                        [this] () { return zoneProperties.getLoopStart ().value_or (0); },
                                        [this] () { return minZoneProperties.getLoopStart ().value_or (0); },
                                        [this] () { return editManager->getMaxLoopStart (parentChannelIndex, zoneIndex); },
                                        // through the field, which holds the loop end still when Loop Length is shown as Loop End
                                        [this] (juce::int64 sampleOffset) { loopStartTextEditor.setValue (sampleOffset); });
        case SamplePoint::loopEnd:
        default:
            return getSampleAdjustMenu (adjustMenu,
                                        [this] () { return zoneProperties.getLoopStart ().value_or (0) + static_cast<juce::int64> (zoneProperties.getLoopLength ().value_or (4.)); },
                                        [this] () { return zoneProperties.getLoopStart ().value_or (0); },
                                        [this] () { return sampleProperties.getLengthInSamples (); },
                                        [this] (juce::int64 sampleOffset) { zoneProperties.setLoopLength (static_cast<double> (sampleOffset - zoneProperties.getLoopStart ().value_or (0.)), true); });
    }
}

// The loop tuner's two sides are the end (left) and the start (right) of whichever pair the zone is
// working with, so a right-click on either side offers that point's adjustments.
void ZoneEditor::showLoopPointsViewMenu (bool isStartSide)
{
    if (isStereoRightChannelMode || sampleProperties.getStatus () != SampleStatus::exists)
        return;

    const auto samplePointsActive { samplePointsSelector == AudioPlayerProperties::SamplePointsSelector::SamplePoints };
    const auto samplePoint { samplePointsActive ? (isStartSide ? SamplePoint::sampleStart : SamplePoint::sampleEnd)
                                                : (isStartSide ? SamplePoint::loopStart : SamplePoint::loopEnd) };
    const auto pointName = [this, samplePoint] () -> juce::String
    {
        switch (samplePoint)
        {
            case SamplePoint::sampleStart: return "SAMPLE START";
            case SamplePoint::sampleEnd: return "SAMPLE END";
            case SamplePoint::loopStart: return "LOOP START";
            case SamplePoint::loopEnd:
            default: return treatLoopLengthAsEndInUi ? "LOOP END" : "LOOP LENGTH";
        }
    } ();

    juce::PopupMenu menu;
    menu.addSectionHeader (pointName);
    menu.addSeparator ();
    createSamplePointAdjustMenu (samplePoint, menu).showMenuAsync ({});
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

    auto setupLabel = [this] (juce::Label& label, juce::String text, [[maybe_unused]] float fontSize, juce::Justification justification)
    {
        label.setBorderSize ({ 0, 0, 0, 0 });
        label.setJustificationType (justification);
        label.setFont (A8Type::parameterLabel ());
        label.setText (text, juce::NotificationType::dontSendNotification);
        parameterLabels.push_back (&label);
        addAndMakeVisible (label);
    };
    auto setupTextEditor = [this, &parameterToolTipData] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters,
                                                          juce::String parameterName)
    {
        textEditor.setJustification (justification);
        textEditor.setIndents (2, 0);
        textEditor.setFont (A8Type::value ());
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.setTooltip (parameterToolTipData.getToolTip ("Zone", parameterName));
        HoverHighlight::attach (textEditor);
        addAndMakeVisible (textEditor);
    };

    // SAMPLE FILE SELECTOR LABEL
    setupLabel (sampleNameLabel, "FILE", 15.0, juce::Justification::centredLeft);

    // SAMPLE FILE SELECTOR
    // its colours come from the palette, in applyExplicitColours
    sampleNameSelectLabel.setBorderSize ({ 0, 2, 0, 0 });
    sampleNameSelectLabel.setDialogTitle ("Please select the Assimil8or Preset file you want to load...");
    sampleNameSelectLabel.canMultiSelect (true);
    sampleNameSelectLabel.onFilesSelected = [this] (const juce::StringArray& files)
    {
        if (! handleSamplesInternal (zoneProperties.getId () - 1, files))
        {
            // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
        }
    };
    sampleNameSelectLabel.onPopupMenuCallback = [this] ()
    {
        if (zoneProperties.getSample ().isEmpty ())
            return;
        // for individual cloning: can clone to any zone that has a sample, or the next zone after the last zone that has a sample
        // for all clone: just works
        auto editMenu { createZoneEditMenu ({}, [this] (ZoneProperties& destZoneProperties, [[maybe_unused]] SampleProperties& destSampleProperties)
                                            {
                                                const auto destZoneIndex { destZoneProperties.getId () - 1 };
                                                const auto fileName { juce::File (appProperties.getMostRecentFolder ()).getChildFile (zoneProperties.getSample ()).getFullPathName () };
                                                handleSamplesInternal (destZoneIndex, { fileName });
                                            },
                                            nullptr /* reset is not possible for the sample file parameter, since zones have to have contiguous samples assigned, and resetting one in the middle would break that */,
                                            [this] () { handleSamplesInternal (zoneIndex, { uneditedZoneProperties.getSample () }); },
                                            [this] (ZoneProperties& destZoneProperties) { return destZoneProperties.getId () - 1 <= editManager->getNumUsedZones (parentChannelIndex); },
                                            [] (ZoneProperties&) { return true; }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupLabel (sampleNameSelectLabel, "", 15.0, juce::Justification::centredLeft);
    // it shows a value, not a name, so it is not coloured as a label
    parameterLabels.pop_back ();
    sampleNameSelectLabel.setFont (A8Type::value ());
    applyExplicitColours ();

    // AUDIO FILE CHANNEL SELECT BUTTONS
    auto setupChannelSelectButton = [this] (juce::TextButton& channelSelectButton, juce::String buttonText, int side)
    {
        channelSelectButton.setButtonText (buttonText);
        channelSelectButton.setEnabled (false);
        channelSelectButton.onClick = [this, side] ()
        {
            sideUiChanged (side);
            updateSideSelectButtons (side);
        };
        addAndMakeVisible (channelSelectButton);
    };
    setupChannelSelectButton (leftChannelSelectButton, "L", 0);
    setupChannelSelectButton (rightChannelSelectButton, "R", 1);

    selectSamplePointsClickListener.onClick = [this] () { setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector::SamplePoints, false); };
    // SAMPLE START
    sampleStartLabel.addMouseListener (&selectSamplePointsClickListener, false);
    setupLabel (sampleStartLabel, "SMPL START", 12.0, juce::Justification::centredRight);
    sampleStartTextEditor.getMinValueCallback = [this] { return minZoneProperties.getSampleStart ().value_or (0); };
    sampleStartTextEditor.getMaxValueCallback = [this] { return zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()); };
    sampleStartTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    sampleStartTextEditor.updateDataCallback = [this] (juce::int64 value)
    {
        sampleStartUiChanged (value);
    };
    sampleStartTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getSampleStart ().value_or (0) + static_cast<juce::int64> (valueDelta) };
        sampleStartTextEditor.setValue (newValue);
    };
    sampleStartTextEditor.onPopupMenuCallback = [this] ()
    {
        auto adjustMenu { createSamplePointAdjustMenu (SamplePoint::sampleStart) };
        auto editMenu { createZoneEditMenu (adjustMenu, [this] (ZoneProperties& destZoneProperties, SampleProperties& destSampleProperties)
                                            {
                                                const auto clampedSampleStart { std::clamp (zoneProperties.getSampleStart ().value_or (0),
                                                                                            minZoneProperties.getSampleStart ().value_or (0),
                                                                                            destZoneProperties.getSampleEnd ().value_or (destSampleProperties.getLengthInSamples ())) };
                                                destZoneProperties.setSampleStart (clampedSampleStart, false);
                                            },
                                            [this] () { zoneProperties.setSampleStart (0, true); },
                                            [this] ()
                                            {
                                                const auto uneditedSampleStart { uneditedZoneProperties.getSampleStart ().value_or (-1) };
                                                DebugLog ("ZoneEditor", "uneditedSampleStart: " + juce::String (uneditedSampleStart));
                                                zoneProperties.setSampleStart (uneditedSampleStart, true);
                                                const auto updatedSampleStart { zoneProperties.getSampleStart () };
                                                if (updatedSampleStart.has_value ())
                                                    DebugLog ("ZoneEditor", "updatedSampleStart: " + juce::String (updatedSampleStart.value ()));
                                                else
                                                    DebugLog ("ZoneEditor", "updatedSampleStart: no value");
                                            },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    sampleStartTextEditor.addMouseListener (&selectSamplePointsClickListener, false);
    setupTextEditor (sampleStartTextEditor, juce::Justification::centred, 0, "0123456789", "SampleStart");

    // SAMPLE END
    sampleEndLabel.addMouseListener (&selectSamplePointsClickListener, false);
    setupLabel (sampleEndLabel, "SMPL END", 12.0, juce::Justification::centredRight);
    sampleEndTextEditor.getMinValueCallback = [this] { return zoneProperties.getSampleStart ().value_or (0); };
    sampleEndTextEditor.getMaxValueCallback = [this] { return sampleProperties.getLengthInSamples (); };
    sampleEndTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    sampleEndTextEditor.updateDataCallback = [this] (juce::int64 value)
    {
        sampleEndUiChanged (value);
    };
    sampleEndTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getSampleEnd ().value_or (sampleProperties.getLengthInSamples ()) + static_cast<juce::int64> (valueDelta) };
        sampleEndTextEditor.setValue (newValue);
    };
    sampleEndTextEditor.onPopupMenuCallback = [this] ()
    {
        auto adjustMenu { createSamplePointAdjustMenu (SamplePoint::sampleEnd) };

        auto editMenu { createZoneEditMenu (adjustMenu, [this] (ZoneProperties& destZoneProperties, SampleProperties& destSampleProperties)
                                            {
                                                const auto clampedSampleEnd { std::clamp (zoneProperties.getSampleEnd ().value_or (0),
                                                                                          destZoneProperties.getSampleStart ().value_or (0),
                                                                                          destSampleProperties.getLengthInSamples ()) };
                                                destZoneProperties.setSampleEnd (clampedSampleEnd , false);
                                            },
                                            [this] () { zoneProperties.setSampleEnd (sampleProperties.getLengthInSamples (), true); },
                                            [this] () { zoneProperties.setSampleEnd (uneditedZoneProperties.getSampleEnd ().value_or (-1), true); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    sampleEndTextEditor.addMouseListener (&selectSamplePointsClickListener, false);
    setupTextEditor (sampleEndTextEditor, juce::Justification::centred, 0, "0123456789", "SampleEnd");

    // LOOP START
    selectLoopPointsClickListener.onClick = [this] () { setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector::LoopPoints, false); };
    loopStartLabel.addMouseListener (&selectLoopPointsClickListener, false);
    setupLabel (loopStartLabel, "LOOP START", 12.0, juce::Justification::centredRight);
    loopStartTextEditor.getMinValueCallback = [this] { return minZoneProperties.getLoopStart ().value_or (0); };
    loopStartTextEditor.getMaxValueCallback = [this]
    {
        return editManager->getMaxLoopStart (parentChannelIndex, zoneIndex);
    };
    loopStartTextEditor.toStringCallback = [this] (juce::int64 value) { return juce::String (value); };
    loopStartTextEditor.updateDataCallback = [this] (juce::int64 value)
    {
        const auto originalLoopStart { zoneProperties.getLoopStart ().value_or (0) };
        loopStartUiChanged (value);
        if (treatLoopLengthAsEndInUi)
        {
            // When treating Loop Length as Loop End, we need to adjust the internal storage of Loop Length by the amount Loop Start changed
            const auto lengthChangeAmount { static_cast<double> (originalLoopStart - value) };
            const auto newLoopLength { zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()) + lengthChangeAmount };
            loopLengthUiChanged (newLoopLength);
        }
    };
    loopStartTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getLoopStart ().value_or (0) + static_cast<juce::int64> (valueDelta) };
        loopStartTextEditor.setValue (newValue);
    };
    loopStartTextEditor.onPopupMenuCallback = [this] ()
    {
        auto adjustMenu { createSamplePointAdjustMenu (SamplePoint::loopStart) };

        auto editMenu { createZoneEditMenu (adjustMenu , [this] (ZoneProperties& destZoneProperties, SampleProperties& destSampleProperties)
                                            {
                                                const auto originalLoopStart { destZoneProperties.getLoopStart ().value_or (0) };
                                                const auto clampedLoopStart { std::clamp (zoneProperties.getLoopStart ().value_or (0),
                                                                                          minZoneProperties.getLoopStart ().value_or (0),
                                                                                          editManager->getMaxLoopStart (parentChannelIndex, destZoneProperties.getId () - 1)) };
                                                destZoneProperties.setLoopStart (clampedLoopStart, false);
                                                if (treatLoopLengthAsEndInUi)
                                                {
                                                    // When treating Loop Length as Loop End, we need to adjust the internal storage of Loop Length by the amount Loop Start changed
                                                    const auto lengthChangeAmount { static_cast<double> (originalLoopStart - clampedLoopStart) };
                                                    const auto newLoopLength { destZoneProperties.getLoopLength ().value_or (destSampleProperties.getLengthInSamples ()) + lengthChangeAmount };
                                                    destZoneProperties.setLoopLength (newLoopLength, false);
                                                }

                                            },
                                            [this] ()
                                            {
                                                const auto originalLoopStart { zoneProperties.getLoopStart ().value_or (0) };
                                                zoneProperties.setLoopStart (0, true);
                                                if (treatLoopLengthAsEndInUi)
                                                {
                                                    // When treating Loop Length as Loop End, we need to adjust the internal storage of Loop Length by the amount Loop Start changed
                                                    const auto lengthChangeAmount { static_cast<double> (originalLoopStart) };
                                                    const auto newLoopLength { zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()) + lengthChangeAmount };
                                                    zoneProperties.setLoopLength (newLoopLength, false);
                                                }
                                            },
                                            [this] ()
                                            {
                                                // TODO - need to check and update Loop Length if treatLoopLengthAsEndInUi is true
                                                zoneProperties.setLoopStart (uneditedZoneProperties.getLoopStart ().value_or (-1), true);
                                            },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    loopStartTextEditor.addMouseListener (&selectLoopPointsClickListener, false);
    setupTextEditor (loopStartTextEditor, juce::Justification::centred, 0, "0123456789", "LoopStart");

    // LOOP LENGTH/END
    loopLengthLabel.addMouseListener (&selectLoopPointsClickListener, false);
    setupLabel (loopLengthLabel, "LOOP LENGTH", 12.0, juce::Justification::centredRight);
    loopLengthTextEditor.getMinValueCallback = [this]
    {
        if (sampleProperties.getLengthInSamples () == 0)
            return  0.0;
        if (treatLoopLengthAsEndInUi)
            return zoneProperties.getLoopStart ().value_or (0.0) + minZoneProperties.getLoopLength ().value ();
        else
            return minZoneProperties.getLoopLength ().value ();
    };
    loopLengthTextEditor.getMaxValueCallback = [this]
    {
        if (treatLoopLengthAsEndInUi)
            return static_cast<double> (sampleProperties.getLengthInSamples ());
        else
            return static_cast<double> (sampleProperties.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0));
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
    };
    loopLengthTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()) + valueDelta };
        loopLengthTextEditor.setValue (newValue);
    };

    loopLengthTextEditor.onPopupMenuCallback = [this] ()
    {
        auto adjustMenu { createSamplePointAdjustMenu (SamplePoint::loopEnd) };
        auto editMenu { createZoneEditMenu (adjustMenu, [this] (ZoneProperties& destZoneProperties, SampleProperties& destSampleProperties)
                                            {
                                                const auto clampedLoopLength { std::clamp (zoneProperties.getLoopLength ().value_or (sampleProperties.getLengthInSamples ()),
                                                                                           minZoneProperties.getLoopLength ().value (),
                                                                                           static_cast<double> (destSampleProperties.getLengthInSamples () - destZoneProperties.getLoopStart ().value_or (0))) };
                                                destZoneProperties.setLoopLength (clampedLoopLength, false);
                                            },
                                            [this] ()
                                            {
                                                zoneProperties.setLoopLength (sampleProperties.getLengthInSamples () - static_cast<double> (zoneProperties.getLoopStart ().value_or (0)), true);
                                            },
                                            [this] () { zoneProperties.setLoopLength (uneditedZoneProperties.getLoopLength ().value_or (-1.0), true); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    loopLengthTextEditor.addMouseListener (&selectLoopPointsClickListener, false);
    setupTextEditor (loopLengthTextEditor, juce::Justification::centred, 0, ".0123456789", "LoopLength");

    // MIN VOLTAGE
    setupLabel (minVoltageLabel, "MIN VOLTAGE", 15.0, juce::Justification::centredRight);
    minVoltageTextEditor.getMinValueCallback = [this] { return minZoneProperties.getMinVoltage (); };
    minVoltageTextEditor.getMaxValueCallback = [this] { return maxZoneProperties.getMinVoltage (); };
    minVoltageTextEditor.highlightErrorCallback = [this] ()
    {
        ErrorHelpers::setColorIfError (minVoltageTextEditor, editManager->isMinVoltageInRange (parentChannelIndex, zoneIndex, minVoltageTextEditor.getText ().getDoubleValue ()));
    };
    minVoltageTextEditor.snapValueCallback = [this] (double value) { return editManager->clampMinVoltage (parentChannelIndex, zoneIndex, value); };
    minVoltageTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    minVoltageTextEditor.updateDataCallback = [this] (double value) { minVoltageUiChanged (value); };
    minVoltageTextEditor.getIncrementCallback = [] () { return 0.01; };
    minVoltageTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getMinVoltage () + valueDelta };
        minVoltageTextEditor.setValue (newValue);
    };
    minVoltageTextEditor.onPopupMenuCallback = [this] ()
    {
        if (zoneProperties.getSample ().isEmpty ())
            return;
        auto editMenu { createZoneEditMenu ({}, nullptr /* cloning across zones does not makes sense, as they have to be unique values */,
                                            [this] () { editManager->resetMinVoltage (parentChannelIndex, zoneIndex); },
                                            [this] () { zoneProperties.setMinVoltage (editManager->clampMinVoltage (parentChannelIndex, zoneIndex, uneditedZoneProperties.getMinVoltage ()), true); },
                                            [] (ZoneProperties&) { return false; },
                                            [] (ZoneProperties&) { return false; }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (minVoltageTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MinVoltage");

    // PITCH OFFSET
    pitchOffsetTextEditor.getMinValueCallback = [this] { return minZoneProperties.getPitchOffset (); };
    pitchOffsetTextEditor.getMaxValueCallback = [this] { return maxZoneProperties.getPitchOffset (); };
    pitchOffsetTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 2, true); };
    pitchOffsetTextEditor.updateDataCallback = [this] (double value) { pitchOffsetUiChanged (value); };
    pitchOffsetTextEditor.getIncrementCallback = [] () { return 0.01; };
    pitchOffsetTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getPitchOffset () + valueDelta };
        pitchOffsetTextEditor.setValue (newValue);
    };
    pitchOffsetTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createZoneEditMenu ({}, [this] (ZoneProperties& destZoneProperties, SampleProperties&) { destZoneProperties.setPitchOffset (zoneProperties.getPitchOffset (), false); },
                                            [this] () { zoneProperties.setPitchOffset (0, true); },
                                            [this] () { zoneProperties.setPitchOffset (uneditedZoneProperties.getPitchOffset (), true); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };

    setupLabel (pitchOffsetLabel, "PITCH OFFSET", 15.0, juce::Justification::centredRight);
    setupTextEditor (pitchOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PitchOffset");

    // LEVEL OFFSET
    setupLabel (levelOffsetLabel, "LEVEL OFFSET", 15.0, juce::Justification::centredRight);
    levelOffsetTextEditor.getMinValueCallback = [this] { return minZoneProperties.getLevelOffset (); };
    levelOffsetTextEditor.getMaxValueCallback = [this] { return maxZoneProperties.getLevelOffset (); };
    levelOffsetTextEditor.toStringCallback = [this] (double value) { return FormatHelpers::formatDouble (value, 1, true); };
    levelOffsetTextEditor.updateDataCallback = [this] (double value) { levelOffsetUiChanged (value); };
    levelOffsetTextEditor.getIncrementCallback = [] () { return 0.1; };
    levelOffsetTextEditor.onDragCallback = [this] (double valueDelta)
    {
        const auto newValue { zoneProperties.getLevelOffset () + valueDelta };
        levelOffsetTextEditor.setValue (newValue);
    };
    levelOffsetTextEditor.onPopupMenuCallback = [this] ()
    {
        auto editMenu { createZoneEditMenu ({}, [this] (ZoneProperties& destZoneProperties, SampleProperties&) { destZoneProperties.setLevelOffset (zoneProperties.getLevelOffset (), false); },
                                            [this] () { zoneProperties.setLevelOffset (0, true); },
                                            [this] () { zoneProperties.setLevelOffset (uneditedZoneProperties.getLevelOffset (), true); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); },
                                            [] (ZoneProperties& destZoneProperties) { return destZoneProperties.getSample ().isNotEmpty (); }) };
        editMenu.showMenuAsync ({}, [this] (int) {});
    };
    setupTextEditor (levelOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LevelOffset");
}

void ZoneEditor::init (juce::ValueTree zonePropertiesVT, juce::ValueTree uneditedZonePropertiesVT, juce::ValueTree rootPropertiesVT)
{
    //DebugLog ("ZoneEditor[" + juce::String (zoneProperties.getId ()) + "]", "init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);
    SystemServices systemServices { runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::yes };
    audioManager = systemServices.getAudioManager ();
    sampleNameSelectLabel.setFileFilter (audioManager->getFileTypesList ());
    editManager = systemServices.getEditManager ();

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
    uneditedZoneProperties.wrap (uneditedZonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    zoneIndex = zoneProperties.getId () - 1;
    jassert (ChannelProperties::isChannelPropertiesVT (zoneProperties.getValueTree ().getParent ()));
    parentChannelProperties.wrap (zoneProperties.getValueTree ().getParent (), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    parentChannelIndex = parentChannelProperties.getId () - 1;

    SampleManagerProperties sampleManagerProperties (runtimeRootProperties.getValueTree (), SampleManagerProperties::WrapperType::client, SampleManagerProperties::EnableCallbacks::no);
    sampleProperties.wrap (sampleManagerProperties.getSamplePropertiesVT (parentChannelIndex, zoneIndex), SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::yes);
    sampleProperties.onStatusChange = [this] (SampleStatus status)
    {
        if (status == SampleStatus::exists)
        {
            //DebugLog ("ZoneEditor", "sample Status exists");
            // when we receive this callback, it means all of the other sample data is updated too
            setEditComponentsEnabled (true);
            oneShotPlayButton.setEnabled (true && ! isStereoRightChannelMode);
            loopPlayButton.setEnabled (true && ! isStereoRightChannelMode);
            updateLoopPointsView ();
            updateSamplePositionInfo ();
            updateSampleFileInfo (zoneProperties.getSample ());
            updateSideSelectButtons (zoneProperties.getSide ());
        }
        else if (status == SampleStatus::uninitialized)
        {
            //DebugLog ("ZoneEditor", "sample Status uninitialized");
            // this means the sample has been unloaded
            setEditComponentsEnabled (false);
            oneShotPlayButton.setEnabled (false);
            loopPlayButton.setEnabled (false);
            updateLoopPointsView ();
            updateSamplePositionInfo ();
            updateSideSelectButtons (0);
        }
        else if (status == SampleStatus::doesNotExist)
        {
            //DebugLog ("ZoneEditor", "sample Status doesNotExist");
            // obviously none of the sample data can be used
            setEditComponentsEnabled (false);
            oneShotPlayButton.setEnabled (false);
            loopPlayButton.setEnabled (false);
            updateLoopPointsView ();
            updateSamplePositionInfo ();
            updateSampleFileInfo (zoneProperties.getSample ());
            updateSideSelectButtons (0);
        }
        else if (status == SampleStatus::wrongFormat)
        {
            //DebugLog ("ZoneEditor", "sample Status wrongFormat");
            // we should not be able to load a sample of the wrong format, but if an already loaded sample is changed outside of the app, this could happen
            setEditComponentsEnabled (false);
            oneShotPlayButton.setEnabled (false);
            loopPlayButton.setEnabled (false);
            updateLoopPointsView ();
            updateSamplePositionInfo ();
            updateSampleFileInfo (zoneProperties.getSample ());
            updateSideSelectButtons (0);
        }
    };

    setupZonePropertiesCallbacks ();

    levelOffsetDataChanged (zoneProperties.getLevelOffset ());
    loopLengthDataChanged (zoneProperties.getLoopLength ());
    loopStartDataChanged (zoneProperties.getLoopStart ());
    minVoltageDataChanged (zoneProperties.getMinVoltage ());
    pitchOffsetDataChanged (zoneProperties.getPitchOffset ());
    sampleDataChanged (zoneProperties.getSample ());
    sampleStartDataChanged (zoneProperties.getSampleStart ());
    sampleEndDataChanged (zoneProperties.getSampleEnd ());
    sideDataChanged (zoneProperties.getSide ());

    setLoopLengthIsEnd (parentChannelProperties.getLoopLengthIsEnd ());
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

void ZoneEditor::setStereoRightChannelMode (bool newStereoRightChannelMode)
{
    isStereoRightChannelMode = newStereoRightChannelMode;

    oneShotPlayButton.setEnabled (! isStereoRightChannelMode && sampleProperties.getStatus () == SampleStatus::exists);
    loopPlayButton.setEnabled (! isStereoRightChannelMode && sampleProperties.getStatus () == SampleStatus::exists);
    toolsButton.setEnabled (! isStereoRightChannelMode);
    levelOffsetTextEditor.setEnabled (! isStereoRightChannelMode);
    loopLengthTextEditor.setEnabled (! isStereoRightChannelMode);
    loopStartTextEditor.setEnabled (! isStereoRightChannelMode);
    minVoltageTextEditor.setEnabled (! isStereoRightChannelMode);
    pitchOffsetTextEditor.setEnabled (! isStereoRightChannelMode);
    //leftChannelSelectButton.setEnabled (! isStereoRightChannelMode); // can still edit in stereo/right channel mode
    //rightChannelSelectButton.setEnabled (! isStereoRightChannelMode); // can still edit in stereo/right channel mode
    //sampleNameSelectLabel.setEnabled (! isStereoRightChannelMode); // can still edit in stereo/right channel mode
    sampleEndTextEditor.setEnabled (! isStereoRightChannelMode);
    sampleStartTextEditor.setEnabled (! isStereoRightChannelMode);
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
    zoneProperties.onIdChange = [this] ([[maybe_unused]] int id) { jassertfalse; /* I don't think this should change while we are editing */ };
    zoneProperties.onLevelOffsetChange = [this] (double levelOffset) { levelOffsetDataChanged (levelOffset); };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double> loopLength) { loopLengthDataChanged (loopLength); };
    zoneProperties.onLoopStartChange = [this] (std::optional <juce::int64> loopStart) { loopStartDataChanged (loopStart); };
    zoneProperties.onMinVoltageChange = [this] (double minVoltage) { minVoltageDataChanged (minVoltage); };
    zoneProperties.onPitchOffsetChange = [this] (double pitchOffset) { pitchOffsetDataChanged (pitchOffset); };
    zoneProperties.onSampleChange = [this] (juce::String sample) { sampleDataChanged (sample); };
    zoneProperties.onSampleStartChange = [this] (std::optional <juce::int64> sampleStart) { sampleStartDataChanged (sampleStart); };
    zoneProperties.onSampleEndChange = [this] (std::optional <juce::int64> sampleEnd) { sampleEndDataChanged (sampleEnd); };
    zoneProperties.onSideChange = [this] (int side) { sideDataChanged (side); };
}

void ZoneEditor::applyExplicitColours ()
{
    // a sample the folder does not hold is said so in red
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, findColour (sampleFileMissing ? A8Colours::danger : A8Colours::text));
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::backgroundColourId, findColour (juce::TextEditor::ColourIds::backgroundColourId));
    sampleNameSelectLabel.setOutline (findColour (juce::TextEditor::ColourIds::outlineColourId));
}

void ZoneEditor::lookAndFeelChanged ()
{
    juce::Component::lookAndFeelChanged ();
    for (auto* label : parameterLabels)
        label->setColour (juce::Label::ColourIds::textColourId, findColour (A8Colours::textDim));
    applyExplicitColours ();
}

void ZoneEditor::paint (juce::Graphics& g)
{
    // the group of sample points (sample or loop) that the waveform and the audition buttons are using;
    // its outline is drawn in paintOverChildren, as the loop tuner that both groups share covers this
    g.setColour (findColour (A8Colours::selectedRow));
    g.fillRoundedRectangle (activePointBackground->toFloat (), 2.0f);

    // A short bar between each point's name and its field, in the colour of its marker on the waveform,
    // as SquidManager has, so the number and the line it moves read as one object. The labels are cut
    // short in resized to leave room for it.
    auto drawSwatch = [this, &g] (const juce::Label& label, int colourId)
    {
        const auto swatchBounds { juce::Rectangle<int> { label.getRight () + kSwatchGap, label.getY (), kSwatchWidth, label.getHeight () }
                                    .withSizeKeepingCentre (kSwatchWidth, std::min (14, label.getHeight ())) };
        g.setColour (findColour (colourId));
        g.fillRoundedRectangle (swatchBounds.toFloat (), 1.0f);
    };
    drawSwatch (sampleStartLabel, A8Colours::markerStart);
    drawSwatch (sampleEndLabel, A8Colours::markerEnd);
    drawSwatch (loopStartLabel, A8Colours::markerLoop);
    drawSwatch (loopLengthLabel, A8Colours::markerLoopEnd);
}

void ZoneEditor::paintOverChildren (juce::Graphics& g)
{
    // drawn on the group's own bounds, which end a pixel short of the fields of the other group, so the
    // outline meets the loop tuner's edge rather than crossing a field
    g.setColour (findColour (A8Colours::accentDeep));
    g.drawRoundedRectangle (activePointBackground->toFloat ().reduced (0.5f), 2.0f, 1.0f);

    if (draggingFilesCount == 0)
        return;

    // the drop target is washed over, and what the drop will do is said on a plate,
    // as a tooltip would say it
    auto drawMessage = [this, &g] (juce::Rectangle<int> area, const juce::String& message, bool isProblem)
    {
        const auto messageFont { A8Type::body () };
        const auto plateWidth { std::min (area.getWidth () - 12, A8Paint::textWidth (messageFont, message) + 20) };
        const auto plateHeight { isProblem ? 44 : 26 };
        const auto plateBounds { area.withSizeKeepingCentre (plateWidth, plateHeight).toFloat () };
        A8Paint::messagePlate (g, *this, plateBounds, 3.0f);
        g.setFont (messageFont);
        g.setColour (A8Paint::messageInk (*this, isProblem));
        g.drawFittedText (message, plateBounds.toNearestInt ().reduced (6, 2), juce::Justification::centred, 3);
    };

    auto localBounds { getLocalBounds () };
    if (! supportedFile)
    {
        g.setColour (findColour (A8Colours::dropOverlay));
        g.fillRect (localBounds);
        drawMessage (localBounds, draggingFilesCount == 1 ? "Unsupported file type" : "One, or more, unsupported file types", true);
        return;
    }

    if (dropIndex == -1 || zoneProperties.getId () == 1)
    {
        g.setColour (findColour (A8Colours::dropOverlay));
        g.fillRect (localBounds);
        drawMessage (localBounds, "Start on Zone " + juce::String (zoneProperties.getId ()), false);
        return;
    }

    // with several files, the upper half starts them on zone 1 and the lower half on this zone; the half
    // under the pointer is the one washed over
    const auto topHalfBounds { localBounds.removeFromTop (localBounds.getHeight () / 2) };
    g.setColour (findColour (A8Colours::dropOverlay));
    g.fillRect (dropIndex == 0 ? topHalfBounds : localBounds);
    if (dropIndex == 0)
        drawMessage (topHalfBounds, "Start on Zone 1", false);
    else
        drawMessage (localBounds, "Start on Zone " + juce::String (zoneProperties.getId ()), false);
}

void ZoneEditor::resized ()
{
    const auto xOffset { 10 };
    // the zone column is sized in ChannelEditor; everything here stretches to fill it
    const auto width { std::max (160, getWidth () - xOffset - 8) };
    const auto interParameterYOffset { 1 };
    const auto spaceBetweenLabelAndInput { 3 };
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    jassert (displayToolsMenu != nullptr);
    toolsButton.setBounds (getWidth () - 6 - toolsButton.getIdealWidth (), getHeight () - 6 - ActionButton::kSmallHeight, toolsButton.getIdealWidth (), ActionButton::kSmallHeight);

    const auto sampleNameLabelScale { 0.156f };
    const auto sampleNameInputScale { 1.f - sampleNameLabelScale };
    sampleNameLabel.setBounds (xOffset, 5, scaleWidth (sampleNameLabelScale), 20);
    sampleNameSelectLabel.setBounds (sampleNameLabel.getRight () + spaceBetweenLabelAndInput, 5,
                                     scaleWidth (sampleNameInputScale) - spaceBetweenLabelAndInput + 1 - 22, 20);

    leftChannelSelectButton.setBounds (sampleNameSelectLabel.getRight () + 2, sampleNameSelectLabel.getY (), 20, 10);
    rightChannelSelectButton.setBounds (sampleNameSelectLabel.getRight () + 2, leftChannelSelectButton.getBottom () + 1, 20, 10);

    const auto loopPointsViewHeight { 50 };
    const auto samplePointLabelScale { 0.45f };
    const auto samplePointInputScale { 1.f - samplePointLabelScale };
    sampleStartLabel.setBounds (xOffset, sampleNameSelectLabel.getBottom () + 5, scaleWidth (samplePointLabelScale), 20);
    sampleStartTextEditor.setBounds (sampleStartLabel.getRight () + spaceBetweenLabelAndInput, sampleStartLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    sampleEndLabel.setBounds (xOffset, sampleStartLabel.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    sampleEndTextEditor.setBounds (sampleEndLabel.getRight () + spaceBetweenLabelAndInput, sampleEndLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    samplePointsBackground = { sampleStartLabel.getX (), sampleStartLabel.getY () - 1,
                               sampleEndTextEditor.getRight () - sampleStartLabel.getX () + 1,
                               sampleStartTextEditor.getHeight () + sampleEndTextEditor.getHeight () + loopPointsViewHeight + (interParameterYOffset * 2) + 1 };

    auto loopPointsViewBounds { juce::Rectangle<int> { xOffset, sampleEndTextEditor.getBottom () + interParameterYOffset, width + 1, loopPointsViewHeight } };
    loopPointsView.setBounds (loopPointsViewBounds/*.reduced (3, 0)*/);

    loopStartLabel.setBounds (xOffset, loopPointsView.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    loopStartTextEditor.setBounds (loopStartLabel.getRight () + spaceBetweenLabelAndInput, loopStartLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    loopLengthLabel.setBounds (xOffset, loopStartLabel.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    loopLengthTextEditor.setBounds (loopLengthLabel.getRight () + spaceBetweenLabelAndInput, loopLengthLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    loopPointsBackground = { loopStartLabel.getX (), loopStartLabel.getY () - loopPointsViewHeight,
                             loopLengthTextEditor.getRight () - loopStartLabel.getX () + 1,
                             loopStartTextEditor.getHeight () + loopLengthTextEditor.getHeight () + loopPointsViewHeight + (interParameterYOffset * 2) + 1 };

    // the names give up the right of their space to their marker swatches, which are drawn in paint
    for (auto* label : { &sampleStartLabel, &sampleEndLabel, &loopStartLabel, &loopLengthLabel })
        label->setBounds (label->getBounds ().withTrimmedRight (kSwatchGap + kSwatchWidth));

    auto playControlsArea { loopPointsView.getBounds () };
    const auto buttonHeight { playControlsArea.getHeight () / 3 };
    oneShotPlayButton.setBounds (playControlsArea.getX () + 3, playControlsArea.getY () + 3, 35, buttonHeight);
    loopPlayButton.setBounds (playControlsArea.getX () + 3, playControlsArea.getBottom () - 3 - buttonHeight, 35, buttonHeight);

    const auto otherLabelScale { 0.66f };
    const auto otherInputScale { 1.f - otherLabelScale };
    minVoltageLabel.setBounds (xOffset, loopLengthTextEditor.getBottom () + 5, scaleWidth (otherLabelScale), 20);
    minVoltageTextEditor.setBounds (minVoltageLabel.getRight () + spaceBetweenLabelAndInput, minVoltageLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);

    pitchOffsetLabel.setBounds (xOffset, minVoltageTextEditor.getBottom () + 3, scaleWidth (otherLabelScale), 20);
    pitchOffsetTextEditor.setBounds (pitchOffsetLabel.getRight () + spaceBetweenLabelAndInput, pitchOffsetLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);

    levelOffsetLabel.setBounds (xOffset, pitchOffsetLabel.getBottom () + 3, scaleWidth (otherLabelScale), 20);
    levelOffsetTextEditor.setBounds (levelOffsetLabel.getRight () + spaceBetweenLabelAndInput, levelOffsetLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);
}

void ZoneEditor::setEditComponentsEnabled (bool enabled)
{
    sampleStartTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
    sampleEndTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
    loopStartTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
    loopLengthTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
    minVoltageTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
    pitchOffsetTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
    levelOffsetTextEditor.setEnabled (enabled && ! isStereoRightChannelMode);
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

juce::PopupMenu ZoneEditor::createZoneEditMenu (juce::PopupMenu existingPopupMenu, std::function <void (ZoneProperties&, SampleProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter,
                                                std::function<bool (ZoneProperties&)> canCloneCallback, std::function<bool (ZoneProperties&)> canCloneToAllCallback)
{
    // you can pass in a nullptr for one of the callbacks, to disable that item, but at least one of these should be valid, if not the caller should just not be trying to display an edit menu
    jassert (setter != nullptr || resetter != nullptr);
    jassert (reverter != nullptr);
    jassert (canCloneCallback != nullptr);
    jassert (canCloneToAllCallback != nullptr);
    juce::PopupMenu editMenu (existingPopupMenu);
    if (setter != nullptr)
    {
        juce::PopupMenu cloneMenu;
        for (auto destZoneIndex { 0 }; destZoneIndex < 8; ++destZoneIndex)
        {
            // TODO - the actual clone code should be in the EditManager too
            if (destZoneIndex != zoneIndex)
            {
                auto canCloneToDestZone { true };
                editManager->forZones (parentChannelIndex, { destZoneIndex }, [this, canCloneCallback, &canCloneToDestZone] (juce::ValueTree zonePropertiesVT, juce::ValueTree sampleZonePropertiesVT)
                {
                    ZoneProperties destZoneProperties (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    canCloneToDestZone = canCloneCallback (destZoneProperties);
                });
                if (canCloneToDestZone)
                {
                    cloneMenu.addItem ("To Zone " + juce::String (destZoneIndex + 1), true, false, [this, destZoneIndex, setter] ()
                    {
                        editManager->forZones (parentChannelIndex, { destZoneIndex }, [this, setter] (juce::ValueTree zonePropertiesVT, juce::ValueTree sampleZonePropertiesVT)
                        {
                            ZoneProperties destZoneProperties (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                            SampleProperties destSampleProperties (sampleZonePropertiesVT, SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::no);
                            setter (destZoneProperties, destSampleProperties);
                        });
                    });
                }
            }
        }
        std::vector<int> zoneIndexList;
        // build list of other zones
        for (auto destZoneIndex { 0 }; destZoneIndex < 8; ++destZoneIndex)
            if (destZoneIndex != zoneIndex)
                zoneIndexList.emplace_back (destZoneIndex);
        auto canCloneDestZoneCount { 0 };
        editManager->forZones (parentChannelIndex, zoneIndexList, [this, canCloneToAllCallback, &canCloneDestZoneCount] (juce::ValueTree zonePropertiesVT, juce::ValueTree sampleZonePropertiesVT)
        {
            ZoneProperties destZoneProperties (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
            if (canCloneToAllCallback (destZoneProperties))
                ++canCloneDestZoneCount;
        });
        if (canCloneDestZoneCount > 0)
        {
            cloneMenu.addItem ("To All", true, false, [this, setter, zoneIndexList] ()
            {
                // clone to other zones
                editManager->forZones (parentChannelIndex, zoneIndexList, [this, setter] (juce::ValueTree zonePropertiesVT, juce::ValueTree sampleZonePropertiesVT)
                {
                    ZoneProperties destZoneProperties (zonePropertiesVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                    SampleProperties destSampleProperties (sampleZonePropertiesVT, SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::no);
                    setter (destZoneProperties, destSampleProperties);
                });
            });
        }
        editMenu.addSubMenu ("Clone", cloneMenu, true);
    }

    if (resetter != nullptr)
        editMenu.addItem ("Default", true, false, [this, resetter] () { resetter (); });

    editMenu.addItem ("Revert", true, false, [this, reverter] () { reverter (); });

    return editMenu;
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
    if (sampleProperties.getStatus () != SampleStatus::uninitialized)
        loopLengthTextEditor.setText (formatLoopLength (loopLength.value_or (static_cast<double> (sampleProperties.getLengthInSamples () - zoneProperties.getLoopStart ().value_or (0)))));
    else
        loopLengthTextEditor.setText ("0");
    updateLoopPointsView ();
}

void ZoneEditor::loopLengthUiChanged (double loopLength)
{
    zoneProperties.setLoopLength (loopLength == static_cast<double> (sampleProperties.getLengthInSamples ()) ? -1.0 : loopLength, false);
    updateLoopPointsView ();
}

void ZoneEditor::loopStartDataChanged (std::optional<juce::int64> loopStart)
{
    loopStartTextEditor.setText (juce::String (loopStart.value_or (0)));
    if (treatLoopLengthAsEndInUi)
        loopLengthDataChanged (zoneProperties.getLoopLength ());
    updateLoopPointsView ();
}

void ZoneEditor::loopStartUiChanged (juce::int64 loopStart)
{
    zoneProperties.setLoopStart (loopStart == 0 ? -1 : loopStart, false);
    updateLoopPointsView ();
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
    sampleFileMissing = sampleProperties.getStatus () != SampleStatus::exists;
    if (! sampleFileMissing)
    {
        if (! zoneProperties.getSampleEnd ().has_value ())
            sampleEndTextEditor.setText (juce::String (sampleProperties.getLengthInSamples ()));
        if (! zoneProperties.getLoopLength ().has_value ())
            loopLengthTextEditor.setText (formatLoopLength (static_cast<double> (sampleProperties.getLengthInSamples ())));
    }
    loopPointsView.repaint ();
    applyExplicitColours ();
}

void ZoneEditor::updateSamplePositionInfo ()
{
    loopLengthDataChanged (zoneProperties.getLoopLength ());
    loopStartDataChanged (zoneProperties.getLoopStart ());
    sampleStartDataChanged (zoneProperties.getSampleStart ());
    sampleEndDataChanged (zoneProperties.getSampleEnd ());
}

void ZoneEditor::updateSideSelectButtons (int side)
{
    auto disableButtons = [this] ()
    {
        leftChannelSelectButton.setEnabled (false);
        rightChannelSelectButton.setEnabled (false);
        leftChannelSelectButton.setToggleState (false, juce::NotificationType::dontSendNotification);
        rightChannelSelectButton.setToggleState (false, juce::NotificationType::dontSendNotification);
    };
    if (sampleProperties.getStatus () != SampleStatus::exists)
    {
        disableButtons ();
    }
    else if (sampleProperties.getNumChannels () == 1)
    {
        disableButtons ();
    }
    else
    {
        leftChannelSelectButton.setEnabled (true);
        rightChannelSelectButton.setEnabled (true);
        leftChannelSelectButton.setToggleState (side == 0, juce::NotificationType::dontSendNotification);
        rightChannelSelectButton.setToggleState (side == 1, juce::NotificationType::dontSendNotification);
    }
}

void ZoneEditor::sampleDataChanged (juce::String sample)
{
    //DebugLog ("ZoneEditor", "ZoneEditor[" + juce::String (zoneProperties.getId ()) + "]::sampleDataChanged: '" + sample + "'");
    // the file name is shown without its extension, which is always .wav on an Assimil8or card
    sampleNameSelectLabel.setText (sample.containsChar ('.') ? sample.upToLastOccurrenceOf (".", false, false) : sample,
                                   juce::NotificationType::dontSendNotification);
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
    // -1 indicates the value is default
    zoneProperties.setSampleStart (sampleStart == 0 ? -1 : sampleStart, false);
    updateLoopPointsView ();
}

void ZoneEditor::sampleEndDataChanged (std::optional<juce::int64> sampleEnd)
{
    if (sampleProperties.getStatus () != SampleStatus::uninitialized)
        sampleEndTextEditor.setText (juce::String (sampleEnd.value_or (sampleProperties.getLengthInSamples ())));
    else
        sampleEndTextEditor.setText ("0");

    updateLoopPointsView ();
}

void ZoneEditor::sampleEndUiChanged (juce::int64 sampleEnd)
{
    // -1 indicates the value is default
    zoneProperties.setSampleEnd (sampleEnd == sampleProperties.getLengthInSamples () ? -1 : sampleEnd, false);
    updateLoopPointsView ();
}

void ZoneEditor::sideDataChanged (int side)
{
    if (sampleProperties.getStatus () == SampleStatus::exists && sampleProperties.getNumChannels () == 1 && side == 1)
    {
        side = 0;
        zoneProperties.setSide (0, false);
    }
    updateSideSelectButtons (side);
    updateLoopPointsView ();
}

void ZoneEditor::sideUiChanged (int side)
{
    zoneProperties.setSide (side, false);
    updateLoopPointsView ();
}
