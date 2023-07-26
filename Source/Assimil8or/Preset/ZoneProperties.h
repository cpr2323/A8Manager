#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class Assimil8orZoneProperties : public ValueTreeWrapper
{
public:
    Assimil8orZoneProperties () noexcept : ValueTreeWrapper (ZoneTypeId) {}

    void setLevelOffset (float levelOffset, bool includeSelfCallback);
    void setLoopLength (float loopLength, bool includeSelfCallback);
    void setLoopStart (int loopStart, bool includeSelfCallback);
    void setMinVoltage (float minVoltage, bool includeSelfCallback);
    void setPitchOffset (float pitchOffset, bool includeSelfCallback);
    void setSample (juce::String sampleFileName, bool includeSelfCallback);
    void setSampleStart (int sampleStart, bool includeSelfCallback);
    void setSampleEnd (int sampleEnd, bool includeSelfCallback);
    void setSide (int side, bool includeSelfCallback);

    float getLevelOffset ();
    float getLoopLength ();
    int getLoopStart ();
    float getMinVoltage ();
    float getPitchOffset ();
    juce::String getSample ();
    int getSampleStart ();
    int getSampleEnd ();
    int getSide ();

    std::function<void (float levelOffset)> onLevelOffsetChange;
    std::function<void (float loopLength)> onLoopLengthChange;
    std::function<void (int loopStart)> onLoopStartChange;
    std::function<void (float minVoltage)> onMinVoltageChange;
    std::function<void (float pitchOffset)> onPitchOffsetChange;
    std::function<void (juce::String sampleFileName)> onSampleChange;
    std::function<void (int sampleStart)> onSampleStartChange;
    std::function<void (int sampleEnd)> onSampleEndChange;
    std::function<void (int side)> onSideChange;

    static inline const juce::Identifier ZoneTypeId { "Zone" };
    static inline const juce::Identifier LevelOffsetPropertyId { "levelOffset" };
    static inline const juce::Identifier LoopLengthPropertyId  { "loopLength" };
    static inline const juce::Identifier LoopStartPropertyId   { "loopStart" };
    static inline const juce::Identifier MinVoltagePropertyId  { "minVoltage" };
    static inline const juce::Identifier PitchOffsetPropertyId { "pitchOffset" };
    static inline const juce::Identifier SamplePropertyId      { "sample" };
    static inline const juce::Identifier SampleStartPropertyId { "sampleStart" };
    static inline const juce::Identifier SampleEndPropertyId   { "sampleEnd" };
    static inline const juce::Identifier SidePropertyId        { "side" };

private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
