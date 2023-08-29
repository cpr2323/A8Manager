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
        minZoneProperties.wrap(minChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }
    {
        PresetProperties maxPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        ChannelProperties maxChannelProperties (maxPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        maxZoneProperties.wrap (maxChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
    }

    sampleNameSelectLabel.onCheckInterest = [this] (const juce::StringArray& files)
    {
        if (files.size () != 1)
            return false;
        const auto draggedFile { juce::File (files [0]) };
        return isSupportedAudioFile (draggedFile);
    };
    sampleNameSelectLabel.onFilesSelected = [this] (const juce::StringArray& files)
    {
        sampleNameSelectLabel.setOutline (levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::outlineColourId));
        sampleNameSelectLabel.repaint ();
        
        if (files.size () != 1)
            return;
        if (! handleSelectedFile (juce::File (files [0])))
        {
            // TODO - indicate an error? first thought was a red outline that fades out over a couple of second
        }
    };
    sampleNameSelectLabel.onDragEnter = [this] (const juce::StringArray& files)
    {
        if (files.size () != 1)
            sampleNameSelectLabel.setOutline(juce::Colours::red);
        else
        {
            const auto draggedFile { juce::File (files [0]) };
            if (isSupportedAudioFile (draggedFile))
                sampleNameSelectLabel.setOutline (juce::Colours::white);
            else
                sampleNameSelectLabel.setOutline (juce::Colours::red);
        }
        sampleNameSelectLabel.repaint ();
    };
    sampleNameSelectLabel.onDragExit = [this] (const juce::StringArray&)
    {
        sampleNameSelectLabel.setOutline (levelOffsetTextEditor.findColour (juce::TextEditor::ColourIds::outlineColourId));
        sampleNameSelectLabel.repaint ();
    };

    deleteButton.setButtonText ("DEL");
    deleteButton.onClick = [this] ()
    {
        zoneProperties.setSample ("", true);
    };
    addAndMakeVisible (deleteButton);
    setupZoneComponents ();
}

bool ZoneEditor::handleSelectedFile (juce::File fileNameAndPath)
{
    if (! isSupportedAudioFile (fileNameAndPath))
        return false;

    // if file not in preset folder, then copy
    if (appProperties.getMostRecentFolder () != fileNameAndPath.getParentDirectory ().getFullPathName ())
    {
        // TODO handle case where file of same name already exists

        // copy file
        fileNameAndPath.copyFileTo (juce::File (appProperties.getMostRecentFolder ()).getChildFile (fileNameAndPath.getFileName ()));
    }
    // assign file to zone
    loadSample (fileNameAndPath.getFileName ());

    return true;
}

