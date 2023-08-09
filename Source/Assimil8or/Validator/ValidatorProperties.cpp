#include "ValidatorProperties.h"

void ValidatorProperties::initValueTree ()
{
    setScanStatus ("idle", false);
    data.addChild (juce::ValueTree { ValidatorResultListProperties::ValidatorResultListTypeId }, -1, nullptr);
}

void ValidatorProperties::setScanStatus (juce::String scanStatus, bool includeSelfCallback)
{
    setValue (scanStatus, ScanStatusPropertyId, includeSelfCallback);
}

void ValidatorProperties::setProgressUpdate (juce::String progressUpdate, bool includeSelfCallback)
{
    setValue (progressUpdate, ProgressUpdatePropertyId, includeSelfCallback);
}

void ValidatorProperties::startScan (bool includeSelfCallback)
{
    toggleValue (StartScanPropertyId, includeSelfCallback);
}

juce::String ValidatorProperties::getScanStatus ()
{
    return getValue<juce::String> (ScanStatusPropertyId);
}

juce::String ValidatorProperties::getProgressUpdate ()
{
    return getValue<juce::String> (ProgressUpdatePropertyId);
}

juce::ValueTree ValidatorProperties::getValidatorResultListVT ()
{
    return data.getChildWithName (ValidatorResultListProperties::ValidatorResultListTypeId);
}

void ValidatorProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == ProgressUpdatePropertyId)
        {
            if (onProgressUpdateChanged != nullptr)
                onProgressUpdateChanged (getProgressUpdate ());
        }
        else if (property == ScanStatusPropertyId)
        {
            if (onScanStatusChanged != nullptr)
                onScanStatusChanged (getScanStatus ());
        }
        else if (property == StartScanPropertyId)
        {
            if (onStartScan != nullptr)
                onStartScan ();
        }
    }
}