#pragma once

#include <JuceHeader.h>
#include "ChannelProperties.h"
#include "ZoneProperties.h"
#include "../../Utility/ValueTreeWrapper.h"

class PresetProperties : public ValueTreeWrapper<PresetProperties>
{
public:
    PresetProperties () noexcept : ValueTreeWrapper<PresetProperties> (PresetTypeId)
    {
    }
    PresetProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<PresetProperties> (PresetTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setId (int id, bool includeSelfCallback);
    void setData2AsCV (juce::String data2AsCv, bool includeSelfCallback);
    void setMidiSetup (int id, bool includeSelfCallback);
    void setName (juce::String name, bool includeSelfCallback);
    void setXfadeACV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeAWidth (double width, bool includeSelfCallback);
    void setXfadeBCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeBWidth (double width, bool includeSelfCallback);
    void setXfadeCCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeCWidth (double width, bool includeSelfCallback);
    void setXfadeDCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeDWidth (double width, bool includeSelfCallback);

    int getId ();
    juce::String getData2AsCV ();
    int getMidiSetup ();
    juce::String getName ();
    juce::String getXfadeACV ();
    double getXfadeAWidth ();
    juce::String getXfadeBCV ();
    double getXfadeBWidth ();
    juce::String getXfadeCCV ();
    double getXfadeCWidth ();
    juce::String getXfadeDCV ();
    double getXfadeDWidth ();

    std::function<void (int id)> onIdChange;
    std::function<void (juce::String data2AsCv)> onData2AsCVChange;
    std::function<void (int midiSetupId)> onMidiSetupChange;
    std::function<void (juce::String name)> onNameChange;
    std::function<void (juce::String cvInput)> onXfadeACVChange;
    std::function<void (double width)> onXfadeAWidthChange;
    std::function<void (juce::String cvInput)> onXfadeBCVChange;
    std::function<void (double width)> onXfadeBWidthChange;
    std::function<void (juce::String cvInput)> onXfadeCCVChange;
    std::function<void (double width)> onXfadeCWidthChange;
    std::function<void (juce::String cvInput)> onXfadeDCVChange;
    std::function<void (double width)> onXfadeDWidthChange;

    void forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback);
    juce::ValueTree getChannelVT (int channelIndex);

    static inline const juce::Identifier PresetTypeId { "Preset" };
    static inline const juce::Identifier IdPropertyId          { "_id" };
    static inline const juce::Identifier Data2asCVPropertyId   { "data2asCV" };
    static inline const juce::Identifier MidiSetpPropertyId    { "midiSetup" };
    static inline const juce::Identifier NamePropertyId        { "name" };
    static inline const juce::Identifier XfadeACVPropertyId    { "xfadeACV" };
    static inline const juce::Identifier XfadeAWidthPropertyId { "xfadeAWidth" };
    static inline const juce::Identifier XfadeBCVPropertyId    { "xfadeBCV" };
    static inline const juce::Identifier XfadeBWidthPropertyId { "xfadeBWidth" };
    static inline const juce::Identifier XfadeCCVPropertyId    { "xfadeCCV" };
    static inline const juce::Identifier XfadeCWidthPropertyId { "xfadeCWidth" };
    static inline const juce::Identifier XfadeDCVPropertyId    { "xfadeDCV" };
    static inline const juce::Identifier XfadeDWidthPropertyId { "xfadeDWidth" };

    void initValueTree ();
    void processValueTree () {}

    static void copyTreeProperties (juce::ValueTree source, juce::ValueTree destination);

private:
    int getNumChannels ();

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
