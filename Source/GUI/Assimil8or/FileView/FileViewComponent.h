#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"

class TreeViewMouseDown : public juce::MouseListener
{
public:
    TreeViewMouseDown (juce::TreeView& treeViewToWatch) : treeView { treeViewToWatch }
    {
        treeView.addMouseListener (this, true);
    }

    ~TreeViewMouseDown ()
    {
        treeView.removeMouseListener (this);
    }

    std::function<void(int row)> onItemSelected;

private:
    juce::TreeView& treeView;
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (const auto treeViewItem { treeView.getSelectedItem (0) }; treeViewItem != nullptr)
        {
            if (onItemSelected != nullptr)
                onItemSelected (treeViewItem->getRowNumberInTree ());
        }
    }
};

class FileViewComponent : public juce::Component,
                          private juce::Timer
{
public:
    FileViewComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::TextButton navigateUpButton;

    juce::TimeSliceThread folderContentsThread { "FolderContentsThread" };
    juce::DirectoryContentsList folderContentsDirectoryList { nullptr, folderContentsThread };
    juce::FileTreeComponent fileTreeView { folderContentsDirectoryList };
    TreeViewMouseDown treeViewMouseDown { fileTreeView };

    AppProperties appProperties;

    void startFolderScan (juce::File folderToScan);

    void resized () override;
    void timerCallback () override;
};
    