#include "ZoneEditor.h"

ZoneEditor::ZoneEditor ()
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
    auto setupTextEditor = [this] (juce::TextEditor& textEditor, juce::Justification justification, std::function<void (juce::String)> textEditedCallback)
    {
        jassert (textEditedCallback != nullptr);
        textEditor.setJustification (justification);
        textEditor.setIndents (0, 0);
        textEditor.onFocusLost = [this, &textEditor, textEditedCallback] () { textEditedCallback (textEditor.getText ()); };
        textEditor.onReturnKey = [this, &textEditor, textEditedCallback] () { textEditedCallback (textEditor.getText ()); };
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
    setupTextEditor (sampleTextEditor, juce::Justification::centredLeft, [this] (juce::String text) { sampleUiChanged (text); });
    
    setupLabel (levelOffsetLabel, "LEVEL OFFSET", 15.0, juce::Justification::centredLeft);
    setupTextEditor (levelOffsetTextEditor, juce::Justification::centred, [this] (juce::String text) { levelOffsetUiChanged (text.getDoubleValue ()); });

    setupLabel (loopStartLabel, "LOOP START", 15.0, juce::Justification::centredLeft);
    setupTextEditor (loopStartTextEditor, juce::Justification::centred, [this] (juce::String text) { loopStartUiChanged (text.getIntValue ()); });

    setupLabel (loopLengthLabel, "LOOP LENGTH", 15.0, juce::Justification::centredLeft);
    setupTextEditor (loopLengthTextEditor, juce::Justification::centred, [this] (juce::String text) { loopLengthUiChanged (text.getDoubleValue ()); });

    setupLabel (minVoltageLabel, "MIN VOLTAGE", 15.0, juce::Justification::centredLeft);
    setupTextEditor (minVoltageTextEditor, juce::Justification::centred, [this] (juce::String text) { minVoltageUiChanged (text.getDoubleValue ()); });

    setupLabel (pitchOffsetLabel, "PITCH OFFSET", 15.0, juce::Justification::centredLeft);
    setupTextEditor (pitchOffsetTextEditor, juce::Justification::centred, [this] (juce::String text) { pitchOffsetUiChanged (text.getDoubleValue ()); });

    setupLabel (sampleStartLabel, "SAMPLE START", 15.0, juce::Justification::centredLeft);
    setupTextEditor (sampleStartTextEditor, juce::Justification::centred, [this] (juce::String text) { sampleStartUiChanged (text.getIntValue ()); });

    setupLabel (sampleEndLabel, "SAMPLE START", 15.0, juce::Justification::centredLeft);
    setupTextEditor (sampleEndTextEditor, juce::Justification::centred, [this] (juce::String text) { sampleEndUiChanged (text.getIntValue ()); });

    setupLabel (sideLabel, "SIDE", 15.0, juce::Justification::centredLeft);
    setupButton (sideButton, "x", [this] () { sideUiChanged (sideButton.getToggleState ()); });
}

void ZoneEditor::init (juce::ValueTree zonePropertiesVT)
{
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
    sampleStartLabel.setBounds (xOffset, sampleTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    sampleStartTextEditor.setBounds (sampleStartLabel.getX () + inputXOffset, sampleStartLabel.getBottom () + inputYOffset, inputWidth, 20);
    sampleEndLabel.setBounds (xOffset, sampleStartTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    sampleEndTextEditor.setBounds (sampleEndLabel.getX () + inputXOffset, sampleEndLabel.getBottom () + inputYOffset, inputWidth, 20);
    loopStartLabel.setBounds (xOffset, sampleEndTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    loopStartTextEditor.setBounds (loopStartLabel.getX () + inputXOffset, loopStartLabel.getBottom () + inputYOffset, inputWidth, 20);
    loopLengthLabel.setBounds (xOffset, loopStartTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    loopLengthTextEditor.setBounds (loopLengthLabel.getX () + inputXOffset, loopLengthLabel.getBottom () + inputYOffset, inputWidth, 20);
    minVoltageLabel.setBounds (xOffset, loopLengthTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    minVoltageTextEditor.setBounds (minVoltageLabel.getX () + inputXOffset, minVoltageLabel.getBottom () + inputYOffset, inputWidth, 20);
    levelOffsetLabel.setBounds (xOffset, minVoltageTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    levelOffsetTextEditor.setBounds (levelOffsetLabel.getX () + inputXOffset, levelOffsetLabel.getBottom () + inputYOffset, inputWidth, 20);
    pitchOffsetLabel.setBounds (xOffset, levelOffsetTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    pitchOffsetTextEditor.setBounds (pitchOffsetLabel.getX () + inputXOffset, pitchOffsetLabel.getBottom () + inputYOffset, inputWidth, 20);
    sideLabel.setBounds (xOffset, pitchOffsetTextEditor.getBottom () + interParameterYOffset, labelWidth, 20);
    sideButton.setBounds (sideLabel.getX () + 3, sideLabel.getBottom () + inputXOffset, inputWidth, 20);
}

void ZoneEditor::levelOffsetDataChanged (double levelOffset)
{
    levelOffsetTextEditor.setText (juce::String (levelOffset));
}

void ZoneEditor::levelOffsetUiChanged (double levelOffset)
{
    zoneProperties.setLevelOffset (levelOffset, false);
}

void ZoneEditor::loopLengthDataChanged (double loopLength)
{
    loopLengthTextEditor.setText (juce::String (loopLength));
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
    minVoltageTextEditor.setText (juce::String (minVoltage));
}

void ZoneEditor::minVoltageUiChanged (double minVoltage)
{
    zoneProperties.setMinVoltage (minVoltage, false);
}

void ZoneEditor::pitchOffsetDataChanged (double pitchOffset)
{
    pitchOffsetTextEditor.setText (juce::String (pitchOffset));
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
