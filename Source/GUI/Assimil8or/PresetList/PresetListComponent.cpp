#include "PresetListComponent.h"
#include "../../Theme/A8ColourIds.h"
#include "../../../Assimil8or/Assimil8orPreset.h"
#include "../../../Assimil8or/FileTypeHelpers.h"
#include "../../../Assimil8or/PresetManagerProperties.h"
#include "../../../Assimil8or/Preset/ParameterPresetsSingleton.h"
#include "oolib/Debug/DebugLog.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"
#include "oolib/Debug/WatchDogTimer.h"

#define LOG_PRESET_LIST 0
#if LOG_PRESET_LIST
#define LogPresetList(text) DebugLog ("PresetListComponent", text);
#else
#define LogPresetList(text) ;
#endif

PresetListComponent::PresetListComponent ()
{
    setOpaque (true);
    // there is always a count, so the minimum width allows for one before the first scan has finished
    paneHeader.setCountText ("0/" + juce::String (kMaxPresets));
    addAndMakeVisible (paneHeader);
    showAllPresets.setClickingTogglesState (true);
    showAllPresets.setToggleState (true, juce::NotificationType::dontSendNotification);
    showAllPresets.setTooltip ("Show all Presets, Show only existing presets");
    showAllPresets.onClick = [this] ()
    {
        requestedShowAllPresets.store (showAllPresets.getToggleState ());
        requestPresetCheck ();
    };
    addAndMakeVisible (showAllPresets);
    presetListBox.setRowHeight (24);
    presetListBox.setOutlineThickness (0);
    addAndMakeVisible (presetListBox);

    checkPresetsThread.onThreadLoop = [this] ()
    {
        checkPresets (requestedShowAllPresets.load ());
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
    presetFileTypeId = directoryDataProperties.getFileTypeId (FileTypeHelpers::kPresetFileTypeName);
    directoryDataProperties.onRootScanComplete = [this] ()
    {
        LogPresetList ("PresetListComponent::init - directoryDataProperties.onRootScanComplete");
        requestPresetCheck ();
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

    checkPresetsThread.start ();
}

void PresetListComponent::requestPresetCheck ()
{
    if (! checkPresetsThread.isThreadRunning ())
    {
        LogPresetList ("PresetListComponent::requestPresetCheck - starting thread");
        checkPresetsThread.start ();
    }
    else
    {
        LogPresetList ("PresetListComponent::requestPresetCheck - starting timer");
        startTimer (1);
    }
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
            if (fileProperties.getType () == presetFileTypeId)
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

void PresetListComponent::checkPresets (bool showAll)
{
    WatchdogTimer timer;
    timer.start (100000);

    // this runs on the check presets thread, so we work from a detached snapshot of the live tree
    const auto rootFolderSnapshotVT { ValueTreeHelpers::getMessageThreadSnapshot (directoryDataProperties.getRootFolderVT ()) };
    if (! rootFolderSnapshotVT.isValid ())
        return;
    FolderProperties rootFolder (rootFolderSnapshotVT, FolderProperties::WrapperType::client, FolderProperties::EnableCallbacks::no);
    const auto scannedFolder { juce::File (rootFolder.getName ()) };
    PresetInfoList newPresetInfoList;

    // clear preset info list
    for (auto curPresetInfoIndex { 0 }; curPresetInfoIndex < newPresetInfoList.size (); ++curPresetInfoIndex)
        newPresetInfoList [curPresetInfoIndex] = { curPresetInfoIndex + 1, false, "" };

    auto newNumPresets { showAll ? kMaxPresets : 0 };
    auto inPresetList { false };
    ValueTreeHelpers::forEachChild (rootFolderSnapshotVT, [this, &inPresetList, &newNumPresets, &newPresetInfoList, showAll] (juce::ValueTree child)
    {
        if (FileProperties::isFileVT (child))
        {
            FileProperties fileProperties (child, FileProperties::WrapperType::client, FileProperties::EnableCallbacks::no);
            if (fileProperties.getType () == presetFileTypeId)
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
                    newPresetInfoList [presetIndex] = { presetIndex + 1 , true, presetName };
                else
                {
                    newPresetInfoList [newNumPresets] = { presetIndex + 1, true, presetName };
                    ++newNumPresets;
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

    juce::MessageManager::callAsync ([safeThis = juce::Component::SafePointer<PresetListComponent> (this),
                                      scannedFolder,
                                      newNumPresets,
                                      newPresetInfoList = std::move (newPresetInfoList)] () mutable
    {
        if (safeThis == nullptr)
            return;

        const auto newFolder { scannedFolder != safeThis->previousFolder };
        safeThis->currentFolder = scannedFolder;
        safeThis->numPresets = newNumPresets;
        safeThis->presetInfoList = std::move (newPresetInfoList);

        // the count is of the presets that exist, whether or not the empty slots are being shown
        auto presetsThatExist { 0 };
        for (const auto& presetInfo : safeThis->presetInfoList)
            if (std::get<1> (presetInfo))
                ++presetsThatExist;
        safeThis->paneHeader.setCountText (juce::String (presetsThatExist) + "/" + juce::String (kMaxPresets));
        // the tool button is placed against the count, which may have changed width
        safeThis->resized ();

        safeThis->presetListBox.updateContent ();
        if (newFolder)
        {
            safeThis->presetListBox.scrollToEnsureRowIsOnscreen (0);
            safeThis->loadFirstPreset ();
        }
        safeThis->presetListBox.repaint ();
        safeThis->previousFolder = scannedFolder;
    });

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

    // then load new preset
    PresetProperties::copyTreeProperties (assimil8orPreset.getPresetVT (), presetPropertiesVT);

    //ValueTreeHelpers::dumpValueTreeContent (presetPropertiesVT, true, [this] (juce::String text) { juce::Logger::outputDebugString (text); });
}

void PresetListComponent::loadPreset (juce::File presetFile)
{
    loadPresetFile (presetFile, unEditedPresetProperties.getValueTree ());
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    PresetProperties::copyTreeProperties (unEditedPresetProperties.getValueTree (), presetProperties.getValueTree ());
}

int PresetListComponent::getMinimumWidth () const
{
    // plus the outline on either side
    return paneHeader.getRequiredWidth (showAllPresets.getIdealWidth ()) + 2;
}

void PresetListComponent::resized ()
{
    // inside the pane's outline
    auto localBounds { getLocalBounds ().reduced (1) };
    auto headerBounds { localBounds.removeFromTop (PaneHeader::kHeight) };
    paneHeader.setBounds (headerBounds);
    // right aligned against the count
    showAllPresets.setBounds ((paneHeader.getFreeBounds () + headerBounds.getPosition ()).removeFromRight (showAllPresets.getIdealWidth ()));
    presetListBox.setBounds (localBounds);
}

void PresetListComponent::paint (juce::Graphics& g)
{
    // opaque, so the background behind the rounded corners is this component's to paint
    g.fillAll (findColour (A8Colours::windowBackground));
    A8Paint::card (g, *this, getLocalBounds ());
}

int PresetListComponent::getNumRows ()
{
    return numPresets;
}

void PresetListComponent::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < numPresets)
    {
        if (rowIsSelected)
            lastSelectedPresetIndex = row;
        const auto hovered { row == rowHover.getRow () };

        auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
        auto nameColourId { static_cast<int> (rowIsSelected ? A8Colours::accentText
                                                            : (hovered ? A8Colours::text : A8Colours::textDim)) };
        if (! thisPresetExists)
        {
            presetName = "empty";
            if (! rowIsSelected)
                nameColourId = A8Colours::textGhost;
        }
        else if (presetName.isEmpty ())
        {
            presetName = "(no name)";
        }

        if (rowIsSelected)
        {
            g.fillAll (findColour (A8Colours::selectedRow));
            g.setColour (findColour (A8Colours::accent));
            g.fillRect (0, 0, 2, height);
        }

        auto rowBounds { juce::Rectangle<int> { 0, 0, width, height }.reduced (8, 0) };

        // the lit dot says the preset file exists, so the name does not have to
        const auto ledBounds { rowBounds.removeFromRight (static_cast<int> (StatusLed::kDiameter)).toFloat ()
                                        .withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
        StatusLed::draw (g, ledBounds, thisPresetExists, *this);
        rowBounds.removeFromRight (7);

        g.setFont (A8Type::presetNumber ());
        g.setColour (findColour (rowIsSelected ? A8Colours::accentText : A8Colours::textGhost));
        g.drawText (juce::String (presetNumber), rowBounds.removeFromLeft (24), juce::Justification::centredRight, false);
        rowBounds.removeFromLeft (7);

        g.setFont (A8Type::body ());
        g.setColour (findColour (nameColourId));
        g.drawText (presetName, rowBounds, juce::Justification::centredLeft, true);

        if (hovered)
            ListRowHover::paintOutline (g, *this, width, height);
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

void PresetListComponent::movePresetUp (int row)
{
    jassert (row > 1);
    swapPresets (row, row - 1);
}

void PresetListComponent::movePresetDown (int row)
{
    jassert (row < kMaxPresets);
    swapPresets (row, row + 1);
}

void PresetListComponent::swapPresets (int fromRow, int toRow)
{
    auto [moveFromPresetNumber, moveFromPresetExists, moveFromPresetName] { presetInfoList [fromRow] };
    jassert (moveFromPresetExists == true);
    auto [moveToPresetNumber, moveToPresetExists, moveToPresetName] { presetInfoList [toRow] };
    auto newFile { getPresetFile (moveToPresetNumber) };
    auto oldFile { getPresetFile (moveFromPresetNumber) };
    if (! moveToPresetExists)
    {
        // rename preset file
        oldFile.moveFileTo (newFile);
    }
    else
    {
        // rename both preset files
        newFile.moveFileTo (newFile.withFileExtension ("tmp"));
        oldFile.moveFileTo (newFile);
        newFile.withFileExtension ("tmp").moveFileTo (oldFile);
    }
}

juce::String PresetListComponent::getTooltipForRow (int row)
{
    auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
    return "Preset " + juce::String (presetNumber);
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
            unEditedPresetProperties.setId (presetNumber, false);
            PresetProperties::copyTreeProperties (copyBufferPresetProperties.getValueTree (), presetProperties.getValueTree ());
            presetProperties.setId (presetNumber, false);
        }
    };

    auto [thisPresetNumber, thisPresetExists, presetName] { presetInfoList [lastSelectedPresetIndex] };
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
            {
                auto defaultPreset { ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType) };
                PresetProperties::copyTreeProperties (defaultPreset, unEditedPresetProperties.getValueTree ());
                unEditedPresetProperties.setId (presetNumber, false);
                PresetProperties::copyTreeProperties (defaultPreset, presetProperties.getValueTree ());
                presetProperties.setId (presetNumber, false);
            }
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
        if (row != lastSelectedPresetIndex)
            presetListBox.selectRow (lastSelectedPresetIndex, true, true);

        auto [presetNumber, thisPresetExists, presetName] { presetInfoList [row] };
        if (! thisPresetExists)
            presetName = "(preset)";

        juce::PopupMenu pm;
        pm.addSectionHeader (juce::String (presetNumber) + " - " + presetName);
        pm.addSeparator ();
        pm.addItem ("Copy", thisPresetExists, false, [this, presetNumber = presetNumber] () { copyPreset (presetNumber); });
        pm.addItem ("Paste", copyBufferPresetProperties.getName ().isNotEmpty (), false, [this, presetNumber = presetNumber] () { pastePreset (presetNumber); });
        pm.addItem ("Delete", thisPresetExists, false, [this, presetNumber = presetNumber] () { deletePreset (presetNumber); });
        {
            juce::PopupMenu moveMenu;
            moveMenu.addItem ("Up", presetNumber > 1, false, [this, row] () { movePresetUp (row); });
            moveMenu.addItem ("Down", presetNumber < kMaxPresets, false, [this, row] () { movePresetDown (row); });
            pm.addSubMenu ("Move", moveMenu, thisPresetExists);
        }
        pm.showMenuAsync ({});
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
