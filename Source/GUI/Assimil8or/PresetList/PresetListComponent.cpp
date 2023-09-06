#include "PresetListComponent.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "../../../Assimil8or/PresetManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"

#define LOG_PRESET_LIST 0
#if LOG_PRESET_LIST
#define LogPresetList(text) juce::Logger::outputDebugString (text);
#else
#define LogPresetList(text) ;
#endif

PresetListComponent::PresetListComponent () : Thread ("PresetListComponent")
{
    showAllPresets.onClick = [this] () { checkForPresets (false); };
    addAndMakeVisible (showAllPresets);
    showAllPresets.setButtonText ("Show All");
    addAndMakeVisible (presetListBox);
    startThread ();
    startTimer (333);
}

PresetListComponent::~PresetListComponent ()
{
    stopTimer ();
    stopThread (100);
}

void PresetListComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogPresetList ("PresetListComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        startScan (juce::File (folderName));
    };
    startScan (appProperties.getMostRecentFolder ());
}

void PresetListComponent::startScan (juce::File folderToScan)
{
    {
        juce::ScopedLock sl (queuedFolderLock);
        queuedFolderToScan = folderToScan;
        newItemQueued = true;
        LogPresetList ("PresetListComponent::startScan: " + queuedFolderToScan.getFullPathName ());
    }
    notify ();
}

bool PresetListComponent::shouldCancelOperation ()
{
    return threadShouldExit () || newItemQueued;
}

void PresetListComponent::run ()
{
    while (wait (-1) && ! threadShouldExit ())
    {
        {
            juce::ScopedLock sl (queuedFolderLock);
            rootFolder = queuedFolderToScan;
            newItemQueued = false;
            queuedFolderToScan = juce::File ();
            LogPresetList ("PresetListComponent::run: " + rootFolder.getFullPathName ());
        }
        checkForPresets (true);
    }
}

void PresetListComponent::timerCallback ()
{
    // TODO - this should run in the thread
    checkForPresets (false);
}

void PresetListComponent::forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback)
{
    jassert (presetFileCallback != nullptr);
    for (auto presetIndex { 0 }; presetIndex < kMaxPresets && ! shouldCancelOperation (); ++presetIndex)
    {
        auto presetFile { rootFolder.getChildFile (getPresetName (presetIndex)).withFileExtension (".yml") };
        if (! presetFileCallback (presetFile, presetIndex))
            break;
    }
}

void PresetListComponent::checkForPresets (bool newFolder)
{
    LogPresetList ("PresetListComponent::checkForPresets");
    const auto showAll { showAllPresets.getToggleState () };
    numPresets = 0;
    forEachPresetFile ([this, showAll] (juce::File presetFile, int index)
    {
        auto thisPresetExists { presetFile.exists () };
        if (showAll || thisPresetExists)
        {
            presetExists [numPresets] = { index, thisPresetExists };
            ++numPresets;
        }
        return ! shouldCancelOperation ();
    });

    if (! shouldCancelOperation ())
        juce::MessageManager::callAsync ([this, newFolder] ()
        {
            presetListBox.updateContent ();
            if (newFolder)
            {
                presetListBox.scrollToEnsureRowIsOnscreen (0);
                loadFirstPreset ();
            }
            presetListBox.repaint ();
        });
}

void PresetListComponent::loadFirstPreset ()
{
    LogPresetList ("PresetListComponent::loadFirstPreset");
    bool presetLoaded { false };
    forEachPresetFile ([this, &presetLoaded] (juce::File presetFile, int index)
    {
        if (auto [presetNumber, thisPresetExists] = presetExists [index]; ! thisPresetExists)
            return ! shouldCancelOperation ();

        presetListBox.selectRow (index, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (index);
        loadPreset (presetFile);
        appProperties.addRecentlyUsedFile (presetFile.getFullPathName ());
        presetLoaded = true;
        return false;
    });

    if (! presetLoaded)
    {
        presetListBox.selectRow (0, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (0);
        loadDefault (0);
    }
}

void PresetListComponent::loadDefault (int row)
{
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    // set the ID, since the default that was just loaded always has Id 1
    presetProperties.setId (row + 1, false);
    PresetProperties::copyTreeProperties (presetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
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

    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    PresetProperties::copyTreeProperties (assimil8orPreset.getPresetVT (), presetProperties.getValueTree ());
    PresetProperties::copyTreeProperties (presetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
}

void PresetListComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    auto toolRow { localBounds.removeFromTop (25) };
    showAllPresets.setBounds (toolRow.removeFromLeft (100));
    presetListBox.setBounds (localBounds);
}

int PresetListComponent::getNumRows ()
{
    return numPresets;
}

void PresetListComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < numPresets)
    {
        juce::Colour textColor;
        juce::Colour rowColor;
        if (rowIsSelected)
        {
            lastSelectedRow = row;
            rowColor = juce::Colours::darkslategrey;
            textColor = juce::Colours::yellow;
        }
        else
        {
            rowColor = juce::Colours::black;
            textColor = juce::Colours::whitesmoke;
        }
        auto [presetNumber, thisPresetExists] { presetExists [row] };
        if (! thisPresetExists)
            textColor = textColor.withAlpha (0.5f);
        g.setColour (rowColor);
        g.fillRect (width - 1, 0, 1, height);
        g.setColour (textColor);
        g.drawText ("  Preset " + juce::String (presetNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

juce::String PresetListComponent::getTooltipForRow (int row)
{
    return "Preset " + juce::String (row + 1);
}

void PresetListComponent::copyPreset (int presetNumber)
{
}

void PresetListComponent::pastePreset (int presetNumber)
{
}

void PresetListComponent::deletePreset (int presetNumber)
{
}


void PresetListComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (me.mods.isPopupMenu ())
    {
        presetListBox.selectRow (lastSelectedRow, false, true);
        const auto [presetNumber, thisPresetExists] { presetExists [row] };
        auto presetFile { rootFolder.getChildFile (getPresetName (presetNumber)).withFileExtension (".yml") };

        juce::PopupMenu pm;
        pm.addSectionHeader ("Preset " + juce::String(presetNumber + 1));
        pm.addItem ("Copy", thisPresetExists, false, [this, presetNumber = presetNumber] () { copyPreset (presetNumber); });
        pm.addItem ("Paste", copyBuffer.getName ().isNotEmpty(), false, [this, presetNumber = presetNumber] () { pastePreset (presetNumber); });
        pm.addItem ("Delete", thisPresetExists, false, [this, presetNumber = presetNumber] () { deletePreset (presetNumber); });
        pm.showMenuAsync ({}, [this] (int) {});
    }
    else
    {
        if (row == lastSelectedRow)
            return;

        auto completeSelection = [this, row] ()
            {
                const auto [presetNumber, thisPresetExists] = presetExists [row];
                auto presetFile { juce::File (appProperties.getMostRecentFolder ()).getChildFile (getPresetName (presetNumber)).withFileExtension (".yml") };
                if (thisPresetExists)
                    loadPreset (presetFile);
                else
                    loadDefault (row);
                presetListBox.selectRow (row, false, true);
                presetListBox.scrollToEnsureRowIsOnscreen (row);
                appProperties.addRecentlyUsedFile (presetFile.getFullPathName ());
            };

        if (overwritePresetOrCancel != nullptr)
        {
            presetListBox.selectRow (lastSelectedRow, false, true);
            overwritePresetOrCancel (completeSelection, [this] () {});
        }
        else
        {
            completeSelection ();
        }
    }
}
