#include "ValidatorProperties.h"

void ValidatorProperties::initValueTree ()
{
    setScanStatus ("idle", false);
    data.addChild (juce::ValueTree {ValidationsStatusId}, -1, nullptr);
}

void ValidatorProperties::setRootFolder (juce::String rootFolder, bool includeSelfCallback)
{
    setValue (rootFolder, RootFolderPropertyId, includeSelfCallback);
}

void ValidatorProperties::setScanStatus (juce::String scanStatus, bool includeSelfCallback)
{
    setValue (scanStatus, ScanStatusPropertyId, includeSelfCallback);
}

void ValidatorProperties::startAsyncScan (bool includeSelfCallback)
{
    toggleValue (StartScanAsyncPropertyId, includeSelfCallback);
}

juce::String ValidatorProperties::getRootFolder ()
{
    return getValue<juce::String> (RootFolderPropertyId);
}

juce::String ValidatorProperties::getScanStatus ()
{
    return getValue<juce::String> (ScanStatusPropertyId);
}

void ValidatorProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (data == vt)
    {
        if (property == RootFolderPropertyId)
        {
            if (onRootFolderChanged != nullptr)
                onRootFolderChanged (getRootFolder ());
        }
        else if (property == ScanStatusPropertyId)
        {
            if (onScanStatusChanged != nullptr)
                onScanStatusChanged (getScanStatus ());
        }
        else if (property == StartScanAsyncPropertyId)
        {
            if (onStartScanAsync!= nullptr)
                onStartScanAsync ();
        }
    }
}