#include "PresetListComponent.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "../../../Assimil8or/PresetManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/DebugLog.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/WatchDogTimer.h"

#define LOG_PRESET_LIST 0
#if LOG_PRESET_LIST
#define LogPresetList(text) DebugLog ("PresetListComponent", text);
#else
#define LogPresetList(text) ;
#endif

PresetListComponent::PresetListComponent ()
{
    showAllPresets.setToggleState (true, juce::NotificationType::dontSendNotification);
    showAllPresets.setButtonText ("Show All");
    showAllPresets.onClick = [this] () { checkPresetsThread.start (); };
    addAndMakeVisible (showAllPresets);
    addAndMakeVisible (presetListBox);

    checkPresetsThread.onThreadLoop = [this] ()
    {
        checkPresets ();
        return false;
    };
}

void PresetListComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogPresetList ("PresetListComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onRootScanComplete = [this] ()
    {
        LogPresetList ("PresetListComponent::init - directoryDataProperties.onRootScanComplete");
        if (! checkPresetsThread.isThreadRunning ())
        {
            LogPresetList ("PresetListComponent::init - directoryDataProperties.onRootScanComplete - starting thread");
            checkPresetsThread.startThread ();
        }
        else
        {
            LogPresetList ("PresetListComponent::init - directoryDataProperties.onRootScanComplete - starting timer");
            startTimer (1);
        }
    };
//     directoryDataProperties.onStatusChange = [this] (DirectoryDataProperties::ScanStatus status)
//     {
//         switch (status)
//         {
//             case DirectoryDataProperties::ScanStatus::empty:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::scanning:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::canceled:
//             {
//             }
//             break;
//             case DirectoryDataProperties::ScanStatus::done:
//             {
//                 checkPresetsThread.startThread ();
//             }
//             break;
//         }
//     };
    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    checkPresetsThread.startThread ();
}

void PresetListComponent::forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback)
{
    jassert (presetFileCallback != nullptr);

    auto inPresetList { false };
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, presetFileCallback, &inPresetList] (juce::ValueTree child)
    {
        if (FileProperties::isFileVT (child))
        {
            FileProperties fileProperties (child, FileProperties::WrapperType::client, FileProperties::EnableCallbacks::no);
            if (fileProperties.getType ()== DirectoryDataProperties::presetFile)
            {
                inPresetList = true;
                const auto fileToCheck { juce::File (fileProperties.getName ()) };
                const auto presetIndex { FileTypeHelpers::getPresetNumberFromName (fileToCheck) - 1 };
                if (presetIndex < 0 || presetIndex >= kMaxPresets)
                    return false;
                if (! presetFileCallback (fileToCheck, presetIndex))
                    return false;
            }
            else
            {
                // if the entry is not a preset file, but we were processing preset files, then we are done
                if (inPresetList)
                    return false;
            }
        }
        return true;
    });
}

