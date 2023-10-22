#pragma once

#include <JuceHeader.h>
#include "../../../Utility/DirectoryValueTree.h"

// +------------------------------++----------------------+
// | \folder\being\viewed         || Missing Files        |
// +------------------------------++----------------------+
// | > Folder A                   || Kick 1.wav (C1/Z1)   |
// | > Folder X                   || Kick 2.wav (C1/Z2)   |
// | > Folder 2                   || Snare 3.wav          |
// | - File.wav                   || Birds.wav            |
// | - OtherFile.wav              ||                      |
// | - MoreFiles.wav              ||                      |
// +------------------------------++----------------------+
//   [ Look Here ] [ Cancel ]
//
//  
// +------------------------------------------------------------+
// | \folder\being\viewed                                       |
// +------------------------------------------------------------+
// | > Folder A                                                 |
// | > Folder X                                                 |
// | > Folder 2                                                 |
// | - File.wav                                                 |
// | - OtherFile.wav                                            |
// | - MoreFiles.wav                                            |
// +------------------------------------------------------------+
//   [ Look Here ] [ Cancel ]
// +------------------------------------------------------------+
// | Missing Files                                              |
// +------------------------------------------------------------+
// | - Kick 1.wav (C1/Z1)                                       |
// | - Kick 2.wav (C1/Z2)                                       |
// | - Snare 3.wav (C2/Z1)                                      |
// | - Birds.wav (C8/Z1)                                        |
// |                                                            |
// |                                                            |
// +------------------------------------------------------------+
//
//
// +----------------------++------------------------------+
// | Missing Files        || \folder\being\viewed         |
// +----------------------++------------------------------+
// | Kick 1.wav (C1/Z1)   || > Folder A                   |
// | Kick 2.wav (C1/Z2)   || > Folder X                   |
// | Snare 3.wav          || > Folder 2                   |
// | Birds.wav            || - File.wav                   |
// |                      || - OtherFile.wav              |
// |                      || - MoreFiles.wav              |
// +----------------------++------------------------------+
//   [ Look Here ] [ Cancel ]
//
//
// +------------------------------------------------------------+
// | Missing Files                                              |
// +------------------------------------------------------------+
// | - Kick 1.wav (C1/Z1)                                       |
// | - Kick 2.wav (C1/Z2)                                       |
// | - Snare 3.wav (C2/Z1)                                      |
// | - Birds.wav (C8/Z1)                                        |
// |                                                            |
// |                                                            |
// +------------------------------------------------------------+
// +------------------------------------------------------------+
// | \folder\being\viewed                                       |
// +------------------------------------------------------------+
// | > Folder A                                                 |
// | > Folder X                                                 |
// | > Folder 2                                                 |
// | - File.wav                                                 |
// | - OtherFile.wav                                            |
// | - MoreFiles.wav                                            |
// +------------------------------------------------------------+
//   [ Look Here ] [ Cancel ]

