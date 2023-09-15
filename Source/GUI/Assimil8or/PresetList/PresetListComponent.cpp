#include "PresetListComponent.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "../../../Assimil8or/PresetManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "../../../Utility/PersistentRootProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"
#include "../../../Utility/WatchDogTimer.h"

#define LOG_PRESET_LIST 1
#if LOG_PRESET_LIST
#define LogPresetList(text) juce::Logger::outputDebugString (text);
#else
#define LogPresetList(text) ;
#endif

PresetListComponent::PresetListComponent ()
{
    showAllPresets.setToggleState (true, juce::NotificationType::dontSendNotification);
    showAllPresets.onClick = [this] () { checkForPresets (); };
    addAndMakeVisible (showAllPresets);
    showAllPresets.setButtonText ("Show All");
    addAndMakeVisible (presetListBox);
}

void PresetListComponent::init (juce::ValueTree rootPropertiesVT)
{
    LogPresetList ("PresetListComponent::init");
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::no);

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onStatusChange = [this] (DirectoryDataProperties::ScanStatus status)
    {
        switch (status)
        {
            case DirectoryDataProperties::ScanStatus::empty:
            {
            }
            break;
            case DirectoryDataProperties::ScanStatus::scanning:
            {
            }
            break;
            case DirectoryDataProperties::ScanStatus::canceled:
            {
            }
            break;
            case DirectoryDataProperties::ScanStatus::done:
            {
                jassert (juce::MessageManager::getInstance ()->isThisTheMessageThread ());
                checkForPresets ();
            }
            break;
        }
    };
    PresetManagerProperties presetManagerProperties (runtimeRootProperties.getValueTree (), PresetManagerProperties::WrapperType::owner, PresetManagerProperties::EnableCallbacks::no);
    unEditedPresetProperties.wrap (presetManagerProperties.getPreset ("unedited"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);
    presetProperties.wrap (presetManagerProperties.getPreset ("edit"), PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::yes);

    checkForPresets ();
}

void PresetListComponent::forEachPresetFile (std::function<bool (juce::File presetFile, int index)> presetFileCallback)
{
    jassert (presetFileCallback != nullptr);


    auto directoryValueTreeVT { directoryDataProperties.getDirectoryValueTreeVT () };
    jassert (directoryValueTreeVT.getType ().toString () == "Folder");
    currentFolder = juce::File (directoryValueTreeVT.getProperty ("name").toString ());

    auto presetIndex { 0 };
    ValueTreeHelpers::forEachChild (directoryDataProperties.getDirectoryValueTreeVT (), [this, &presetIndex, presetFileCallback] (juce::ValueTree child)
    {
        auto presetFile { getPresetFile (presetIndex) };
        if (FileTypeHelpers::isPresetFile (presetFile) && ! presetFileCallback (presetFile, presetIndex))
            return false;
        ++presetIndex;
        return true;
    });
}

void PresetListComponent::checkForPresets ()
{
    WatchdogTimer timer;
    timer.start (100000);

    const auto showAll { showAllPresets.getToggleState () };

    // clear preset info list
    for (auto curPresetInfoIndex { 0 }; curPresetInfoIndex < presetInfoList.size (); ++curPresetInfoIndex)
        presetInfoList[curPresetInfoIndex] = {curPresetInfoIndex, false, ""};

    // find first preset file in list
    auto inPresetList { false };
    numPresets = kMaxPresets;
    ValueTreeHelpers::forEachChild (directoryDataProperties.getDirectoryValueTreeVT (), [this, &inPresetList] (juce::ValueTree child)
    {
        // TODO - still need to
        //    if (showAll)

        if (child.getType ().toString () == "File")
        {
            const auto fileToCheck { juce::File (child.getProperty ("name")) };
            if (FileTypeHelpers::isPresetFile (fileToCheck))
            {
                inPresetList = true;
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

                presetInfoList [presetIndex] = { presetIndex , true, presetName };
                //++numPresets;
            }
            else
            {
                // if the entry is not a preset file, but we were processing preset files, then we are done
                if (inPresetList)
                    return false;
            }
        }
        return true; // keep looking
    });

    presetListBox.updateContent ();
    if (currentFolder != previousFolder)
    {
        presetListBox.scrollToEnsureRowIsOnscreen (0);
        loadFirstPreset ();
        previousFolder = currentFolder;
    }
    presetListBox.repaint ();

    juce::Logger::outputDebugString ("PresetListComponent::checkPresets - elapsed time: " + juce::String (timer.getElapsedTime ()));
}

