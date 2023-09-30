#include "ZoneEditor.h"
#include "FormatHelpers.h"
#include "ParameterToolTipData.h"
#include "../../../Assimil8or/Preset/ChannelProperties.h"
#include "../../../Assimil8or/Preset/PresetProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/PersistentRootProperties.h"
#include <algorithm>

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

    sampleNameSelectLabel.onFilesSelected = [this] (const juce::StringArray& files)
    {
        if (files.size () != 1)
            return;
        if (! handleSelectedFile (juce::File (files [0])))
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
    setupZoneComponents ();
}

bool ZoneEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    if (files.size () != 1)
        return false;
    const auto draggedFile { juce::File (files [0]) };
    return isSupportedAudioFile (draggedFile);
}

void ZoneEditor::filesDropped (const juce::StringArray& files, int, int)
{
    draggingFiles = false;
    repaint ();
    if (files.size () != 1)
        return;
    if (! handleSelectedFile (juce::File (files [0])))
    {
        // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
    }

}

void ZoneEditor::fileDragEnter (const juce::StringArray& files, int, int)
{
    draggingFiles = true;
    repaint ();
}

void ZoneEditor::fileDragMove (const juce::StringArray& files, int, int)
{
}

void ZoneEditor::fileDragExit (const juce::StringArray& files)
{
    draggingFiles = false;;
    repaint ();
}

bool ZoneEditor::handleSelectedFile (juce::File fileNameAndPath)
{
    if (! isSupportedAudioFile (fileNameAndPath))
        return false;

    // if file not in preset folder, then copy
    if (appProperties.getMostRecentFolder () != fileNameAndPath.getParentDirectory ().getFullPathName ())
    {
        // TODO handle case where file of same name already exists
        // TODO should copy be moved to a thread?
        // 
        // copy file
        fileNameAndPath.copyFileTo (juce::File (appProperties.getMostRecentFolder ()).getChildFile (fileNameAndPath.getFileName ()));
    }
    // assign file to zone
    loadSample (fileNameAndPath.getFileName ());

    return true;
}

