#pragma once

#include <JuceHeader.h>

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
        addAndMakeVisible (directoryListBox);
        directoryListBox.updateContent ();
    }

    juce::File getCurrentFolder ()
    {
        return juce::File("C:/");
    }

private:
    juce::ListBox directoryListBox { {}, this };

    int getNumRows () override
    {
        return 2;
    }
    //juce::String getTooltipForRow (int row) override;
    //void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        if (rowNumber < 2)
        {
            //             g.setColour (juce::Colours::grey.brighter (0.3f));
            //             g.fillRect (width, 0, 1, height);
            g.setColour (juce::Colours::white);
            g.drawText (" " + juce::String (rowNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
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