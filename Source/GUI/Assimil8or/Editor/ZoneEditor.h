#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ZoneProperties.h"

class ZoneEditor : public juce::Component
{
public:
private:
    ZoneProperties zoneProperties;

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
};
