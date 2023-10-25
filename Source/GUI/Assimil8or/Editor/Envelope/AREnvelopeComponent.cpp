#include "AREnvelopeComponent.h"

AREnvelopeComponent::AREnvelopeComponent ()
{
    initAnchors (eAnchorCount);

    // start Anchor is locked at the bottom, cannot move it
    envelopeAnchors [eStartAnchor]->xAxis.locked = true;
    envelopeAnchors [eStartAnchor]->yAxis.locked = true;
    envelopeAnchors [eStartAnchor]->xAxis.percentage = 0.0;
    envelopeAnchors [eStartAnchor]->yAxis.percentage = 0.0;
    
    // attack Anchor is locked at the top, can move in the horizontal only
    envelopeAnchors[eAttackAnchor]->xAxis.locked = false;
    envelopeAnchors[eAttackAnchor]->yAxis.locked = true;
    envelopeAnchors[eAttackAnchor]->xAxis.percentage  = 0.0;
    envelopeAnchors[eAttackAnchor]->yAxis.percentage = 1.0;
    envelopeAnchors[eAttackAnchor]->yAxis.followRight = true;

	// release Anchor is locked at the bottom, can move the horizontal only
    envelopeAnchors[eReleaseAnchor]->xAxis.locked = false;
    envelopeAnchors[eReleaseAnchor]->yAxis.locked = true;
    envelopeAnchors[eReleaseAnchor]->xAxis.percentage  = 1.0;
    envelopeAnchors[eReleaseAnchor]->yAxis.percentage = 0.0;
    envelopeAnchors[eReleaseAnchor]->yAxis.followLeft = true;

    recalculateCoordinates ();

    arEnvelopeProperties.wrap ({}, AREnvelopeProperties::WrapperType::owner, AREnvelopeProperties::EnableCallbacks::yes);
    arEnvelopeProperties.onAttackPercentChanged = [this] (double attackPercent) { setAttackTimePercent (attackPercent); };
    arEnvelopeProperties.onReleasePercentChanged = [this] (double releasePercent) { setReleaseTimePercent (releasePercent); };
}

void AREnvelopeComponent::setAttackTimePercent (double attackTimePercent)
{
    setAnchorXPercent (eAttackAnchor, attackTimePercent);
    repaint ();
}
void AREnvelopeComponent::setReleaseTimePercent (double releaseTimePercent)
{
    setAnchorXPercent (eReleaseAnchor, releaseTimePercent);
    repaint ();
}

void AREnvelopeComponent::anchorChanged ()
{
    arEnvelopeProperties.setAttackPercent (getAttackTimePercent (), false);
    arEnvelopeProperties.setReleasePercent (getReleaseTimePercent (), false);
}