void PresetListComponent::checkPresets ()
{
    WatchdogTimer timer;
    timer.start (100000);

    FolderProperties rootFolder (directoryDataProperties.getRootFolderVT (), FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    currentFolder = juce::File (rootFolder.getName ());

    const auto showAll { showAllPresets.getToggleState () };

    // clear preset info list
    for (auto curPresetInfoIndex { 0 }; curPresetInfoIndex < presetInfoList.size (); ++curPresetInfoIndex)
        presetInfoList[curPresetInfoIndex] = { curPresetInfoIndex + 1, false, "" };

    if (showAll)
        numPresets = kMaxPresets;
    else
        numPresets = 0;
    auto inPresetList { false };
    ValueTreeHelpers::forEachChild (directoryDataProperties.getRootFolderVT (), [this, &inPresetList, showAll] (juce::ValueTree child)
    {
        if (FileProperties::isFileVT (child))
        {
            FileProperties fileProperties (child, FileProperties::WrapperType::client, FileProperties::EnableCallbacks::no);
            if (fileProperties.getType () == DirectoryDataProperties::TypeIndex::presetFile)
            {
                inPresetList = true;
                const auto fileToCheck { juce::File (fileProperties.getName ()) };
                const auto presetIndex { FileTypeHelpers::getPresetNumberFromName (fileToCheck) - 1 };

                if (presetIndex >= kMaxPresets)
                    return true;
                juce::String presetName;
                juce::StringArray fileContents;
                fileToCheck.readLines (fileContents);
                Assimil8orPreset assimil8orPreset;
                assimil8orPreset.parse (fileContents);
                PresetProperties thisPresetProperties (assimil8orPreset.getPresetVT (), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
                presetName = thisPresetProperties.getName ();

                if (showAll)
                    presetInfoList [presetIndex] = { presetIndex + 1 , true, presetName };
                else
                {
                    presetInfoList [numPresets] = { presetIndex + 1, true, presetName };
                    ++numPresets;
                }
            }
            else
            {
                // if the entry is not a preset file, but we had started processing preset files, then we are done, because the files are sorted by type
                if (inPresetList)
                    return false;
            }
        }
        return true; // keep looking
    });

    juce::MessageManager::callAsync ([this, newFolder = (currentFolder != previousFolder)] ()
    {
        presetListBox.updateContent ();
        if (newFolder)
        {
            presetListBox.scrollToEnsureRowIsOnscreen (0);
            loadFirstPreset ();
        }
        presetListBox.repaint ();
    });
    previousFolder = currentFolder;

    //juce::Logger::outputDebugString ("PresetListComponent::checkPresets - elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void PresetListComponent::loadFirstPreset ()
{
    LogPresetList ("PresetListComponent::loadFirstPreset");
    bool presetLoaded { false };
    juce::File loadedPresetFile;
    forEachPresetFile ([this, &presetLoaded, &loadedPresetFile] (juce::File presetFile, int presetIndex)
    {
        if (auto [presetNumber, thisPresetExists, presetName] { presetInfoList [presetIndex] }; ! thisPresetExists)
            return true;

        presetListBox.selectRow (presetIndex, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (presetIndex);
        loadedPresetFile = presetFile;
        appProperties.addRecentlyUsedFile (loadedPresetFile.getFullPathName ());
        loadPreset (presetFile);
        presetLoaded = true;
        return false;
    });

    if (! presetLoaded)
    {
        presetListBox.selectRow (0, false, true);
        presetListBox.scrollToEnsureRowIsOnscreen (0);
        loadedPresetFile = getPresetFile (1);
        appProperties.addRecentlyUsedFile (loadedPresetFile.getFullPathName ());
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

void PresetListComponent::loadPresetFile (juce::File presetFile, juce::ValueTree presetPropertiesVT)
{
    juce::StringArray fileContents;
    presetFile.readLines (fileContents);

    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.parse (fileContents);

    // TODO - is this redundant, since assimil8orPreset.parse resets it's PresetProperties to defaults already
    // first set preset to defaults
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetPropertiesVT);
    // then load new preset
    PresetProperties::copyTreeProperties (assimil8orPreset.getPresetVT (), presetPropertiesVT);

    //ValueTreeHelpers::dumpValueTreeContent (presetPropertiesVT, true, [this] (juce::String text) { juce::Logger::outputDebugString (text); });
}

void PresetListComponent::loadPreset (juce::File presetFile)
{
    loadPresetFile (presetFile, presetProperties.getValueTree ());
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
            lastSelectedPresetIndex = row;
            rowColor = juce::Colours::darkslategrey;
            textColor = juce::Colours::yellow;
        }
        else
        {
            rowColor = juce::Colours::black;
            textColor = juce::Colours::whitesmoke;
        }
        auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
        if (thisPresetExists)
        {

        }
        else
        {
            presetName = "(preset)";
            textColor = textColor.withAlpha (0.5f);
        }
        g.setColour (rowColor);
        g.fillRect (width - 1, 0, 1, height);
        g.setColour (textColor);
        g.drawText ("  " + juce::String (presetNumber) + "-" + presetName, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

void PresetListComponent::timerCallback ()
{
    LogPresetList ("PresetListComponent::timerCallback - enter");
    if (! checkPresetsThread.isThreadRunning ())
    {
        LogPresetList ("PresetListComponent::timerCallback - starting thread, stopping timer");
        checkPresetsThread.start ();
        stopTimer ();
    }
    LogPresetList ("PresetListComponent::timerCallback - enter");
}

juce::String PresetListComponent::getTooltipForRow (int row)
{
    return "Preset " + juce::String (row + 1);
}

void PresetListComponent::copyPreset (int presetNumber)
{
    loadPresetFile (getPresetFile (presetNumber), copyBufferPresetProperties.getValueTree ());
}

void PresetListComponent::pastePreset (int presetNumber)
{
    auto doPaste = [this, presetNumber] ()
    {
        Assimil8orPreset assimil8orPreset;
        PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), assimil8orPreset.getPresetVT ());
        assimil8orPreset.write (getPresetFile (presetNumber));
        auto [lastSelectedPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex] };
        if (presetNumber == lastSelectedPresetNumber)
        {
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), presetProperties.getValueTree ());
        }
    };

    auto [thisPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex]};
    if (thisPresetExists)
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "OVERWRITE PRESET", "Are you sure you want to overwrite '" + FileTypeHelpers::getPresetFileName (presetNumber) + "'", "YES", "NO", nullptr,
            juce::ModalCallbackFunction::create ([this, doPaste] (int option)
            {
                if (option == 0) // no
                    return;
                doPaste ();
            }));
    }
    else
    {
        doPaste ();
    }
}

