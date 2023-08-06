#include "PresetListComponent.h"
#include "../../../Utility/PersistentRootProperties.h"

PresetListComponent::PresetListComponent ()
{
    addAndMakeVisible (presetListBox);
}

void PresetListComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startFolderScan (juce::File (folderName));
    };
    startFolderScan (appProperties.getMostRecentFolder ());
    presetListBox.updateContent ();
}

void PresetListComponent::startFolderScan (juce::File folderToScan)
{
//     folderContentsThread.startThread ();
//     folderContentsDirectoryList.clear ();
//     folderContentsDirectoryList.setDirectory (folderToScan, true, true);
//     folderContentsDirectoryList.refresh ();
//     startTimer (5);
}

void PresetListComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    presetListBox.setBounds (localBounds);
}

int PresetListComponent::getNumRows ()
{
    return kMaxPresets;
}

void PresetListComponent::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < kMaxPresets)
    {
        if (rowIsSelected)
            g.setColour (juce::Colours::darkslategrey);
        else
            g.setColour (juce::Colours::black);

        g.fillRect (width - 1, 0, 1, height);
        juce::Colour textColor;
        if (rowIsSelected)
            textColor = juce::Colours::yellow;
        else
            textColor = juce::Colours::whitesmoke;
        if (! presetExitsts [rowNumber])
            textColor = textColor.withAlpha (0.5f);
        g.setColour (textColor);
        g.drawText ("  Preset " + juce::String (rowNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

juce::String PresetListComponent::getTooltipForRow (int row)
{
    return "Preset " + juce::String (row + 1);
}

void PresetListComponent::listBoxItemClicked (int row, const juce::MouseEvent& me)
{

}
