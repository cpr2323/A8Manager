#pragma once

#include <JuceHeader.h>
#include "../../Utility/ValueTreeWrapper.h"
#include "../Fixer/FixerEntryProperties.h"

class ValidatorResultProperties : public ValueTreeWrapper<ValidatorResultProperties>
{
public:
     ValidatorResultProperties () noexcept : ValueTreeWrapper<ValidatorResultProperties> (ValidatorResultTypeId) {}
     ValidatorResultProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks)
         : ValueTreeWrapper<ValidatorResultProperties> (ValidatorResultTypeId, vt, wrapperType, shouldEnableCallbacks) {}

    void reset (bool includeSelfCallback);
    void updateType (juce::String resultType, bool includeSelfCallback);
    void updateText (juce::String resultText, bool includeSelfCallback);
    void update (juce::String resultType, juce::String resultText, bool includeSelfCallback);

    juce::String getType ();
    juce::String getText ();

    std::function<void (juce::String resultType)> onTypeChange;
    std::function<void (juce::String resultText)> onTextChange;

    void addFixerEntry (juce::String fixerType, juce::String fileName);
    void forEachFixerEntry (std::function<bool (juce::ValueTree fixerEntryVT)> fixerEntryVTCallback);
    int getNumFixerEntries ();

    static inline const juce::String ResultTypeNone    { "" };
    static inline const juce::String ResultTypeInfo    { "info" };
    static inline const juce::String ResultTypeWarning { "warning" };
    static inline const juce::String ResultTypeError   { "error" };

    static inline const juce::Identifier ValidatorResultTypeId { "VaildatorResult" };
    static inline const juce::Identifier TypePropertyId { "type" };
    static inline const juce::Identifier TextPropertyId { "text" };

    void initValueTree ();
    void processValueTree () {}

private:
    juce::String getNewTypeBasedOnPriority (juce::String newType);
    void setType (juce::String resultType, bool includeSelfCallback);
    void setText (juce::String resultText, bool includeSelfCallback);

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