void PresetListComponent::deletePreset (int presetNumber)
{
    juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE PRESET", "Are you sure you want to delete '" + FileTypeHelpers::getPresetFileName (presetNumber) + "'", "YES", "NO", nullptr,
        juce::ModalCallbackFunction::create ([this, presetNumber, presetFile = getPresetFile (presetNumber)] (int option)
        {
            if (option == 0) // no
                return;
            presetFile.deleteFile ();
            // TODO handle delete error
            auto [lastSelectedPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex] };
            if (presetNumber == lastSelectedPresetNumber)
                PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                                      unEditedPresetProperties.getValueTree ());
        }));
}

juce::File PresetListComponent::getPresetFile (int presetNumber)
{
    return currentFolder.getChildFile (FileTypeHelpers::getPresetFileName (presetNumber)).withFileExtension (".yml");
}

void PresetListComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (me.mods.isPopupMenu ())
    {
        presetListBox.selectRow (lastSelectedPresetIndex, false, true);
        auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
        if (! thisPresetExists)
            presetName = "(preset)";

        juce::PopupMenu pm;
        pm.addSectionHeader (juce::String (presetNumber) + "-" + presetName);
        pm.addItem ("Copy", thisPresetExists, false, [this, presetNumber = presetNumber] () { copyPreset (presetNumber); });
        pm.addItem ("Paste", copyBufferPresetProperties.getName ().isNotEmpty (), false, [this, presetNumber = presetNumber] () { pastePreset (presetNumber); });
        pm.addItem ("Delete", thisPresetExists, false, [this, presetNumber = presetNumber] () { deletePreset (presetNumber); });
        pm.showMenuAsync ({}, [this] (int) {});
    }
    else
    {
        // don't reload the currently loaded preset
        if (row == lastSelectedPresetIndex)
            return;

        auto completeSelection = [this, row] ()
        {
            auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
            auto presetFile { getPresetFile (presetNumber) };
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
            presetListBox.selectRow (lastSelectedPresetIndex, false, true);
            overwritePresetOrCancel (completeSelection, [this] () {});
        }
        else
        {
            completeSelection ();
        }
    }
}
