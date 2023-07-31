#include "ValidatorResultListProperties.h"
#include "ValidatorResultProperties.h"

void ValidatorResultListProperties::addResult(juce::ValueTree validatorResultVT)
{
    jassert (validatorResultVT.getType () == ValidatorResultProperties::ValidatorResultTypeId);
    data.addChild (validatorResultVT, -1, nullptr);
}

void ValidatorResultListProperties::clear ()
{
    data.removeAllChildren (nullptr);
}

void ValidatorResultListProperties::forEachResult (std::function<bool (juce::ValueTree validatorResultVT)> validatorResultVTCallback)
{
    jassert (validatorResultVTCallback!= nullptr);
    ValueTreeHelpers::forEachChildOfType (data, ValidatorResultProperties::ValidatorResultTypeId, [this, validatorResultVTCallback] (juce::ValueTree validatorResultsVT)
    {
        return validatorResultVTCallback (validatorResultsVT);
    });
}

int ValidatorResultListProperties::getNumResults ()
{
    auto numResults { 0 };
    forEachResult ([&numResults] (juce::ValueTree) { ++numResults; return true; });
    return numResults;
}
