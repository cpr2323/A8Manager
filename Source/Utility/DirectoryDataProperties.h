#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

static const juce::Identifier NameProperty { "name" };

class FileProperties : public ValueTreeWrapper<FileProperties>
{
public:
    FileProperties () noexcept : ValueTreeWrapper<FileProperties> (FileTypeId)
    {
    }
    FileProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<FileProperties> (FileTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    static inline const juce::Identifier FileTypeId { "File" };

    void initValueTree () {}
    void processValueTree () {}

private:
};

class FolderProperties : public ValueTreeWrapper<FolderProperties>
{
public:
    FolderProperties () noexcept : ValueTreeWrapper<FolderProperties> (FolderTypeId)
    {
    }
    FolderProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<FolderProperties> (FolderTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setName (juce::String name, bool includeSelfCallback)
    {
        setValue (name, NamePropertyId, includeSelfCallback);
    }

    juce::String getName ()
    {
        return getValue<juce::String> (NamePropertyId);
    }

    std::function<void (juce::ValueTree folder)> onFolderAdded;
    std::function<void (juce::ValueTree folder)> onFolderRemoved;
    std::function<void (juce::ValueTree folder)> onFolderUpdated;
    std::function<void (juce::ValueTree file)> onFileAdded;
    std::function<void (juce::ValueTree file)> onFileRemoved;
    std::function<void (juce::ValueTree file)> onFileUpdated;

    static inline const juce::Identifier FolderTypeId { "Folder" };
    static inline const juce::Identifier NamePropertyId   { "name" };
    static inline const juce::Identifier StatusPropertyId { "status" };

    static juce::ValueTree create (juce::String filePath)
    {
        juce::ValueTree fileVT { "Folder" };
        fileVT.setProperty (NamePropertyId, filePath, nullptr);
        fileVT.setProperty (StatusPropertyId, "unscanned", nullptr);
        return fileVT;
    }

    void initValueTree () {}
    void processValueTree () {}

private:
};

// class DirectoryProperties : public ValueTreeWrapper<DirectoryProperties>
// {
// public:
//     DirectoryProperties () noexcept : ValueTreeWrapper<DirectoryProperties> (DirectoryTypeId)
//     {
//     }
//     DirectoryProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
//         : ValueTreeWrapper<DirectoryProperties> (DirectoryTypeId, vt, wrapperType, shouldEnableCallbacks)
//     {
//     }
// 
//     std::function<void (juce::ValueTree folder)> onRootUpdated;
// 
//     static inline const juce::Identifier DirectoryTypeId { "Directory" };
// 
//     bool isEmpty ()
//     {
//         const auto numChildren { data.getNumChildren () };
//         jassert (numChildren == 0 || numChildren == 1);
//         return numChildren == 0;
//     }
// 
//     void setRootFolder (juce::ValueTree rootFolderVT)
//     {
//         const auto numChildren { data.getNumChildren () };
//         jassert (numChildren == 0 || numChildren == 1);
//         if (numChildren > 1)
//             data.removeChild (0, nullptr);
// 
//         FolderProperties folderProperties (rootFolderVT, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
//         data.addChild (rootFolderVT, -1, nullptr);
//     }
// 
//     juce::ValueTree getRootFolder ()
//     {
//         return data.getChild (0);
//     }
// 
//     void initValueTree () {}
//     void processValueTree () {}
// 
// private:
// 
//     void valueTreeChildAdded (juce::ValueTree& vt, juce::ValueTree& child) override
//     {
//         if (vt != data)
//             return;
// 
//         // TODO - work out better callback
//         if (onRootUpdated = nullptr)
//             onRootUpdated (child);
//     }
// 
//     void valueTreeChildRemoved (juce::ValueTree& vt, juce::ValueTree& child, int index) override
//     {
//         if (vt != data)
//             return;
// 
//         // TODO - work out better callback
//         if (onRootUpdated = nullptr)
//             onRootUpdated (child);
//     }
// //     void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
// //     void valueTreeChildOrderChanged (juce::ValueTree& vt, int oldIndex, int newIndex) override;
// //     void valueTreeParentChanged (juce::ValueTree& vt) override;
// //     void valueTreeRedirected (juce::ValueTree& vt) override;
// };

class DirectoryDataProperties : public ValueTreeWrapper<DirectoryDataProperties>
{
public:
    DirectoryDataProperties () noexcept : ValueTreeWrapper<DirectoryDataProperties> (DirectoryDataTypeId)
    {
    }
    DirectoryDataProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<DirectoryDataProperties> (DirectoryDataTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    enum class ScanStatus
    {
        empty,
        scanning,
        canceled,
        done
    };

    enum TypeIndex
    {
        folder,
        systemFile,
        presetFile,
        audioFile,
        unknownFile,
        size
    };

    void setProgress (juce::String progressString, bool includeSelfCallback);
    void setRootFolder (juce::String rootFolder, bool includeSelfCallback);
    void setScanDepth (int scanDepth, bool includeSelfCallback);
    void setStatus (ScanStatus status, bool includeSelfCallback);
    void triggerRootScanComplete (bool includeSelfCallback);
    void triggerStartScan (bool includeSelfCallback);

    juce::String getProgress ();
    juce::String getRootFolder ();
    int getScanDepth ();
    DirectoryDataProperties::ScanStatus getStatus ();

    std::function<void (juce::String progressString)> onProgressChange;
    std::function<void (juce::String rootFolder)> onRootFolderChange;
    std::function<void ()> onRootScanComplete;
    std::function<void (int scanDepth)> onScanDepthChange;
    std::function<void (ScanStatus status)> onStatusChange;
    std::function<void ()> onStartScanChange;

    juce::ValueTree getRootFolderVT ();

    static inline const juce::Identifier DirectoryDataTypeId { "DirectoryData" };
    static inline const juce::Identifier ProgressPropertyId   { "progress" };
    static inline const juce::Identifier RootFolderPropertyId { "rootFolder" };
    static inline const juce::Identifier RootScanCompletePropertyId { "rootScanComplete" };
    static inline const juce::Identifier ScanDepthPropertyId  { "scanDepth" };
    static inline const juce::Identifier StartScanPropertyId  { "startScan" };
    static inline const juce::Identifier StatusPropertyId     { "status" };

    static inline const juce::Identifier DirectoryValueTreeTypeId { "directoryValueTree" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree& vt, juce::ValueTree& child) override;
    void valueTreeChildRemoved (juce::ValueTree& vt, juce::ValueTree& child, int index) override;
};

