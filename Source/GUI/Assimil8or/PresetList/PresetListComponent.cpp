#include "PresetListComponent.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Assimil8or/Assimil8orPreset.h"

PresetListComponent::PresetListComponent () : Thread ("PresetListComponent")
{
    addAndMakeVisible (presetListBox);
}

void PresetListComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startScan (juce::File (folderName));
    };
    startScan (appProperties.getMostRecentFolder ());
}

void PresetListComponent::startScan (juce::File folderToScan)
{
    // TODO - need to cancel any current scan and start
    rootFolder = folderToScan;
    startThread ();
}

void PresetListComponent::loadFirstPreset ()
{
    bool presetLoaded { false };
    for (auto presetIndex { 0 }; presetIndex < kMaxPresets; ++presetIndex)
    {
        if (! presetExists [presetIndex])
            continue;

        auto presetFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (getPresetName (presetIndex)).withFileExtension (".yml") };
        presetListBox.selectRow (presetIndex, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (presetIndex);
        loadPreset (presetFile);
        appProperties.addRecentlyUsedFile (presetFile.getFullPathName ());
        presetLoaded = true;
        break;
    }

    if (! presetLoaded)
    {
        presetListBox.selectRow (0, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (0);
        presetProperties.clear ();
    }
}

void PresetListComponent::checkForPresets ()
{
    for (auto presetIndex { 0 }; presetIndex < kMaxPresets; ++presetIndex)
    {
        auto presetFile { rootFolder.getChildFile (getPresetName (presetIndex)).withFileExtension (".yml")};
        presetExists [presetIndex] = presetFile.exists ();
    }

    juce::MessageManager::callAsync ([this] ()
    {
        presetListBox.updateContent ();
        presetListBox.scrollToEnsureRowIsOnscreen (0);
        presetListBox.repaint ();
        loadFirstPreset ();
    });
}

void PresetListComponent::run ()
{
    checkForPresets ();
}

juce::String PresetListComponent::getPresetName (int presetIndex)
{
    const auto rawPresetIndexString { juce::String (presetIndex + 1) };
    const auto presetIndexString { juce::String ("000").substring (0, 3 - rawPresetIndexString.length ()) + rawPresetIndexString };
    return "prst" + presetIndexString;
}

void PresetListComponent::loadPreset (juce::File presetFile)
{
    jassert (presetProperties.isValid ());
    juce::StringArray fileContents;
    presetFile.readLines (fileContents);

    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.parse (fileContents);

    presetProperties.clear ();
    presetProperties.getValueTree ().copyPropertiesAndChildrenFrom (assimil8orPreset.getPresetVT (), nullptr);
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
        if (! presetExists [rowNumber])
            textColor = textColor.withAlpha (0.5f);
        g.setColour (textColor);
        g.drawText ("  Preset " + juce::String (rowNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

juce::String PresetListComponent::getTooltipForRow (int row)
{
    return "Preset " + juce::String (row + 1);
}

void PresetListComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    auto presetFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (getPresetName (row)).withFileExtension (".yml") };
    if (presetExists [row])
        loadPreset (presetFile);
    else
        presetProperties.clear ();
    appProperties.addRecentlyUsedFile (presetFile.getFullPathName ());
}
