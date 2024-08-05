#include "ValidatorComponentProperties.h"

void ValidatorComponentProperties::initValueTree ()
{
    setViewInfo (true, false);
    setViewWarning (true, false);
    setViewError (true, false);
    triggerRenameAll (false);
}

void ValidatorComponentProperties::setViewInfo (bool shouldView, bool includeSelfCallback)
{
    setValue (shouldView, ViewInfoPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::setViewWarning (bool shouldView, bool includeSelfCallback)
{
    setValue (shouldView, ViewWarningPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::setViewError (bool shouldView, bool includeSelfCallback)
{
    setValue (shouldView, ViewErrorPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::enableConvertAll (bool enabled, bool includeSelfCallback)
{
    setValue (enabled, EnableConvertAllPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::enableLocateAll (bool enabled, bool includeSelfCallback)
{
    setValue (enabled, EnableLocateAllPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::enableRenameAll (bool enabled, bool includeSelfCallback)
{
    setValue (enabled, EnableRenameAllPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::triggerConvertAll (bool includeSelfCallback)
{
    toggleValue (ConvertAllPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::triggerLocateAll (bool includeSelfCallback)
{
    toggleValue (LocateAllPropertyId, includeSelfCallback);
}

void ValidatorComponentProperties::triggerRenameAll (bool includeSelfCallback)
{
    toggleValue (RenameAllPropertyId, includeSelfCallback);
}

bool ValidatorComponentProperties::getViewInfo ()
{
    return getValue<bool> (ViewInfoPropertyId);
}

bool ValidatorComponentProperties::getViewWarning ()
{
    return getValue<bool> (ViewWarningPropertyId);
}

bool ValidatorComponentProperties::getViewError ()
{
    return getValue<bool> (ViewErrorPropertyId);
}

bool ValidatorComponentProperties::getEnabledConvertAll ()
{
    return getValue<bool> (EnableConvertAllPropertyId);
}

bool ValidatorComponentProperties::getEnabledLocateAll ()
{
    return getValue<bool> (EnableLocateAllPropertyId);
}

bool ValidatorComponentProperties::getEnabledRenameAll ()
{
    return getValue<bool> (EnableRenameAllPropertyId);
}

void ValidatorComponentProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == ViewInfoPropertyId)
        {
            if (onViewInfoChange != nullptr)
                onViewInfoChange (getViewInfo ());
        }
        else if (property == ViewWarningPropertyId)
        {
            if (onViewWarningChange != nullptr)
                onViewWarningChange (getViewWarning ());
        }
        else if (property == ViewErrorPropertyId)
        {
            if (onViewErrorChange != nullptr)
                onViewErrorChange (getViewError ());
        }
        else if (property == EnableConvertAllPropertyId)
        {
            if (onEnableConvertAllChange != nullptr)
                onEnableConvertAllChange (getEnabledConvertAll ());
        }
        else if (property == EnableLocateAllPropertyId)
        {
            if (onEnableLocateAllChange != nullptr)
                onEnableLocateAllChange (getEnabledLocateAll ());
        }
        else if (property == EnableRenameAllPropertyId)
        {
            if (onEnableRenameAllChange != nullptr)
                onEnableRenameAllChange (getEnabledRenameAll ());
        }
        else if (property == ConvertAllPropertyId)
        {
            if (onConvertAllTrigger != nullptr)
                onConvertAllTrigger ();
        }
        else if (property == LocateAllPropertyId)
        {
            if (onLocateAllTrigger != nullptr)
                onLocateAllTrigger ();
        }
        else if (property == RenameAllPropertyId)
        {
            if (onRenameAllTrigger != nullptr)
                onRenameAllTrigger ();
        }
    }
}