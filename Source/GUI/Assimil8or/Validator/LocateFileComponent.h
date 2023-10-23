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
        directoryDataProperties.setScanDepth (0, false);
        directoryDataProperties.onRootScanComplete = [this] ()
        {
            juce::Logger::outputDebugString ("DirectoryViewerComponent::DirectoryViewerComponent - row : directoryDataProperties.onRootScanComplete");
            isRootFolder = juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory () == juce::File (directoryDataProperties.getRootFolder ());
            buildQuickLookupList ();
            juce::MessageManager::callAsync ([this] ()
            {
                directoryListBox.updateContent ();
                directoryListBox.repaint ();
            });
        };
    }

    void setCurrentFolder (juce::File folder)
    {
        directoryDataProperties.setRootFolder (folder.getFullPathName (), false);
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
    bool isRootFolder { false };

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
        return static_cast<int> (directoryListQuickLookupList.size ()) + (isRootFolder ? 0 : 1);
    }
    //juce::String getTooltipForRow (int row) override;

    void listBoxItemClicked (int row, const juce::MouseEvent&) override
    {
        if (! isRootFolder && row == 0)
        {
            directoryDataProperties.setRootFolder (juce::File (directoryDataProperties.getRootFolder ()).getParentDirectory ().getFullPathName (), false);
        }
        else
        {
            const auto directoryEntryVT { directoryListQuickLookupList [row - (isRootFolder ? 0 : 1)] };
            auto folder { juce::File (directoryEntryVT.getProperty ("name").toString ()) };
            directoryDataProperties.setRootFolder (folder.getFullPathName (), false);
        }
        startScan ();
        if (onFolderChange != nullptr)
            onFolderChange (directoryDataProperties.getRootFolder ());
    }

    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        if (rowNumber < getNumRows ())
        {
            if (! isRootFolder && rowNumber == 0)
            {
                g.setColour (juce::Colours::white);
                g.drawText (" > ..", juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
            }
            else
            {
                const auto directoryEntry { juce::File (directoryListQuickLookupList [rowNumber - (isRootFolder ? 0 : 1)].getProperty (FileProperties::NamePropertyId).toString ()) };
                if (directoryEntry.isDirectory ())
                {
                    g.setColour (juce::Colours::white);
                    g.drawText (" > " + directoryEntry.getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
                }
                else
                {
                    g.setColour (juce::Colours::forestgreen);
                    g.drawText (" - " + directoryEntry.getFileName (), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
                }
            }
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
    MissingFileComponent ()
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
            g.setColour (juce::Colours::red.darker (0.2f));
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
    LocateFileComponent (std::vector<juce::File> theMissingFiles, juce::File startingFolder,
                         std::function<void (std::vector<std::tuple <juce::File, juce::File>>)> theLocatedFilesCallback, std::function<void ()> theCancelCallback);

    juce::File getCurFolder ();
private:
    juce::Label curFolderLabel;
    DirectoryViewerComponent directoryViewerComponent;
    MissingFileComponent missingFileComponent;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton openButton;
    juce::Label missingFilesLabel;
    juce::TextButton lookHereButton;
    juce::TextButton cancelButton;
    std::function<void (std::vector<std::tuple <juce::File, juce::File>>)> locatedFilesCallback;
    std::function<void ()> cancelCallback;

    void locateFiles ();

    void paint (juce::Graphics& g) override;
    void resized () override;
};