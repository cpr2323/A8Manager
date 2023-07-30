#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"

class ValidatorResultProperties : public ValueTreeWrapper
{
public:
    ValidatorResultProperties () noexcept : ValueTreeWrapper (ValidatorResultTypeId) {}

    void reset (bool includeSelfCallback);
    void updateType (juce::String resultType, bool includeSelfCallback);
    void updateText (juce::String resultText, bool includeSelfCallback);

    juce::String getType ();
    juce::String getText ();

    std::function<void (juce::String resultType)> onTypeChange;
    std::function<void (juce::String resultText)> onTextChange;

    juce::ValueTree addTag (juce::String tag, juce::String description);
    void forEachTag (std::function<bool (juce::ValueTree tagVT)> tagVTCallback);
    int getNumTags ();

    static inline const juce::Identifier ValidatorResultTypeId { "VaildatorResult" };
    static inline const juce::Identifier TypePropertyId { "type" };
    static inline const juce::Identifier TextPropertyId { "text" };

private:
    void setType (juce::String resultType, bool includeSelfCallback);
    void setText (juce::String resultText, bool includeSelfCallback);

    void initValueTree () override;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
