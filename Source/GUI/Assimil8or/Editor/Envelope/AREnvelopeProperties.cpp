#include "AREnvelopeProperties.h"

void AREnvelopeProperties::initValueTree ()
{
    setAttackPercent (0.0, false);
    setReleasePercent (1.0, false);
}

void AREnvelopeProperties::setAttackPercent (double attackPercent, bool includeSelfCallback)
{
    juce::Logger::outputDebugString ("AREnvelopeProperties::setAttackPercent: " + juce::String (attackPercent, 4));
    jassert (attackPercent >= 0.0 && attackPercent <= 1.0);
    setValue (attackPercent, AttackPercentPropertyId, includeSelfCallback);
}

void AREnvelopeProperties::setReleasePercent (double releasePercent, bool includeSelfCallback)
{
    juce::Logger::outputDebugString ("AREnvelopeProperties::setReleasePercent: " + juce::String (releasePercent, 4));
    jassert (releasePercent >= 0.0 && releasePercent <= 1.0);
    setValue (releasePercent, ReleasePercentPropertyId, includeSelfCallback);
}

double AREnvelopeProperties::getAttackPercent ()
{
    return getValue<double> (AttackPercentPropertyId);
}

double AREnvelopeProperties::getReleasePercent ()
{
    return getValue<double> (ReleasePercentPropertyId);
}

void AREnvelopeProperties::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == data)
    {
        if (property == AttackPercentPropertyId)
        {
            if (onAttackPercentChanged != nullptr)
                onAttackPercentChanged (getAttackPercent ());
        }
        else if (property == ReleasePercentPropertyId)
        {
            if (onReleasePercentChanged != nullptr)
                onReleasePercentChanged (getReleasePercent ());
        }
    }
}
