#include "A8SDCardValidatorProperties.h"

void A8SDCardValidatorProperties::initValueTree ()
{
    setScanStatus ("idle", false);
    data.addChild (juce::ValueTree {ValidationsStatusId}, -1, nullptr);
}

void A8SDCardValidatorProperties::setScanStatus (juce::String scanStatus, bool includeSelfCallback)
{
    setValue (scanStatus, ScanStatusPropertyId, includeSelfCallback);
}

juce::String A8SDCardValidatorProperties::getScanStatus ()
{
    return getValue<juce::String> (ScanStatusPropertyId);
}

void A8SDCardValidatorProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == ScanStatusPropertyId)
        {
            if (onScanStatusChanged != nullptr)
                onScanStatusChanged (getScanStatus ());
        }
    }
}