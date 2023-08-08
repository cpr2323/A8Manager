#pragma once

#include <JuceHeader.h>
#include "../../../AppProperties.h"
#include "../../../Utility/DirectoryValueTree.h"

class FileViewComponent : public juce::Component,
                          private juce::ListBoxModel
{
public:
    FileViewComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    AppProperties appProperties;

    juce::TextButton navigateUpButton;
    juce::TextButton openFolderButton;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ListBox directoryContentsListBox { {}, this };

    DirectoryValueTree directoryValueTree;
    std::vector<juce::ValueTree> directoryListQuickLookupList;

    void buildQuickLookupList ();
    void openFolder ();
    void startFolderScan (juce::File folderToScan);

    void resized () override;
    int getNumRows () override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::String getTooltipForRow (int row) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& me) override;
};
    