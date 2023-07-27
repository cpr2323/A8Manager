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

    void setData2AsCV (bool data2AsCv, bool includeSelfCallback);
    void setName (juce::String name, bool includeSelfCallback);
    void setXfadeACV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeAWidth (float width, bool includeSelfCallback);
    void setXfadeBCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeBWidth (float width, bool includeSelfCallback);
    void setXfadeCCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeCWidth (float width, bool includeSelfCallback);
    void setXfadeDCV (juce::String cvInput, bool includeSelfCallback);
    void setXfadeDWidth (float width, bool includeSelfCallback);

    bool getData2AsCV ();
    juce::String getName ();
    juce::String getXfadeACV ();
    float getXfadeAWidth ();
    juce::String getXfadeBCV ();
    float getXfadeBWidth ();
    juce::String getXfadeCCV ();
    float getXfadeCWidth ();
    juce::String getXfadeDCV ();
    float getXfadeDWidth ();

    std::function<void (bool data2AsCv)> onData2AsCVChange;
    std::function<void (juce::String name)> onNameChange;
    std::function<void (juce::String cvInput)> onXfadeACVChange;
    std::function<void (float width)> onXfadeAWidthChange;
    std::function<void (juce::String cvInput)> onXfadeBCVChange;
    std::function<void (float width)> onXfadeBWidthChange;
    std::function<void (juce::String cvInput)> onXfadeCCVChange;
    std::function<void (float width)> onXfadeCWidthChange;
    std::function<void (juce::String cvInput)> onXfadeDCVChange;
    std::function<void (float width)> onXfadeDWidthChange;

    void addChannel ();
    void forEachChannel (std::function<bool (juce::ValueTree channelVT)> channelVTCallback);

    static inline const juce::Identifier PresetTypeId { "Preset" };
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
    int getNumChannels ();

    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};