void ZoneEditor::loadSample (juce::String sampleFileName)
{
    if (sampleFileName == zoneProperties.getSample ())
        return;

    sampleLength = 0;
    if (sampleFileName.isNotEmpty ())
    {
        auto sampleFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (sampleFileName) };
        if (sampleFile.getFileExtension () == "")
            sampleFile = sampleFile.withFileExtension (".wav");
        if (sampleFile.getFileName () == zoneProperties.getSample ())
            return;

        if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile)); reader != nullptr)
        {
            sampleLength = reader->lengthInSamples;
            sampleFileName = sampleFile.getFileName (); // this copies the added .wav extension if it wasn't in the original name
            if (reader->numChannels == 2)
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
    }
    zoneProperties.setSampleStart (-1, true);
    zoneProperties.setSampleEnd (-1, true);
    zoneProperties.setLoopStart (-1, true);
    zoneProperties.setLoopLength (-1, true);

    if (onSampleChange != nullptr)
        onSampleChange (sampleFileName);

    updateSamplePositionInfo ();

    sampleUiChanged (sampleFileName);
    sampleNameSelectLabel.setText (sampleFileName, juce::NotificationType::dontSendNotification);
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
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
                                                          juce::String parameterName, std::function<void ()> validateCallback, std::function<void (juce::String)> doneEditingCallback)
    {
        jassert (doneEditingCallback != nullptr);
        textEditor.setJustification (justification);
        textEditor.setIndents (2, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.onFocusLost = [this, &textEditor, doneEditingCallback] () { doneEditingCallback (textEditor.getText ()); };
        textEditor.onReturnKey = [this, &textEditor, doneEditingCallback] () { doneEditingCallback (textEditor.getText ()); };
        textEditor.setTooltip (parameterToolTipData.getToolTip ("Zone", parameterName));
        if (validateCallback != nullptr)
            textEditor.onTextChange = [this, validateCallback] () { validateCallback (); };
        addAndMakeVisible (textEditor);
    };
    setupLabel (sampleNameLabel, "FILE", 15.0, juce::Justification::centredLeft);
    setupLabel (sampleNameSelectLabel, "", 15.0, juce::Justification::centredLeft);
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::textColourId, levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::textColourId));
    sampleNameSelectLabel.setColour (juce::Label::ColourIds::backgroundColourId, levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::backgroundColourId));
    sampleNameSelectLabel.setOutline (levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::outlineColourId));
    sampleNameSelectLabel.setBorderSize ({ 0, 2, 0, 0 });
    // SAMPLE START
    setupLabel (sampleStartLabel, "SMPL START", 12.0, juce::Justification::centredRight);
    setupTextEditor (sampleStartTextEditor, juce::Justification::centred, 0, "0123456789", "SampleStart", [this] ()
    {
        FormatHelpers::setColorIfError (sampleStartTextEditor, minZoneProperties.getSampleStart ().value_or (0), zoneProperties.getSampleEnd ().value_or (sampleLength));
    },
    [this] (juce::String text)
    {
        const auto sampleStart { std::clamp (text.getLargeIntValue (), minZoneProperties.getSampleStart ().value_or (0), zoneProperties.getSampleEnd ().value_or (sampleLength)) };
        sampleStartUiChanged (sampleStart);
        sampleStartTextEditor.setText (juce::String (sampleStart));
    });
    // SAMPLE END
    setupLabel (sampleEndLabel, "SMPL END", 12.0, juce::Justification::centredRight);
    setupTextEditor (sampleEndTextEditor, juce::Justification::centred, 0, "0123456789", "SampleEnd", [this] ()
    {
        FormatHelpers::setColorIfError (sampleEndTextEditor, zoneProperties.getSampleStart ().value_or (0), sampleLength);
    },
    [this] (juce::String text)
    {
        const auto sampleEnd { std::clamp (text.getLargeIntValue (), zoneProperties.getSampleStart ().value_or (0), sampleLength)};
        sampleEndUiChanged (sampleEnd);
        sampleEndTextEditor.setText (juce::String (sampleEnd));
    });
    // LOOP START
    setupLabel (loopStartLabel, "LOOP START", 12.0, juce::Justification::centredRight);
    setupTextEditor (loopStartTextEditor, juce::Justification::centred, 0, "0123456789", "LoopStart", [this] ()
    {
        if (! loopLengthIsEnd)
            FormatHelpers::setColorIfError (loopStartTextEditor, minZoneProperties.getLoopStart ().value_or (0),
                                            sampleLength - static_cast<int64_t> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
        else
            FormatHelpers::setColorIfError (loopStartTextEditor, minZoneProperties.getLoopStart ().value_or (0),
                                            zoneProperties.getLoopStart ().value_or (0) + static_cast<int64_t> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
    },
    [this] (juce::String text)
    {
        const auto loopStart = [this, text] ()
        {
            if (! loopLengthIsEnd)
                return std::clamp (text.getLargeIntValue (), minZoneProperties.getLoopStart ().value_or (0),
                                   sampleLength - static_cast<int64_t> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
            else
                return std::clamp (text.getLargeIntValue (), minZoneProperties.getLoopStart ().value_or (0),
                                   zoneProperties.getLoopStart ().value_or (0) + static_cast<int64_t> (zoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
        } ();
        loopStartUiChanged (loopStart);
        loopStartTextEditor.setText (juce::String (loopStart));
        if (loopLengthIsEnd)
        {
            const auto loopLength { loopLengthTextEditor.getText ().getDoubleValue () - static_cast<double> (zoneProperties.getLoopStart ().value_or (0)) };
            loopLengthUiChanged (loopLength);
        }
    });
    // LOOP LENGTH
    setupLabel (loopLengthLabel, "LOOP LENGTH", 12.0, juce::Justification::centredRight);
    setupTextEditor (loopLengthTextEditor, juce::Justification::centred, 0, ".0123456789", "LoopLength", [this] ()
    {
        auto loopLengthInput = [this, text = loopLengthTextEditor.getText ()] ()
        {
            if (loopLengthIsEnd)
                return text.getDoubleValue () - zoneProperties.getLoopStart ().value_or (0);
            else
                return text.getDoubleValue ();
        } ();
        FormatHelpers::setColorIfError (loopLengthTextEditor, loopLengthInput,
                                        sampleLength == 0 ? 0.0 : minZoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0))),
                                        static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)));
    },
    [this] (juce::String text)
    {
        auto loopLengthInput = [this, text] ()
        {
            if (loopLengthIsEnd)
                return text.getDoubleValue () - zoneProperties.getLoopStart ().value_or (0);
            else
                return text.getDoubleValue ();
        } ();
        // constrain
        auto loopLength { std::clamp (loopLengthInput,
                                      minZoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0))),
                                      (sampleLength == 0 ? minZoneProperties.getLoopLength ().value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0))) :
                                                           static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)))) };
        // snap
        loopLength = snapLoopLength (loopLength);

        loopLengthUiChanged (loopLength);
        loopLengthTextEditor.setText (formatLoopLength (loopLength));
    });
    setupLabel (minVoltageLabel, "MIN VOLTAGE", 15.0, juce::Justification::centredRight);
    // MIN VOLTAGE
    setupTextEditor (minVoltageTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MinVoltage", [this] ()
    {
        FormatHelpers::setColorIfError (minVoltageTextEditor, minZoneProperties.getMinVoltage (), maxZoneProperties.getMinVoltage ());
        if (isMinVoltageInRange != nullptr)
            FormatHelpers::setColorIfError (minVoltageTextEditor, isMinVoltageInRange (minVoltageTextEditor.getText ().getDoubleValue ()));
    },
    [this] (juce::String text)
    {
        auto minVoltage { std::clamp (text.getDoubleValue (), minZoneProperties.getMinVoltage (), maxZoneProperties.getMinVoltage ()) };
        if (clampMinVoltage != nullptr)
            minVoltage = clampMinVoltage (text.getDoubleValue ());
        minVoltageUiChanged (minVoltage);
        minVoltageTextEditor.setText (FormatHelpers::formatDouble (minVoltage, 2, true));
    });
    setupLabel (pitchOffsetLabel, "PITCH OFFSET", 15.0, juce::Justification::centredRight);
    setupTextEditor (pitchOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", "PitchOffset", [this] ()
    {
        FormatHelpers::setColorIfError (pitchOffsetTextEditor, minZoneProperties.getPitchOffset (), maxZoneProperties.getPitchOffset ());
    },
    [this] (juce::String text)
    {
        const auto pitchOffset { std::clamp (text.getDoubleValue (), minZoneProperties.getPitchOffset (), maxZoneProperties.getPitchOffset ()) };
        pitchOffsetUiChanged (pitchOffset);
        pitchOffsetTextEditor.setText (FormatHelpers::formatDouble (pitchOffset, 2, true));
    });
    setupLabel (levelOffsetLabel, "LEVEL OFFSET", 15.0, juce::Justification::centredRight);
    setupTextEditor (levelOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", "LevelOffset", [this] ()
    {
        FormatHelpers::setColorIfError (levelOffsetTextEditor, minZoneProperties.getLevelOffset (), maxZoneProperties.getLevelOffset ());
    },
    [this] (juce::String text)
    {
        const auto levelOffset { std::clamp (text.getDoubleValue (), minZoneProperties.getLevelOffset (), maxZoneProperties.getLevelOffset ()) };
        levelOffsetUiChanged (levelOffset);
        levelOffsetTextEditor.setText (FormatHelpers::formatDouble (levelOffset, 1, true));
    });
}

