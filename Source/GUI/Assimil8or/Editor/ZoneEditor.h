#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "../../../AppProperties.h"

class ZoneEditor : public juce::Component
{
public:
    ZoneEditor ();
    ~ZoneEditor () = default;

    void init (juce::ValueTree zonePropertiesVT, juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;
    ZoneProperties zoneProperties;
    ZoneProperties minZoneProperties;
    ZoneProperties maxZoneProperties;

    juce::Label levelOffsetLabel;
    juce::TextEditor levelOffsetTextEditor; // double
    juce::TextEditor loopLengthTextEditor; // double
    juce::Label loopBoundsLabel;
    juce::TextEditor loopStartTextEditor; // int
    juce::Label minVoltageLabel;
    juce::TextEditor minVoltageTextEditor; // double
    juce::Label pitchOffsetLabel;
    juce::TextEditor pitchOffsetTextEditor; // double
    juce::Label sampleStartNameLabel;
    juce::TextEditor sampleTextEditor; // filename
    juce::Label sampleBoundsLabel;
    juce::TextEditor sampleStartTextEditor; // int
    juce::TextEditor sampleEndTextEditor; // int
    juce::Label sideLabel;
    juce::TextButton sideButton; // ?

    void setupChannelComponents ();
    void setupZonePropertiesCallbacks ();

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
    void sampleDataChanged (juce::String sample);
    void sampleUiChanged (juce::String sample);
    void sampleStartDataChanged (int sampleStart);
    void sampleStartUiChanged (int sampleStart);
    void sampleEndDataChanged (int sampleEnd);
    void sampleEndUiChanged (int sampleEnd);
    void sideDataChanged (int side);
    void sideUiChanged (int side);

    void paint (juce::Graphics& g) override;
    void resized () override;
};
