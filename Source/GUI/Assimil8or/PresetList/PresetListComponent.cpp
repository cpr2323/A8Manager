#include "PresetListComponent.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"

#define LOG_PRESET_LIST 0
#if LOG_PRESET_LIST
#define LogPresetList(text) juce::Logger::outputDebugString (text);
#else
#define LogPresetList(text) ;
#endif

PresetListComponent::PresetListComponent () : Thread ("PresetListComponent")
{
    addAndMakeVisible (presetListBox);
    startThread ();
}

PresetListComponent::~PresetListComponent ()
{
    stopThread (100);
}

void PresetListComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogPresetList ("PresetListComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    presetProperties.wrap (runtimeRootProperties.getValueTree (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    appActionProperties.wrap (runtimeRootProperties.getValueTree (), AppActionProperties::WrapperType::client, AppActionProperties::EnableCallbacks::no);
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
        checkForPresets ();
    }
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

void PresetListComponent::checkForPresets ()
{
    LogPresetList ("PresetListComponent::checkForPresets");
    forEachPresetFile ([this] (juce::File presetFile, int index)
    {
        presetExists [index] = presetFile.exists ();
        return ! shouldCancelOperation ();
    });

    if (! shouldCancelOperation ())
        juce::MessageManager::callAsync ([this] ()
        {
            presetListBox.updateContent ();
            presetListBox.scrollToEnsureRowIsOnscreen (0);
            presetListBox.repaint ();
            loadFirstPreset ();
        });
}

void PresetListComponent::loadFirstPreset ()
{
    LogPresetList ("PresetListComponent::loadFirstPreset");
    bool presetLoaded { false };
    forEachPresetFile ([this, &presetLoaded] (juce::File presetFile, int index)
    {
        if (! presetExists [index])
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
    appActionProperties.setPresetLoadState (true);
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance()->getParameterPresetListProperties().getParameterPreset(ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    // set the ID, since the default that was just loaded always has Id 1
    presetProperties.setId (row + 1, false);
    appActionProperties.setPresetLoadState (false);
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

    appActionProperties.setPresetLoadState (true);
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    PresetProperties::copyTreeProperties (assimil8orPreset.getPresetVT (), presetProperties.getValueTree ());
    appActionProperties.setPresetLoadState (false);
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
        loadDefault (row);
    appProperties.addRecentlyUsedFile (presetFile.getFullPathName ());
}
