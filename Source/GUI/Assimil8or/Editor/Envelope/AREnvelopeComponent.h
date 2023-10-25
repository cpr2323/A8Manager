#pragma once

#include <JuceHeader.h>
#include "AREnvelopeProperties.h"
#include "../../../../Utility/GUI/EnvelopeComponent.h"

class AREnvelopeComponent : public EnvelopeComponent
{
public:
    AREnvelopeComponent ();

    juce::ValueTree getPropertiesVT () { return arEnvelopeProperties.getValueTree (); }

private:
    enum
    {
        eNoAnchor = -1,

        eStartAnchor = 0,
        eAttackAnchor,
        eReleaseAnchor,

        eAnchorCount
    };
    AREnvelopeProperties arEnvelopeProperties;

    void setAttackTimePercent (double attackTimePercent);
    void setReleaseTimePercent (double releaseTimePercent);

    double getAttackTimePercent () { return envelopeAnchors [eAttackAnchor]->xAxis.percentage; }
    double getReleaseTimePercent () { return envelopeAnchors [eReleaseAnchor]->xAxis.percentage; }

    void anchorChanged () override;
};
