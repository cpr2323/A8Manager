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
    void setLoopLengthIsEnd (bool loopLengthIsEnd);

private:
    AppProperties appProperties;
    ZoneProperties zoneProperties;
    ZoneProperties minZoneProperties;
    ZoneProperties maxZoneProperties;
    juce::AudioFormatManager audioFormatManager;
    bool loopLengthIsEnd { false };

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

    int64_t sampleLength { 0 };
    void setupZoneComponents ();
    void setupZonePropertiesCallbacks ();
    void updateSampleFileInfo (juce::String sample);
    juce::String formatLoopLength (double loopLength);

    void levelOffsetDataChanged (double levelOffset);
    void levelOffsetUiChanged (double levelOffset);
    void loopLengthDataChanged (double loopLength);
    void loopLengthUiChanged (double loopLength);
    void loopStartDataChanged (int64_t loopStart);
    void loopStartUiChanged (int64_t loopStart);
    void minVoltageDataChanged (double minVoltage);
    void minVoltageUiChanged (double minVoltage);
    void pitchOffsetDataChanged (double pitchOffset);
    void pitchOffsetUiChanged (double pitchOffset);
    void sampleDataChanged (juce::String sample);
    void sampleUiChanged (juce::String sample);
    void sampleStartDataChanged (int64_t sampleStart);
    void sampleStartUiChanged (int64_t sampleStart);
    void sampleEndDataChanged (int64_t sampleEnd);
    void sampleEndUiChanged (int64_t sampleEnd);

    void paint (juce::Graphics& g) override;
    void resized () override;
};
