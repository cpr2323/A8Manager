#include "DirectoryDataProperties.h"

void DirectoryDataProperties::initValueTree ()
{
    // create all the initial properties
    setRootFolder ("", false);
    setScanDepth (-1, false);
    triggerStartScan (false);
    data.addChild (juce::ValueTree (DirectoryValueTreeTypeId), -1, nullptr);
}

void DirectoryDataProperties::setRootFolder (juce::String rootFolder, bool includeSelfCallback)
{
    setValue (rootFolder, RootFolderPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::setScanDepth (int scanDepth, bool includeSelfCallback)
{
    setValue (scanDepth, ScanDepthPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::setStatus (int status, bool includeSelfCallback)
{
    setValue (status, StatusPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::triggerStartScan (bool includeSelfCallback)
{
    toggleValue (StartScanPropertyId, includeSelfCallback);
}

juce::String DirectoryDataProperties::getRootFolder ()
{
    return getValue<juce::String> (RootFolderPropertyId);
}

int DirectoryDataProperties::getScanDepth ()
{
    return getValue<int> (ScanDepthPropertyId);
}

int DirectoryDataProperties::getStatus ()
{
    return getValue<int> (StatusPropertyId);
}

juce::ValueTree DirectoryDataProperties::getDirectoryValueTreeVT ()
{
    auto directoryValueTreeVT { data.getChildWithName (DirectoryValueTreeTypeId).getChildWithName("Folder")};
    jassert (directoryValueTreeVT.isValid ());
    return directoryValueTreeVT;
}

void DirectoryDataProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == RootFolderPropertyId)
        {
            if (onRootFolderChange != nullptr)
                onRootFolderChange (getRootFolder ());
        }
        else if (property == ScanDepthPropertyId)
        {
            if (onScanDepthChange != nullptr)
                onScanDepthChange (getScanDepth ());
        }
        else if (property == StartScanPropertyId)
        {
            if (onStartScanChange != nullptr)
                onStartScanChange ();
        }
        else if (property == StatusPropertyId)
        {
            if (onStatusChange != nullptr)
                onStatusChange (getStatus ());
        }
    }
}
