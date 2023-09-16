#include "DirectoryDataProperties.h"

void DirectoryDataProperties::initValueTree ()
{
    // create all the initial properties
    setProgress ("", false);
    setRootFolder ("", false);
    setScanDepth (-1, false);
    triggerStartScan (false);
    setStatus (ScanStatus::empty, false);
    data.addChild (juce::ValueTree (DirectoryValueTreeTypeId), -1, nullptr);
}

void DirectoryDataProperties::setProgress (juce::String progressString, bool includeSelfCallback)
{
    setValue (progressString, ProgressPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::setRootFolder (juce::String rootFolder, bool includeSelfCallback)
{
    setValue (rootFolder, RootFolderPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::setScanDepth (int scanDepth, bool includeSelfCallback)
{
    setValue (scanDepth, ScanDepthPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::setStatus (DirectoryDataProperties::ScanStatus status, bool includeSelfCallback)
{
    setValue (static_cast<int>(status), StatusPropertyId, includeSelfCallback);
}

void DirectoryDataProperties::triggerStartScan (bool includeSelfCallback)
{
    toggleValue (StartScanPropertyId, includeSelfCallback);
}

juce::String DirectoryDataProperties::getProgress ()
{
    return getValue<juce::String> (ProgressPropertyId);
}

juce::String DirectoryDataProperties::getRootFolder ()
{
    return getValue<juce::String> (RootFolderPropertyId);
}

int DirectoryDataProperties::getScanDepth ()
{
    return getValue<int> (ScanDepthPropertyId);
}

DirectoryDataProperties::ScanStatus DirectoryDataProperties::getStatus ()
{
    return static_cast<DirectoryDataProperties::ScanStatus>(getValue<int> (StatusPropertyId));
}

juce::ValueTree DirectoryDataProperties::getDirectoryValueTreeVT ()
{
    auto directoryValueTreeVT { data.getChildWithName (DirectoryValueTreeTypeId).getChildWithName ("Folder")};
    jassert (directoryValueTreeVT.isValid ());
    return directoryValueTreeVT;
}

juce::ValueTree DirectoryDataProperties::getDirectoryValueTreeContainerVT ()
{
    auto directoryValueTreeContainerVT { data.getChildWithName (DirectoryValueTreeTypeId) };
    jassert (directoryValueTreeContainerVT.isValid ());
    return directoryValueTreeContainerVT;
}

void DirectoryDataProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt == data)
    {
        if (property == ProgressPropertyId)
        {
            if (onProgressChange != nullptr)
                onProgressChange (getProgress ());
        }
        else if (property == RootFolderPropertyId)
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
