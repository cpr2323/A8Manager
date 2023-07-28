#pragma once

#include <JuceHeader.h>
#include "ChannelProperties.h"
#include "../../Utility/ValueTreeWrapper.h"

// TODO - under construction - idea exploration
///////////////////////////////////////////////
//
template <typename T>
struct ParameterSpec
{
    T min;
    T max;
    T dflt;
};

using ParameterSpecInt = ParameterSpec<int>;
using ParameterSpecFloat = ParameterSpec<float>;
using ParameterSpecString = ParameterSpec<juce::String>;
//
///////////////////////////////////////////////

class PresetProperties : public ValueTreeWrapper
{
public:
    PresetProperties () noexcept : ValueTreeWrapper (PresetTypeId) {}

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

    juce::ValueTree addChannel (int index);
    void forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback);
    int getNumChannels ();

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

private:
    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};