void ZoneEditor::init (juce::ValueTree zonePropertiesVT, juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);

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
    loopLengthIsEnd = newLoopLengthIsEnd;
    if (! loopLengthIsEnd)
    {
        loopLengthLabel.setText ("LOOP LENGTH", juce::NotificationType::dontSendNotification);
        loopLengthTextEditor.setInputRestrictions (0, ".0123456789");
    }
    else
    {
        loopLengthLabel.setText ("LOOP END", juce::NotificationType::dontSendNotification);
        loopLengthTextEditor.setInputRestrictions (0, "0123456789");
    }
    // reformat the UI string
    loopLengthDataChanged (zoneProperties.getLoopLength ());
}

void ZoneEditor::receiveSampleLoadRequest (juce::File sampleFile)
{
    if (! handleSelectedFile (sampleFile))
    {
        // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
    }
}

void ZoneEditor::setupZonePropertiesCallbacks ()
{
    zoneProperties.onIdChange = [this] ([[maybe_unused]] int id) { jassertfalse; /* I don't think this should change while we are editing */};
    zoneProperties.onLevelOffsetChange = [this] (double levelOffset) { levelOffsetDataChanged (levelOffset);  };
    zoneProperties.onLoopLengthChange = [this] (std::optional<double> loopLength) { loopLengthDataChanged (loopLength);  };
    zoneProperties.onLoopStartChange = [this] (std::optional <int64_t> loopStart) { loopStartDataChanged (loopStart);  };
    zoneProperties.onMinVoltageChange = [this] (double minVoltage) { minVoltageDataChanged (minVoltage);  };
    zoneProperties.onPitchOffsetChange = [this] (double pitchOffset) { pitchOffsetDataChanged (pitchOffset);  };
    zoneProperties.onSampleChange = [this] (juce::String sample) { sampleDataChanged (sample);  };
    zoneProperties.onSampleStartChange = [this] (std::optional <int64_t> sampleStart) { sampleStartDataChanged (sampleStart);  };
    zoneProperties.onSampleEndChange = [this] (std::optional <int64_t> sampleEnd) { sampleEndDataChanged (sampleEnd);  };
}

