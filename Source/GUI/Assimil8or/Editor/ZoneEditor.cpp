#include "ZoneEditor.h"
#include "FormatHelpers.h"
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

    setupZoneComponents ();
}

void ZoneEditor::setupZoneComponents ()
{
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
    auto setupTextEditor = [this] (juce::TextEditor& textEditor, juce::Justification justification, int maxLen, juce::String validInputCharacters,
                                   std::function<void ()> validateCallback, std::function<void (juce::String)> doneEditingCallback)
    {
        jassert (doneEditingCallback != nullptr);
        textEditor.setJustification (justification);
        textEditor.setIndents (2, 0);
        textEditor.setInputRestrictions (maxLen, validInputCharacters);
        textEditor.onFocusLost = [this, &textEditor, doneEditingCallback] () { doneEditingCallback (textEditor.getText ()); };
        textEditor.onReturnKey = [this, &textEditor, doneEditingCallback] () { doneEditingCallback (textEditor.getText ()); };
        if (validateCallback != nullptr)
            textEditor.onTextChange = [this, validateCallback] () { validateCallback (); };
        addAndMakeVisible (textEditor);
    };
    auto setupButton = [this] (juce::TextButton& textButton, juce::String text, std::function<void ()> onClickCallback)
    {
        textButton.setButtonText (text);
        textButton.setClickingTogglesState (true);
        textButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, textButton.findColour (juce::TextButton::ColourIds::buttonOnColourId).brighter (0.5));
        textButton.onClick = onClickCallback;
        addAndMakeVisible (textButton);
    };
    setupLabel (sampleStartNameLabel, "FILE", 15.0, juce::Justification::centredLeft);
    const auto kMaxFileNameLength { 47 };
    setupTextEditor (sampleTextEditor, juce::Justification::centredLeft, kMaxFileNameLength, " !\"#$%^&'()#+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", [this] ()
    {
        auto sampleFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (sampleTextEditor.getText ()) };
        if (! sampleFile.isDirectory ())
        {
            if (sampleFile.getFileExtension () != "")
                FormatHelpers::setColorIfError (sampleTextEditor, sampleFile);
            else
                FormatHelpers::setColorIfError (sampleTextEditor, sampleFile.withFileExtension(".wav"));
        }
        else
        {
            FormatHelpers::setColorIfError (sampleTextEditor, false);
        }

    },
    [this] (juce::String text)
    {
        auto sampleFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (sampleTextEditor.getText ()) };
        if (text.isNotEmpty ())
        {
            if (sampleFile.getFileExtension () == "")
                sampleFile = sampleFile.withFileExtension (".wav");
            text = sampleFile.getFileName ();

            if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (sampleFile)); reader != nullptr)
            {
                // if Zone1, and stereo file
                if (zoneProperties.getIndex () == 1 && reader->numChannels == 2)
                {
                    ChannelProperties parentChannelProperties (zoneProperties.getValueTree ().getParent (), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                    // if this zone is on the last channel
                    if (auto parentChannelIndex { parentChannelProperties.getIndex () }; parentChannelIndex != 8 && parentChannelProperties.getChannelMode() != ChannelProperties::ChannelMode::stereoRight)
                    {
                        PresetProperties presetProperties (parentChannelProperties.getValueTree ().getParent (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
                        ChannelProperties nextChannelProperties (presetProperties.getChannelVT (parentChannelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                        ZoneProperties nextChannelZone1Properties (nextChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                        // if next Channel does not have a sample
                        if (nextChannelZone1Properties.getSample ().isEmpty ())
                        {
                            //   set that Channel Mode to Stereo/Right for next Channel
                            nextChannelProperties.setChannelMode (ChannelProperties::ChannelMode::stereoRight, false);
                            //   set this file as the sample for next Channel
                            nextChannelZone1Properties.setSample (text, false);
                            //   set Zone Side to 1 for Zone in next Channel
                            nextChannelZone1Properties.setSide (1, false);
                        }
                    }
                }
            }
        }
        sampleUiChanged (text);
        sampleTextEditor.setText (text);
    });
    setupLabel (sampleBoundsLabel, "SAMPLE START/END", 15.0, juce::Justification::centredLeft);
    setupTextEditor (sampleStartTextEditor, juce::Justification::centred, 0, "0123456789", [this] ()
    {
        if (minZoneProperties.getSampleStart () <= maxZoneProperties.getSampleStart ())
            FormatHelpers::setColorIfError (sampleStartTextEditor, minZoneProperties.getSampleStart (), maxZoneProperties.getSampleStart ());
        else
            FormatHelpers::setColorIfError (sampleStartTextEditor, false);
    },
    [this] (juce::String text)
    {
        const auto sampleStart { std::clamp (text.getIntValue (), minZoneProperties.getSampleStart (), maxZoneProperties.getSampleStart ()) };
        sampleStartUiChanged (sampleStart);
        sampleStartTextEditor.setText (juce::String (sampleStart));
    });
    setupTextEditor (sampleEndTextEditor, juce::Justification::centred, 0, "0123456789", [this] ()
    {
        if (minZoneProperties.getSampleEnd () <= maxZoneProperties.getSampleEnd ())
            FormatHelpers::setColorIfError (sampleEndTextEditor, minZoneProperties.getSampleEnd (), maxZoneProperties.getSampleEnd ());
        else
            FormatHelpers::setColorIfError (sampleEndTextEditor, false);
    },
    [this] (juce::String text)
    {
        const auto sampleEnd { std::clamp (text.getIntValue (), minZoneProperties.getSampleEnd (), maxZoneProperties.getSampleEnd ()) };
        sampleEndUiChanged (sampleEnd);
        sampleEndTextEditor.setText (juce::String (sampleEnd));
    });
    setupLabel (loopBoundsLabel, "LOOP START/LENGTH", 15.0, juce::Justification::centredLeft);
    setupTextEditor (loopStartTextEditor, juce::Justification::centred, 0, "0123456789", [this] ()
    {
        if (minZoneProperties.getLoopStart () <= maxZoneProperties.getLoopStart ())
            FormatHelpers::setColorIfError (loopStartTextEditor, minZoneProperties.getLoopStart (), maxZoneProperties.getLoopStart ());
        else
            FormatHelpers::setColorIfError (loopStartTextEditor, false);
    },
    [this] (juce::String text)
    {
        const auto loopStart { std::clamp (text.getIntValue (), minZoneProperties.getLoopStart (), maxZoneProperties.getLoopStart ()) };
        loopStartUiChanged (loopStart);
        loopStartTextEditor.setText (juce::String (loopStart));
    });
    setupTextEditor (loopLengthTextEditor, juce::Justification::centred, 0, ".0123456789", [this] ()
    {
        if (minZoneProperties.getLoopLength () <= maxZoneProperties.getLoopLength ())
            FormatHelpers::setColorIfError (loopLengthTextEditor, minZoneProperties.getLoopLength (), maxZoneProperties.getLoopLength ());
        else
            FormatHelpers::setColorIfError (loopLengthTextEditor, false);
    },
    [this] (juce::String text)
    {
        // TODO - after implementing LoopLength/LoopEnd switch, update code here to use it
        auto loopLength { std::clamp (text.getDoubleValue (), minZoneProperties.getLoopLength (), maxZoneProperties.getLoopLength ()) };
        // TODO - do additional clamping due on the different number of decimal places/increments based on the current value
        //   loopLength > 2048 | 0 decimal places
        //
        loopLengthUiChanged (loopLength);
        loopLengthTextEditor.setText (FormatHelpers::formatDouble (loopLength, 3, false));
    });
    setupLabel (minVoltageLabel, "MIN VOLTAGE", 15.0, juce::Justification::centredLeft);
    setupTextEditor (minVoltageTextEditor, juce::Justification::centred, 0, "+-.0123456789", [this] ()
    {
        FormatHelpers::setColorIfError (minVoltageTextEditor, minZoneProperties.getMinVoltage (), maxZoneProperties.getMinVoltage ());
    },
    [this] (juce::String text)
    {
        const auto minVoltage { std::clamp (text.getDoubleValue (), minZoneProperties.getMinVoltage (), maxZoneProperties.getMinVoltage ()) };
        minVoltageUiChanged (minVoltage);
        minVoltageTextEditor.setText (FormatHelpers::formatDouble (minVoltage, 2, true));
    });
    setupLabel (levelOffsetLabel, "LEVEL OFFSET", 15.0, juce::Justification::centredLeft);
    setupTextEditor (levelOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", [this] ()
    {
        FormatHelpers::setColorIfError (levelOffsetTextEditor, minZoneProperties.getLevelOffset (), maxZoneProperties.getLevelOffset ());
    },
    [this] (juce::String text)
    {
        const auto levelOffset { std::clamp (text.getDoubleValue (), minZoneProperties.getLevelOffset (), maxZoneProperties.getLevelOffset ()) };
        levelOffsetUiChanged (levelOffset);
        levelOffsetTextEditor.setText (FormatHelpers::formatDouble (levelOffset, 1, true));
    });
    setupLabel (pitchOffsetLabel, "PITCH OFFSET", 15.0, juce::Justification::centredLeft);
    setupTextEditor (pitchOffsetTextEditor, juce::Justification::centred, 0, "+-.0123456789", [this] ()
    {
        FormatHelpers::setColorIfError (pitchOffsetTextEditor, minZoneProperties.getPitchOffset (), maxZoneProperties.getPitchOffset ());
    },
    [this] (juce::String text)
    {
        const auto pitchOffset { std::clamp (text.getDoubleValue (), minZoneProperties.getPitchOffset (), maxZoneProperties.getPitchOffset ()) };
        pitchOffsetUiChanged (pitchOffset);
        pitchOffsetTextEditor.setText (FormatHelpers::formatDouble (pitchOffset, 2, true));
    });
    // TODO - this parameter should not be exposed. it is set based on Channel Mode setting 0=left, 1=right
    setupLabel (sideLabel, "SIDE", 15.0, juce::Justification::centredLeft);
    setupButton (sideButton, "x", [this] () { sideUiChanged (sideButton.getToggleState ()); });
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
    sideDataChanged (zoneProperties.getSide ());
}

void ZoneEditor::setupZonePropertiesCallbacks ()
{
    zoneProperties.onIndexChange = [this] ([[maybe_unused]] int index) { jassertfalse; /* I don't think this should change while we are editing */};
    zoneProperties.onLevelOffsetChange = [this] (double levelOffset) { levelOffsetDataChanged (levelOffset);  };
    zoneProperties.onLoopLengthChange = [this] (double loopLength) { loopLengthDataChanged (loopLength);  };
    zoneProperties.onLoopStartChange = [this] (int loopStart) { loopStartDataChanged (loopStart);  };
    zoneProperties.onMinVoltageChange = [this] (double minVoltage) { minVoltageDataChanged (minVoltage);  };
    zoneProperties.onPitchOffsetChange = [this] (double pitchOffset) { pitchOffsetDataChanged (pitchOffset);  };
    zoneProperties.onSampleChange = [this] (juce::String sample) { sampleDataChanged (sample);  };
    zoneProperties.onSampleStartChange = [this] (int sampleStart) { sampleStartDataChanged (sampleStart);  };
    zoneProperties.onSampleEndChange = [this] (int sampleEnd) { sampleEndDataChanged (sampleEnd);  };
    zoneProperties.onSideChange = [this] (int side) { sideDataChanged (side);  };
}

void ZoneEditor::paint ([[maybe_unused]] juce::Graphics& g)
{
//     g.setColour (juce::Colours::magenta);
//     g.drawRect (getLocalBounds ());
}

void ZoneEditor::resized ()
{
    const auto xOffset { 3 };
    const auto labelWidth { 150 };
    const auto interParameterYOffset { 1 };
    const auto inputXOffset { 5 };
    const auto inputYOffset { 0 };
    const auto inputWidth { labelWidth - 5};

    sampleStartNameLabel.setBounds (xOffset, 5, labelWidth, 20);
    sampleTextEditor.setBounds (sampleStartNameLabel.getX () + inputXOffset, sampleStartNameLabel.getBottom () + inputYOffset, inputWidth, 20);
    sampleBoundsLabel.setBounds (xOffset, sampleTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    sampleStartTextEditor.setBounds (sampleBoundsLabel.getX () + inputXOffset, sampleBoundsLabel.getBottom () + inputYOffset, inputWidth / 2, 20);
    sampleEndTextEditor.setBounds (sampleStartTextEditor.getRight() + 1, sampleStartTextEditor.getY (), inputWidth / 2, 20);
    loopBoundsLabel.setBounds (xOffset, sampleEndTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    loopStartTextEditor.setBounds (loopBoundsLabel.getX () + inputXOffset, loopBoundsLabel.getBottom () + inputYOffset, inputWidth / 2, 20);
    loopLengthTextEditor.setBounds (loopStartTextEditor.getRight () + 1, loopStartTextEditor.getY (), inputWidth / 2, 20);
    minVoltageLabel.setBounds (xOffset, loopLengthTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    minVoltageTextEditor.setBounds (minVoltageLabel.getX () + inputXOffset, minVoltageLabel.getBottom () + inputYOffset, inputWidth, 20);
    levelOffsetLabel.setBounds (xOffset, minVoltageTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    levelOffsetTextEditor.setBounds (levelOffsetLabel.getX () + inputXOffset, levelOffsetLabel.getBottom () + inputYOffset, inputWidth, 20);
    pitchOffsetLabel.setBounds (xOffset, levelOffsetTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    pitchOffsetTextEditor.setBounds (pitchOffsetLabel.getX () + inputXOffset, pitchOffsetLabel.getBottom () + inputYOffset, inputWidth, 20);
    sideLabel.setBounds (xOffset, pitchOffsetTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    sideButton.setBounds (sideLabel.getX () + 3, sideLabel.getBottom () + inputYOffset, inputWidth, 20);
}

void ZoneEditor::levelOffsetDataChanged (double levelOffset)
{
    levelOffsetTextEditor.setText (FormatHelpers::formatDouble (levelOffset, 1, true));
}

void ZoneEditor::levelOffsetUiChanged (double levelOffset)
{
    zoneProperties.setLevelOffset (levelOffset, false);
}

void ZoneEditor::loopLengthDataChanged (double loopLength)
{
    loopLengthTextEditor.setText (FormatHelpers::formatDouble (loopLength, 3, false));
}

void ZoneEditor::loopLengthUiChanged (double loopLength)
{
    zoneProperties.setLoopLength (loopLength, false);
}

void ZoneEditor::loopStartDataChanged (int loopStart)
{
    loopStartTextEditor.setText (juce::String (loopStart));
}

void ZoneEditor::loopStartUiChanged (int loopStart)
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

void ZoneEditor::sampleDataChanged (juce::String sample)
{
    sampleTextEditor.setText (sample);
}

void ZoneEditor::sampleUiChanged (juce::String sample)
{
    zoneProperties.setSample (sample, false);
}

void ZoneEditor::sampleStartDataChanged (int sampleStart)
{
    sampleStartTextEditor.setText (juce::String (sampleStart));
}

void ZoneEditor::sampleStartUiChanged (int sampleStart)
{
    zoneProperties.setSampleStart (sampleStart, false);
}

void ZoneEditor::sampleEndDataChanged (int sampleEnd)
{
    sampleEndTextEditor.setText (juce::String (sampleEnd));
}

void ZoneEditor::sampleEndUiChanged (int sampleEnd)
{
    zoneProperties.setSampleEnd (sampleEnd, false);
}

void ZoneEditor::sideDataChanged (int side)
{
    sideButton.setToggleState (side == 1, juce::NotificationType::dontSendNotification);
}

void ZoneEditor::sideUiChanged (int side)
{
    zoneProperties.setSide (side, false);
}
