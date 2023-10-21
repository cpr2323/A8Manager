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

private:
    juce::ListBox directory { {}, this };

    int getNumRows () override
    {
        return 2;
    }
    //juce::String getTooltipForRow (int row) override;
    //void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber < 2)
        {
            g.setColour (juce::Colours::grey.brighter(0.3f));
            g.fillRect (width - 1, 0, 1, height);
            g.setColour (juce::Colours::black);
            g.drawText (juce::String (rowNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
       }
    }
};

class MissingFileComponent : public juce::Component,
                             private juce::ListBoxModel
{
public:

private:
    juce::ListBox missingFiles { {}, this };

    int getNumRows () override
    {
        return 2;
    }
    //juce::String getTooltipForRow (int row) override;
    //void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber < 2)
        {
            g.setColour (juce::Colours::grey.brighter (0.3f));
            g.fillRect (width - 1, 0, 1, height);
            g.setColour (juce::Colours::black);
            g.drawText (juce::String (rowNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        }
    }
};

class LocateFileComponent : public juce::Component
{
public:
    LocateFileComponent ();
private:
    juce::Label curFolderLabel;
    DirectoryViewerComponent directoryViewerComponent;
    MissingFileComponent missingFileComponent;
    juce::TextButton lookHereButton;
    juce::TextButton cancelButton;

    void resized () override;
};