void PresetListComponent::loadFirstPreset ()
{
    LogPresetList ("PresetListComponent::loadFirstPreset");
    bool presetLoaded { false };
    forEachPresetFile ([this, &presetLoaded] (juce::File presetFile, int index)
    {
        if (auto [presetNumber, thisPresetExists, presetName] = presetInfoList [index]; ! thisPresetExists)
            return true;

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

void PresetListComponent::loadPresetFile (juce::File presetFile, juce::ValueTree presetPropertiesVT)
{
    juce::StringArray fileContents;
    presetFile.readLines (fileContents);

    Assimil8orPreset assimil8orPreset;
    assimil8orPreset.parse (fileContents);

    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetPropertiesVT);
    PresetProperties::copyTreeProperties (assimil8orPreset.getPresetVT (), presetPropertiesVT);
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
            lastSelectedRow = row;
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
        g.drawText ("  " + juce::String (presetNumber + 1) + "-" + presetName, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        //g.drawText ("  Preset " + juce::String (presetNumber + 1), juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
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
        auto [lastSelectedPresetNumber, thisPresetExists, presetName] = presetInfoList [lastSelectedRow];
        if (presetNumber == lastSelectedPresetNumber)
        {
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), unEditedPresetProperties.getValueTree ());
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), presetProperties.getValueTree ());
        }
    };

    auto [thisPresetNumber, thisPresetExists, presetName] = presetInfoList [lastSelectedRow];
    if (thisPresetExists)
    {
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "OVERWRITE PRESET", "Are you sure you want to overwrite '" + getPresetName (presetNumber) + "'", "YES", "NO", nullptr,
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
    juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, "DELETE PRESET", "Are you sure you want to delete '" + getPresetName(presetNumber) + "'", "YES", "NO", nullptr,
        juce::ModalCallbackFunction::create ([this, presetNumber, presetFile = getPresetFile (presetNumber)] (int option)
        {
            if (option == 0) // no
                return;
            presetFile.deleteFile ();
            // TODO handle delete error
            auto [lastSelectedPresetNumber, thisPresetExists, presetName] = presetInfoList [lastSelectedRow];
            if (presetNumber == lastSelectedPresetNumber)
                PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                                      unEditedPresetProperties.getValueTree ());
        }));
}

juce::File PresetListComponent::getPresetFile (int presetNumber)
{
    return currentFolder.getChildFile (getPresetName (presetNumber)).withFileExtension (".yml");
}

void PresetListComponent::listBoxItemClicked (int row, [[maybe_unused]] const juce::MouseEvent& me)
{
    if (me.mods.isPopupMenu ())
    {
        presetListBox.selectRow (lastSelectedRow, false, true);
        auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
        if (!thisPresetExists)
            presetName = "(preset)";

        juce::PopupMenu pm;
        pm.addSectionHeader (juce::String (presetNumber + 1) + "-" + presetName);
        pm.addItem ("Copy", thisPresetExists, false, [this, presetNumber = presetNumber] () { copyPreset (presetNumber); });
        pm.addItem ("Paste", copyBufferPresetProperties.getName ().isNotEmpty(), false, [this, presetNumber = presetNumber] () { pastePreset (presetNumber); });
        pm.addItem ("Delete", thisPresetExists, false, [this, presetNumber = presetNumber] () { deletePreset (presetNumber); });
        pm.showMenuAsync ({}, [this] (int) {});
    }
    else
    {
        if (row == lastSelectedRow)
            return;

        auto completeSelection = [this, row] ()
        {
            const auto [presetNumber, thisPresetExists, presetName] = presetInfoList [row];
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
            presetListBox.selectRow (lastSelectedRow, false, true);
            overwritePresetOrCancel (completeSelection, [this] () {});
        }
        else
        {
            completeSelection ();
        }
    }
}