void ZoneEditor::loadSample (juce::String sampleFileName)
{
    if (sampleFileName  == zoneProperties.getSample ())
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
            // if Zone1, and stereo file
            if (zoneProperties.getId () == 1 && reader->numChannels == 2)
            {
                ChannelProperties parentChannelProperties (zoneProperties.getValueTree ().getParent (), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                // if this zone not the last channel && the parent channel isn't set to Stereo/Right
                if (auto parentChannelIndex { parentChannelProperties.getId () }; parentChannelIndex != 8 && parentChannelProperties.getChannelMode () != ChannelProperties::ChannelMode::stereoRight)
                {
                    PresetProperties presetProperties (parentChannelProperties.getValueTree ().getParent (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
                    ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    ZoneProperties nextChannelZone1Properties (nextChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
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
    updateSamplePositionInfo ();
    deleteButton.setVisible (sampleFileName.isNotEmpty ());

    sampleUiChanged (sampleFileName);
    sampleNameSelectLabel.setText (sampleFileName, juce::NotificationType::dontSendNotification);
}

void ZoneEditor::setupZoneComponents ()
{
    juce::XmlDocument xmlDoc { BinaryData::Assimil8orToolTips_xml };
    auto xmlElement { xmlDoc.getDocumentElement (false) };
    if (auto parseError { xmlDoc.getLastParseError () }; parseError != "")
        juce::Logger::outputDebugString ("XML Parsing Error for Assimil8orToolTips_xml: " + parseError);
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
        const auto sampleStart { std::clamp (text.getLargeIntValue(), minZoneProperties.getSampleStart ().value_or (0), zoneProperties.getSampleEnd ().value_or (sampleLength)) };
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
                                            sampleLength - static_cast<int64_t>(zoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
        else
            FormatHelpers::setColorIfError (loopStartTextEditor, minZoneProperties.getLoopStart ().value_or (0),
                                            zoneProperties.getLoopStart ().value_or (0) + static_cast<int64_t>(zoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
    },
    [this] (juce::String text)
    {
        const auto loopStart = [this, text] ()
        {
            if (! loopLengthIsEnd)
                return std::clamp (text.getLargeIntValue (), minZoneProperties.getLoopStart ().value_or (0),
                                   sampleLength - static_cast<int64_t>(zoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
            else
                return std::clamp (text.getLargeIntValue (), minZoneProperties.getLoopStart ().value_or (0),
                                   zoneProperties.getLoopStart ().value_or (0) + static_cast<int64_t>(zoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
        }();
        loopStartUiChanged (loopStart);
        loopStartTextEditor.setText (juce::String (loopStart));
        if (loopLengthIsEnd)
        {
            const auto loopLength = loopLengthTextEditor.getText().getDoubleValue () - static_cast<double>(zoneProperties.getLoopStart ().value_or(0));
            loopLengthUiChanged (loopLength);
        }
    });
    // LOOP LENGTH
    setupLabel (loopLengthLabel, "LOOP LENGTH", 12.0, juce::Justification::centredRight);
    setupTextEditor (loopLengthTextEditor, juce::Justification::centred, 0, ".0123456789", "LoopLength", [this] ()
    {
        auto loopLengthInput = [this, text = loopLengthTextEditor.getText()] ()
        {
            if (loopLengthIsEnd)
                return text.getDoubleValue () - zoneProperties.getLoopStart ().value_or (0);
            else
                return text.getDoubleValue ();
        }();
        FormatHelpers::setColorIfError (loopLengthTextEditor, loopLengthInput, minZoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0))),
                                        static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0)));
    },
    [this] (juce::String text)
    {
        auto loopLengthInput = [this, text] ()
        {
            if (loopLengthIsEnd)
                return text.getDoubleValue () - zoneProperties.getLoopStart ().value_or (0);
            else
                return text.getDoubleValue ();
        }();
        // TODO - do additional clamping due on the different number of decimal places/increments based on the current value
        //   loopLength > 2048 | 0 decimal places
        //
        const auto loopLength = std::clamp (loopLengthInput, minZoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0))),
                                            (sampleLength == 0 ? minZoneProperties.getLoopLength ().value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0))) :
                                                                 static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0))));
        loopLengthUiChanged (loopLength);
        loopLengthTextEditor.setText (formatLoopLength (loopLength));
    });
    setupLabel (minVoltageLabel, "MIN VOLTAGE", 15.0, juce::Justification::centredRight);
    // MIN VOLTAGE
    setupTextEditor (minVoltageTextEditor, juce::Justification::centred, 0, "+-.0123456789", "MinVoltage", [this] ()
    {
        FormatHelpers::setColorIfError (minVoltageTextEditor, minZoneProperties.getMinVoltage (), maxZoneProperties.getMinVoltage ());
    },
    [this] (juce::String text)
    {
        const auto minVoltage { std::clamp (text.getDoubleValue (), minZoneProperties.getMinVoltage (), maxZoneProperties.getMinVoltage ()) };
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

    ChannelProperties channelProperties (zoneProperties.getValueTree().getParent (), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    setLoopLengthIsEnd (channelProperties.getLoopLengthIsEnd ());
}

void ZoneEditor::setLoopLengthIsEnd (bool newLoopLengthIsEnd)
{
    //loopBoundsLabel
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
    zoneProperties.onIdChange = [this] ([[maybe_unused]] int index) { jassertfalse; /* I don't think this should change while we are editing */};
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

void ZoneEditor::resized ()
{
    const auto xOffset { 3 };
    const auto width { 160 };
    const auto interParameterYOffset { 1 };
    const auto spaceBetweenLabelAndInput { 3 };
    auto scaleWidth = [width] (float scaleAmount) { return static_cast<int>(width * scaleAmount); };

    deleteButton.setBounds (getWidth() - 5 - 40, getHeight() - 5 - 20, 40, 20);

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

juce::String ZoneEditor::formatLoopLength (double loopLength)
{
    if (loopLengthIsEnd)
        loopLength = static_cast<int>(static_cast<double>(zoneProperties.getLoopStart().value_or (0)) + loopLength);

    if (loopLength < 2048.0)
        return FormatHelpers::formatDouble (loopLength, 3, false);
    else
        return juce::String (static_cast<int>(loopLength));
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
    loopLengthTextEditor.setText (formatLoopLength (loopLength.value_or (static_cast<double>(sampleLength - zoneProperties.getLoopStart ().value_or (0)))));
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
    auto sampleFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (sample) };
    if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile)); reader != nullptr)
        sampleLength = reader->lengthInSamples;
    else
        sampleLength = 0;
    if (! zoneProperties.getSampleEnd().has_value() )
        sampleEndTextEditor.setText (juce::String (sampleLength));
    if (! zoneProperties.getLoopLength ().has_value ())
        loopLengthTextEditor.setText (formatLoopLength (static_cast<double>(sampleLength)));
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
    deleteButton.setVisible (sample.isNotEmpty ());
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
