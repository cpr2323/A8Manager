#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ZoneProperties : public ValueTreeWrapper<ZoneProperties>
{
public:
    ZoneProperties () noexcept : ValueTreeWrapper<ZoneProperties> (ZoneTypeId)
    {
    }
    ZoneProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ZoneProperties> (ZoneTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setId (int id, bool includeSelfCallback);
    void setLevelOffset (double levelOffset, bool includeSelfCallback);
    void setLoopLength (double loopLength, bool includeSelfCallback);
    void setLoopStart (juce::int64 loopStart, bool includeSelfCallback);
    void setMinVoltage (double minVoltage, bool includeSelfCallback);
    void setPitchOffset (double pitchOffset, bool includeSelfCallback);
    void setSample (juce::String sampleFileName, bool includeSelfCallback);
    void setSampleStart (juce::int64 sampleStart, bool includeSelfCallback);
    void setSampleEnd (juce::int64 sampleEnd, bool includeSelfCallback);
    void setSide (int side, bool includeSelfCallback);

    int getId ();
    double getLevelOffset ();
    std::optional<double> getLoopLength ();
    std::optional<juce::int64> getLoopStart ();
    double getMinVoltage ();
    double getPitchOffset ();
    juce::String getSample ();
    std::optional<juce::int64> getSampleStart ();
    std::optional<juce::int64> getSampleEnd ();
    int getSide ();

    std::function<void (int id)> onIdChange;
    std::function<void (double levelOffset)> onLevelOffsetChange;
    std::function<void (std::optional<double> loopLength)> onLoopLengthChange;
    std::function<void (std::optional<juce::int64> loopStart)> onLoopStartChange;
    std::function<void (double minVoltage)> onMinVoltageChange;
    std::function<void (double pitchOffset)> onPitchOffsetChange;
    std::function<void (juce::String sampleFileName)> onSampleChange;
    std::function<void (std::optional<juce::int64> sampleStart)> onSampleStartChange;
    std::function<void (std::optional<juce::int64> sampleEnd)> onSampleEndChange;
    std::function<void (int side)> onSideChange;

    void copyFrom (juce::ValueTree sourceVT);
    static juce::ValueTree create (int id);

    static inline const juce::Identifier ZoneTypeId { "Zone" };
    static inline const juce::Identifier IdPropertyId          { "_id" };
    static inline const juce::Identifier LevelOffsetPropertyId { "levelOffset" };
    static inline const juce::Identifier LoopLengthPropertyId  { "loopLength" };
    static inline const juce::Identifier LoopStartPropertyId   { "loopStart" };
    static inline const juce::Identifier MinVoltagePropertyId  { "minVoltage" };
    static inline const juce::Identifier PitchOffsetPropertyId { "pitchOffset" };
    static inline const juce::Identifier SamplePropertyId      { "sample" };
    static inline const juce::Identifier SampleStartPropertyId { "sampleStart" };
    static inline const juce::Identifier SampleEndPropertyId   { "sampleEnd" };
    static inline const juce::Identifier SidePropertyId        { "side" };

    void initValueTree () {}
    void processValueTree () {}

private:

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
