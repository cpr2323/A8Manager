#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ZoneProperties.h"

class ZoneEditor : public juce::Component
{
public:
    ZoneEditor ();

private:
    ZoneProperties zoneProperties;

    juce::Label levelOffsetLabel;
    juce::TextEditor levelOffsetTextEditor; // double
    juce::Label loopLengthLabel;
    juce::TextEditor loopLengthTextEditor; // double
    juce::Label loopStartLabel;
    juce::TextEditor loopStartTextEditor; // int
    juce::Label minVoltageLabel;
    juce::TextEditor minVoltageTextEditor; // double
    juce::Label pitchOffsetLabel;
    juce::TextEditor pitchOffsetTextEditor; // double
    juce::Label sampleStartNameLabel;
    juce::TextEditor sampleFileNameTextEditor; // filename
    juce::Label sampleStartLabel;
    juce::TextEditor sampleStartTextEditor; // int
    juce::Label sampleEndLabel;
    juce::TextEditor sampleEndTextEditor; // int
    juce::Label sideLabel;
    juce::ToggleButton sideCheckBox; // ?

    void levelOffsetDataChanged (double levelOffset);
    void levelOffsetUiChanged (double levelOffset);
    void loopLengthDataChanged (double loopLength);
    void loopLengthUiChanged (double loopLength);
    void loopStartDataChanged (int loopStart);
    void loopStartUiChanged (int loopStart);
    void minVoltageDataChanged (double minVoltage);
    void minVoltageUiChanged (double minVoltage);
    void pitchOffsetDataChanged (double pitchOffset);
    void pitchOffsetUiChanged (double pitchOffset);
    void sampleFileNameDataChanged (juce::String sampleFileName);
    void sampleFileNameUiChanged (juce::String sampleFileName);
    void sampleStartDataChanged (int sampleStart);
    void sampleStartUiChanged (int sampleStart);
    void sampleEndDataChanged (int sampleEnd);
    void sampleEndUiChanged (int sampleEnd);
    void sideDataChanged (int side);
    void sideUiChanged (int side);

    void paint (juce::Graphics& g) override;
};
