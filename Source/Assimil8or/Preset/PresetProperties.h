#pragma once

#include <JuceHeader.h>
#include "ChannelProperties.h"
#include "ParameterDataListProperties.h"
#include "../../Utility/ValueTreeWrapper.h"

class PresetProperties : public ValueTreeWrapper<PresetProperties>
{
public:
    PresetProperties () noexcept : parameterDataListProperties (ParameterDataListProperties()), ValueTreeWrapper<PresetProperties> (PresetTypeId)
    {
        clear ();
    }
    PresetProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : parameterDataListProperties (ParameterDataListProperties ()), ValueTreeWrapper<PresetProperties> (PresetTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setIndex (int index, bool includeSelfCallback);
    void setData2AsCV (juce::String data2AsCv, bool includeSelfCallback);
    void setName (juce::String name, bool includeSelfCallback);
    void setXfadeACV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeAWidth (double width, bool includeSelfCallback);
    void setXfadeBCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeBWidth (double width, bool includeSelfCallback);
    void setXfadeCCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeCWidth (double width, bool includeSelfCallback);
    void setXfadeDCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeDWidth (double width, bool includeSelfCallback);

    int getIndex ();
    juce::String getData2AsCV ();
    juce::String getName ();
    juce::String getXfadeACV ();
    double getXfadeAWidth ();
    juce::String getXfadeBCV ();
    double getXfadeBWidth ();
    juce::String getXfadeCCV ();
    double getXfadeCWidth ();
    juce::String getXfadeDCV ();
    double getXfadeDWidth ();

    juce::String getData2AsCVDefault ();
    juce::String getNameDefault ();
    juce::String getXfadeACVDefault ();
    double getXfadeAWidthDefault ();
    juce::String getXfadeBCVDefault ();
    double getXfadeBWidthDefault ();
    juce::String getXfadeCCVDefault ();
    double getXfadeCWidthDefault ();
    juce::String getXfadeDCVDefault ();
    double getXfadeDWidthDefault ();

    std::function<void (int index)> onIndexChange;
    std::function<void (juce::String data2AsCv)> onData2AsCVChange;
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
    static inline const juce::Identifier IndexPropertyId       { "_index" };
    static inline const juce::Identifier Data2asCVPropertyId   { "data2asCV" };
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

    void clear ();

private:
    ParameterDataListProperties parameterDataListProperties;

    juce::ValueTree addChannel (int index);
    int getNumChannels ();

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
