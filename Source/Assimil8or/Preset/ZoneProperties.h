#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ZoneProperties : public ValueTreeWrapper<ZoneProperties>
{
public:
    ZoneProperties () noexcept : ValueTreeWrapper<ZoneProperties> (ZoneTypeId) {}

    void setIndex (int index, bool includeSelfCallback);
    void setLevelOffset (double levelOffset, bool includeSelfCallback);
    void setLoopLength (double loopLength, bool includeSelfCallback);
    void setLoopStart (int loopStart, bool includeSelfCallback);
    void setMinVoltage (double minVoltage, bool includeSelfCallback);
    void setPitchOffset (double pitchOffset, bool includeSelfCallback);
    void setSample (juce::String sampleFileName, bool includeSelfCallback);
    void setSampleStart (int sampleStart, bool includeSelfCallback);
    void setSampleEnd (int sampleEnd, bool includeSelfCallback);
    void setSide (int side, bool includeSelfCallback);

    int getIndex ();
    double getLevelOffset ();
    double getLoopLength ();
    int getLoopStart ();
    double getMinVoltage ();
    double getPitchOffset ();
    juce::String getSample ();
    int getSampleStart ();
    int getSampleEnd ();
    int getSide ();

    std::function<void (int index)> onIndexChange;
    std::function<void (double levelOffset)> onLevelOffsetChange;
    std::function<void (double loopLength)> onLoopLengthChange;
    std::function<void (int loopStart)> onLoopStartChange;
    std::function<void (double minVoltage)> onMinVoltageChange;
    std::function<void (double pitchOffset)> onPitchOffsetChange;
    std::function<void (juce::String sampleFileName)> onSampleChange;
    std::function<void (int sampleStart)> onSampleStartChange;
    std::function<void (int sampleEnd)> onSampleEndChange;
    std::function<void (int side)> onSideChange;

    static juce::ValueTree create (int index);

    static inline const juce::Identifier ZoneTypeId { "Zone" };
    static inline const juce::Identifier IndexPropertyId       { "_index" };
    static inline const juce::Identifier LevelOffsetPropertyId { "levelOffset" };
    static inline const juce::Identifier LoopLengthPropertyId  { "loopLength" };
    static inline const juce::Identifier LoopStartPropertyId   { "loopStart" };
    static inline const juce::Identifier MinVoltagePropertyId  { "minVoltage" };
    static inline const juce::Identifier PitchOffsetPropertyId { "pitchOffset" };
    static inline const juce::Identifier SamplePropertyId      { "sample" };
    static inline const juce::Identifier SampleStartPropertyId { "sampleStart" };
    static inline const juce::Identifier SampleEndPropertyId   { "sampleEnd" };
    static inline const juce::Identifier SidePropertyId        { "side" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