void ZoneEditor::paint ([[maybe_unused]] juce::Graphics& g)
{
//     g.setColour (juce::Colours::magenta);
//     g.drawRect (getLocalBounds ());
}

void ZoneEditor::paintOverChildren (juce::Graphics& g)
{
    juce::Colour fillColor { juce::Colours::white };
    float alpha { 0.2f };
    if (draggingFiles)
        g.fillAll (fillColor.withAlpha (alpha));
}

void ZoneEditor::resized ()
{
    const auto xOffset { 3 };
    const auto width { 160 };
    const auto interParameterYOffset { 1 };
    const auto spaceBetweenLabelAndInput { 3 };
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int> (width * scaleAmount); };

    //dropTargetOverlay.setBounds (getLocalBounds ());

    if (displayToolsMenu != nullptr)
        toolsButton.setBounds (getWidth () - 5 - 40, getHeight () - 5 - 20, 40, 20);

    const auto sampleNameLabelScale { 0.156f };
    const auto sampleNameInputScale { 1.f - sampleNameLabelScale };
    sampleNameLabel.setBounds (xOffset, 5, scaleWidth (sampleNameLabelScale), 20);
    sampleNameSelectLabel.setBounds (sampleNameLabel.getRight () + spaceBetweenLabelAndInput, 5, scaleWidth (sampleNameInputScale) - spaceBetweenLabelAndInput + 1, 20);

    const auto samplePointLabelScale { 0.45f };
    const auto samplePointInputScale { 1.f - samplePointLabelScale };
    sampleStartLabel.setBounds (xOffset, sampleNameSelectLabel.getBottom () + 5, scaleWidth (samplePointLabelScale), 20);
    sampleStartTextEditor.setBounds (sampleStartLabel.getRight () + spaceBetweenLabelAndInput, sampleStartLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    sampleEndLabel.setBounds (xOffset, sampleStartLabel.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    sampleEndTextEditor.setBounds (sampleEndLabel.getRight () + spaceBetweenLabelAndInput, sampleEndLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    loopStartLabel.setBounds (xOffset, sampleEndTextEditor.getBottom (), scaleWidth (samplePointLabelScale), 20);
    loopStartTextEditor.setBounds (loopStartLabel.getRight () + spaceBetweenLabelAndInput, loopStartLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);
    loopLengthLabel.setBounds (xOffset, loopStartLabel.getBottom () + interParameterYOffset, scaleWidth (samplePointLabelScale), 20);
    loopLengthTextEditor.setBounds (loopLengthLabel.getRight ()+ spaceBetweenLabelAndInput, loopLengthLabel.getY (), scaleWidth (samplePointInputScale) - spaceBetweenLabelAndInput, 20);

    const auto otherLabelScale { 0.66f };
    const auto otherInputScale { 1.f - otherLabelScale };
    minVoltageLabel.setBounds (xOffset, loopLengthTextEditor.getBottom () + 5, scaleWidth (otherLabelScale), 20);
    minVoltageTextEditor.setBounds (minVoltageLabel.getRight () + spaceBetweenLabelAndInput, minVoltageLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);

    pitchOffsetLabel.setBounds (xOffset, minVoltageTextEditor.getBottom () + 5, scaleWidth (otherLabelScale), 20);
    pitchOffsetTextEditor.setBounds (pitchOffsetLabel.getRight () + spaceBetweenLabelAndInput, pitchOffsetLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);

    levelOffsetLabel.setBounds (xOffset, pitchOffsetLabel.getBottom () + 5, scaleWidth (otherLabelScale), 20);
    levelOffsetTextEditor.setBounds (levelOffsetLabel.getRight () + spaceBetweenLabelAndInput, levelOffsetLabel.getY (), scaleWidth (otherInputScale) - spaceBetweenLabelAndInput, 20);
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
    if (loopLengthIsEnd)
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

bool ZoneEditor::isSupportedAudioFile (juce::File file)
{
    if (file.isDirectory () || file.getFileExtension () != ".wav")
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
    loopLengthTextEditor.setText (formatLoopLength (loopLength.value_or (static_cast<double> (sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
}

void ZoneEditor::loopLengthUiChanged (double loopLength)
{
    zoneProperties.setLoopLength (loopLength, false);
}

void ZoneEditor::loopStartDataChanged (std::optional<int64_t> loopStart)
{
    loopStartTextEditor.setText (juce::String (loopStart.value_or (0)));
    loopLengthDataChanged (zoneProperties.getLoopLength ());
}

void ZoneEditor::loopStartUiChanged (int64_t  loopStart)
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
    sampleLength = 0;
    auto textColor {juce::Colours::white};
    auto sampleFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (sample) };
    if (sampleFile.exists ())
    {
        if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile)); reader != nullptr)
            sampleLength = reader->lengthInSamples;
        if (! zoneProperties.getSampleEnd ().has_value ())
            sampleEndTextEditor.setText (juce::String (sampleLength));
        if (! zoneProperties.getLoopLength ().has_value ())
            loopLengthTextEditor.setText (formatLoopLength (static_cast<double> (sampleLength)));
    }
    else
    {
        textColor = juce::Colours::red;
    }
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
    if (sample != sampleNameSelectLabel.getText ())
    {
        updateSampleFileInfo (sample);
        updateSamplePositionInfo ();
    }
    sampleNameSelectLabel.setText (sample, juce::NotificationType::dontSendNotification);
}

void ZoneEditor::sampleUiChanged (juce::String sample)
{
    zoneProperties.setSample (sample, false);
}

void ZoneEditor::sampleStartDataChanged (std::optional<int64_t> sampleStart)
{
    sampleStartTextEditor.setText (juce::String (sampleStart.value_or (0)));
}

void ZoneEditor::sampleStartUiChanged (int64_t  sampleStart)
{
    zoneProperties.setSampleStart (sampleStart, false);
}

void ZoneEditor::sampleEndDataChanged (std::optional<int64_t> sampleEnd)
{
    sampleEndTextEditor.setText (juce::String (sampleEnd.value_or (sampleLength)));
}

void ZoneEditor::sampleEndUiChanged (int64_t  sampleEnd)
{
    zoneProperties.setSampleEnd (sampleEnd, false);
}
