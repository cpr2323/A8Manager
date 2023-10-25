#pragma once

#include <JuceHeader.h>
#include "../../../../Utility/ValueTreeWrapper.h"

class AREnvelopeProperties : public ValueTreeWrapper<AREnvelopeProperties>
{
public:
    AREnvelopeProperties () noexcept : ValueTreeWrapper<AREnvelopeProperties> (EnvelopeTypeId)
    {
    }
    AREnvelopeProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<AREnvelopeProperties> (EnvelopeTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setAttackPercent (double attackPercent, bool includeSelfCallback);
    void setReleasePercent (double releasePercent, bool includeSelfCallback);

    double getAttackPercent ();
    double getReleasePercent ();

    std::function<void (double attackPercent)> onAttackPercentChanged;
    std::function<void (double releasePercent)> onReleasePercentChanged;

    static inline const juce::Identifier EnvelopeTypeId { "Envelope" };
    static inline const juce::Identifier AttackPercentPropertyId { "attackPercent" };
    static inline const juce::Identifier ReleasePercentPropertyId { "releasePercent" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
