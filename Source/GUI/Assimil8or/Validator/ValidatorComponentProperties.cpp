#include "ValidatorComponentProperties.h"

void ValidatorComponentProperties::initValueTree ()
{
    setFilterInfo (false, false);
    setFilterWarning (false, false);
    setFilterError (false, false);
}

void ValidatorComponentProperties::setFilterInfo (bool shouldFilter, bool includeSelfCallback)
{
    setValue (shouldFilter, FilterInfoPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::setFilterWarning (bool shouldFilter, bool includeSelfCallback)
{
    setValue (shouldFilter, FilterWarningPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::setFilterError (bool shouldFilter, bool includeSelfCallback)
{
    setValue (shouldFilter, FilterErrorPropertyId, includeSelfCallback);
}

bool ValidatorComponentProperties::getFilterInfo ()
{
    return getValue<bool> (FilterInfoPropertyId);
}

bool ValidatorComponentProperties::getFilterWarning ()
{
    return getValue<bool> (FilterWarningPropertyId);
}

bool ValidatorComponentProperties::getFilterError ()
{
    return getValue<bool> (FilterErrorPropertyId);
}

void ValidatorComponentProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == FilterInfoPropertyId)
        {
            if (onFilterInfoChange != nullptr)
                onFilterInfoChange (getFilterInfo ());
        }
        else if (property == FilterWarningPropertyId)
        {
            if (onFilterWarningChange != nullptr)
                onFilterWarningChange (getFilterWarning ());
        }
        else if (property == FilterErrorPropertyId)
        {
            if (onFilterErrorChange != nullptr)
                onFilterErrorChange (getFilterError ());
        }
    }
}