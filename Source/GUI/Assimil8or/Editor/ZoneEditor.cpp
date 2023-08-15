#include "ZoneEditor.h"

ZoneEditor::ZoneEditor ()
{
//     juce::Label levelOffsetLabel;
//     juce::TextEditor levelOffsetTextEditor; // double
//     juce::Label loopLengthLabel;
//     juce::TextEditor loopLengthTextEditor; // double
//     juce::Label loopStartLabel;
//     juce::TextEditor loopStartTextEditor; // int
//     juce::Label minVoltageLabel;
//     juce::TextEditor minVoltageTextEditor; // double
//     juce::Label pitchOffsetLabel;
//     juce::TextEditor pitchOffsetTextEditor; // double
//     juce::Label sampleStartNameLabel;
//     juce::TextEditor sampleFileNameTextEditor; // filename
//     juce::Label sampleStartLabel;
//     juce::TextEditor sampleStartTextEditor; // int
//     juce::Label sampleEndLabel;
//     juce::TextEditor sampleEndTextEditor; // int
//     juce::Label sideLabel;
//     juce::ToggleButton sideCheckBox; // ?
}

void ZoneEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::magenta);
    g.drawRect (getLocalBounds ());
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

void ZoneEditor::sampleFileNameDataChanged (juce::String sampleFileName)
{
    sampleFileNameTextEditor.setText (sampleFileName);
}

void ZoneEditor::sampleFileNameUiChanged (juce::String sampleFileName)
{
    zoneProperties.setSample (sampleFileName, false);
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
    sideCheckBox.setToggleState (side == 1, juce::NotificationType::dontSendNotification);
}

void ZoneEditor::sideUiChanged (int side)
{
    zoneProperties.setSide (side, false);
}