class DirectoryViewerComponent : public juce::Component,
                                 private juce::ListBoxModel
{
public:
    DirectoryViewerComponent ()
    {
        directoryValueTree.init ({});
        addAndMakeVisible (directoryListBox);

        directoryDataProperties.wrap (directoryValueTree.getDirectoryDataPropertiesVT (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
        directoryDataProperties.setRootFolder ("C:/", false);
        directoryDataProperties.setScanDepth (0, false);
        directoryDataProperties.onRootScanComplete = [this] ()
        {
            juce::Logger::outputDebugString ("DirectoryViewerComponent::DirectoryViewerComponent - row : directoryDataProperties.onRootScanComplete");
            buildQuickLookupList ();
            juce::MessageManager::callAsync ([this] ()
            {
                directoryListBox.updateContent ();
                directoryListBox.repaint ();
            });
        };
    }

    juce::File getCurrentFolder ()
    {
        return directoryDataProperties.getRootFolder ();
    }

    void startScan ()
    {
        directoryDataProperties.triggerStartScan (false);
    }

    std::function<void (juce::File)> onFolderChange;

private:
    DirectoryValueTree directoryValueTree;
    DirectoryDataProperties directoryDataProperties;
    std::vector<juce::ValueTree> directoryListQuickLookupList;
    juce::ListBox directoryListBox { {}, this };

    void buildQuickLookupList ()
    {
        directoryListQuickLookupList.clear ();
        ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this] (juce::ValueTree child)
        {
            if (FolderProperties::isFolderVT (child))
            {
                directoryListQuickLookupList.emplace_back (child);
            }
            else
            {
                FileProperties fp (child, FileProperties::WrapperType::client, FileProperties::EnableCallbacks::no);
                const auto file { juce::File (fp.getName ()) };
                if (file.getFileExtension ().toLowerCase () == ".wav")
                    directoryListQuickLookupList.emplace_back (child);
            }

            return true;
        });
    }

    int getNumRows () override
    {
        return directoryListQuickLookupList.size ();
    }
    //juce::String getTooltipForRow (int row) override;

    void listBoxItemClicked (int row, const juce::MouseEvent& me) override
    {
//         if (! isRootFolder && row == 0)
//         {
//             directoryDataProperties.setRootFolder (juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName (), false);
//         }
//         else
        {
            const auto directoryEntryVT { directoryListQuickLookupList [row] };
            auto folder { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
            directoryDataProperties.setRootFolder (folder.getFullPathName (), false);
            startScan ();
            if (onFolderChange != nullptr)
                onFolderChange (folder);
        }

    }

    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        if (rowNumber < directoryListQuickLookupList.size ())
        {
            //             g.setColour (juce::Colours::grey.brighter (0.3f));
            //             g.fillRect (width, 0, 1, height);
            g.setColour (juce::Colours::white);
            const auto directoryEntry { juce::File (directoryListQuickLookupList [rowNumber].getProperty (FileProperties::NamePropertyId).toString ()) };
            if (directoryEntry.isDirectory ())
                g.drawText (" > " + directoryEntry.getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
            else
                g.drawText (" - " + directoryEntry.getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        }
    }
    void resized () override
    {
        directoryListBox.setBounds (getLocalBounds ());
    }
};

class MissingFileComponent : public juce::Component,
                             private juce::ListBoxModel
{
public:
    MissingFileComponent()
    {
        addAndMakeVisible (missingFilesListBox);
        missingFilesListBox.updateContent ();
    }

    std::vector<juce::File>& getMissingFileList ()
    {
        return missingFilesList;
    }

    void assignMissingFileList (std::vector<juce::File> theMissingFiles)
    {
        missingFilesList = theMissingFiles;
        missingFilesListBox.updateContent ();
    }

private:
    std::vector<juce::File> missingFilesList;
    juce::ListBox missingFilesListBox { {}, this };

    int getNumRows () override
    {
        return static_cast<int> (missingFilesList.size ());
    }
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        if (rowNumber < missingFilesList.size ())
        {
            g.setColour (juce::Colours::white);
            g.drawText (" " + missingFilesList [rowNumber].getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        }
    }
    void resized () override
    {
        missingFilesListBox.setBounds (getLocalBounds ());
    }
};

class LocateFileComponent : public juce::Component
{
public:
    LocateFileComponent (std::vector<juce::File> theMissingFiles, std::function<void(std::vector<std::tuple <juce::File, juce::File>>)> theLocatedFilesCallback, std::function<void ()> theCancelCallback);

private:
    juce::Label curFolderLabel;
    DirectoryViewerComponent directoryViewerComponent;
    MissingFileComponent missingFileComponent;
    juce::TextButton lookHereButton;
    juce::TextButton cancelButton;
    std::function<void (std::vector<std::tuple <juce::File, juce::File>>)> locatedFilesCallback;
    std::function<void ()> cancelCallback;

    void locateFiles ();

    void paint (juce::Graphics& g) override;
    void resized () override;
};