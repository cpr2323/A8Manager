#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

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
};

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

    void setName (juce::String name, bool includeSelfCallback)
    {
        setValue (name, NamePropertyId, includeSelfCallback);
    }

    juce::String getName ()
    {
        return getValue<juce::String> (NamePropertyId);
    }

    void setType (DirectoryDataProperties::TypeIndex theType, bool includeSelfCallback)
    {
        setValue (static_cast<int> (theType), TypePropertyId, includeSelfCallback);
    }

    DirectoryDataProperties::TypeIndex getType ()
    {
        return static_cast<DirectoryDataProperties::TypeIndex> (getValue<int> (TypePropertyId));
    }

    void setCreateTime (int64_t time, bool includeSelfCallback)
    {
        setValue (time, CreationTimePropertyId, includeSelfCallback);
    }

    int64_t getCreateTime ()
    {
        return getValue<int64_t> (CreationTimePropertyId);
    }

    void setModificationTime (int64_t time, bool includeSelfCallback)
    {
        setValue (time, ModificationTimePropertyId, includeSelfCallback);
    }

    int64_t getModificationTime ()
    {
        return getValue<int64_t> (ModificationTimePropertyId);
    }

    static inline const juce::Identifier FileTypeId { "File" };
    static inline const juce::Identifier NamePropertyId             { "name" };
    static inline const juce::Identifier TypePropertyId             { "type" };
    static inline const juce::Identifier CreationTimePropertyId     { "createTime" };
    static inline const juce::Identifier ModificationTimePropertyId { "modificationTime" };

    static juce::ValueTree create (juce::String filePath, int64_t createTime, int64_t modificationTime, DirectoryDataProperties::TypeIndex fileType)
    {
        juce::ValueTree fileVT { FileTypeId };
        fileVT.setProperty (NamePropertyId, filePath, nullptr);
        fileVT.setProperty (TypePropertyId, static_cast<int> (fileType), nullptr);
        fileVT.setProperty (CreationTimePropertyId, createTime, nullptr);
        fileVT.setProperty (ModificationTimePropertyId, modificationTime, nullptr);
        return fileVT;
    }

    static bool isFileVT (juce::ValueTree directoryEntryVT)
    {
        return directoryEntryVT.getType () == FileTypeId;
    }

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

    void setCreateTime (int64_t time, bool includeSelfCallback)
    {
        setValue (time, CreationTimePropertyId, includeSelfCallback);
    }

    int64_t getCreateTime ()
    {
        return getValue<int64_t> (CreationTimePropertyId);
    }

    void setModificationTime (int64_t time, bool includeSelfCallback)
    {
        setValue (time, ModificationTimePropertyId, includeSelfCallback);
    }

    int64_t getModificationTime ()
    {
        return getValue<int64_t> (ModificationTimePropertyId);
    }

    std::function<void (juce::ValueTree folder)> onFolderAdded;
    std::function<void (juce::ValueTree folder)> onFolderRemoved;
    std::function<void (juce::ValueTree folder)> onFolderUpdated;
    std::function<void (juce::ValueTree file)> onFileAdded;
    std::function<void (juce::ValueTree file)> onFileRemoved;
    std::function<void (juce::ValueTree file)> onFileUpdated;

    static inline const juce::Identifier FolderTypeId { "Folder" };
    static inline const juce::Identifier NamePropertyId             { "name" };
    static inline const juce::Identifier StatusPropertyId           { "status" };
    static inline const juce::Identifier CreationTimePropertyId     { "createTime" };
    static inline const juce::Identifier ModificationTimePropertyId { "modificationTime" };

    static bool isFolderVT (juce::ValueTree directoryEntryVT)
    {
        return directoryEntryVT.getType () == FolderTypeId;
    }

    static juce::ValueTree create (juce::String filePath, int64_t createTime, int64_t modificationTime)
    {
        juce::ValueTree fileVT { FolderTypeId };
        fileVT.setProperty (NamePropertyId, filePath, nullptr);
        fileVT.setProperty (StatusPropertyId, "unscanned", nullptr);
        fileVT.setProperty (CreationTimePropertyId, createTime, nullptr);
        fileVT.setProperty (ModificationTimePropertyId, modificationTime, nullptr);
        return fileVT;
    }

    void initValueTree () {}
    void processValueTree () {}

private:
};
