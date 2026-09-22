#include "MainComponent.h"
#include "Theme/A8ColourIds.h"
#include "oolib/Properties/PersistentRootProperties.h"

const auto kPathBarHeight { 34 };
const auto kStatusBarHeight { 26 };
// the background showing around and between the panes
const auto kPaneMargin { 5 };

//  +----------------+-------------+-----------------------------------+
//  |Current Path                                     | OUT | SETTINGS |
//  +----------------+-------------+-----------------------------------+
//  | ..             | Preset 1    |                                   |
//  | folderX        | Preset 2    |                                   |
//  | folder34       | Preset 3    |                                   |
//  | fileAbc        | Preset 4    |                                   |
//  | fileElif       | Preset ...  |                                   |
// ...              ...           ...                                 ...
//  |                | Preset 199  |                                   |
//  +----------------+-------------+-----------------------------------+
//  | Type | Fix | Message (X items)                                   |
//  +------+-----+-----------------------------------------------------+
//  |      |     |                                                     |
//  |      |     |                                                     |
//  |      |     |                                                     |
//  |      |     |                                                     |
//  |      |     |                                                     |
//  +------+-----+-----------------------------------------------------+
//  |  Status Bar                                                      |
//  +------------------------------------------------------------------+

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1117, 609);
    rootProperties = rootPropertiesVT;

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::yes);
    guiProperties.onShowSettingsDialog = [this] () { showSettingsDialog (); };

    fileViewComponent.overwritePresetOrCancel = [this] (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
    {
        assimil8orEditorComponent.overwritePresetOrCancel (overwriteFunction, cancelFunction);
    };
    presetListComponent.overwritePresetOrCancel = [this] (std::function<void ()> overwriteFunction, std::function<void ()> cancelFunction)
    {
        assimil8orEditorComponent.overwritePresetOrCancel (overwriteFunction, cancelFunction);
    };

    assimil8orEditorComponent.init (rootPropertiesVT);
    assimil8orValidatorComponent.init (rootPropertiesVT);
    fileViewComponent.init (rootPropertiesVT);
    presetListComponent.init (rootPropertiesVT);
    bottomStatusWindow.init (rootPropertiesVT);
    currentFolderComponent.init (rootPropertiesVT);
    midiConfigComponent.init (rootPropertiesVT);

    presetListEditorSplitter.setComponents (&presetListComponent, &assimil8orEditorComponent);
    presetListEditorSplitter.setHorizontalSplit (false);
    // the two inner splitters are nested inside the outer one, so they must not inset their panes a second time
    presetListEditorSplitter.setOuterMargin (0);

    folderBrowserEditorSplitter.setComponents (&fileViewComponent, &presetListEditorSplitter);
    folderBrowserEditorSplitter.setHorizontalSplit (false);
    folderBrowserEditorSplitter.setOuterMargin (0);

    topAndBottomSplitter.setComponents (&folderBrowserEditorSplitter, &assimil8orValidatorComponent);
    topAndBottomSplitter.setHorizontalSplit (true);
    topAndBottomSplitter.setOuterMargin (kPaneMargin);

    // the panes cannot be dragged narrower than their headers need
    folderBrowserEditorSplitter.constrainSplitOffset = [this] (int proposedSplitOffset) { return constrainFolderPaneOffset (proposedSplitOffset); };
    presetListEditorSplitter.constrainSplitOffset = [this] (int proposedSplitOffset) { return constrainPresetPaneOffset (proposedSplitOffset); };

    presetListEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };
    folderBrowserEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };
    topAndBottomSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };

    restoreLayout ();

    addAndMakeVisible (currentFolderComponent);
    addAndMakeVisible (topAndBottomSplitter);
    addChildComponent (midiConfigComponent);
    addAndMakeVisible (bottomStatusWindow);

    fileViewComponent.onAudioFileSelected = [this] (juce::File audioFile) { assimil8orEditorComponent.receiveSampleLoadRequest (audioFile); };

    // The theme is installed before any window exists, so the startup pass of
    // sendLookAndFeelChange reached nothing. Without this, every component that
    // applies its own colours in lookAndFeelChanged stays unstyled until the
    // background slider is first moved.
    sendLookAndFeelChange ();
}

void MainComponent::showSettingsDialog ()
{
    juce::DialogWindow::LaunchOptions options;
    options.escapeKeyTriggersCloseButton = true;
    options.dialogBackgroundColour = findColour (A8Colours::dialogBackground);
    options.dialogTitle = "SETTINGS";
    options.resizable = false;
    auto* settingsComponent { new SettingsDialogComponent () };
    settingsComponent->init (rootProperties);
    // sized to the largest page, measured after the pages are built
    settingsComponent->setBounds (settingsComponent->getPreferredBounds ());
    options.content.setOwned (settingsComponent);
    options.launchAsync ();
}

void MainComponent::restoreLayout ()
{
    const auto [pane1Size, pane2Size, pane3Size] { guiProperties.getPaneSizes () };
    presetListEditorSplitter.setSplitOffset (pane1Size);
    folderBrowserEditorSplitter.setSplitOffset (pane2Size);
    topAndBottomSplitter.setSplitOffset (pane3Size);
    applyMinimumPaneWidths ();
}

// A split offset is measured to the middle of its bar, so the pane before it is half
// a bar narrower than the offset.
constexpr auto kHalfSplitBar { 2 };

int MainComponent::constrainFolderPaneOffset (int proposedSplitOffset)
{
    return std::max (proposedSplitOffset, fileViewComponent.getMinimumWidth () + kHalfSplitBar);
}

int MainComponent::constrainPresetPaneOffset (int proposedSplitOffset)
{
    return std::max (proposedSplitOffset, presetListComponent.getMinimumWidth () + kHalfSplitBar);
}

// for offsets that did not come from a drag: the stored layout, which may predate the minimums
void MainComponent::applyMinimumPaneWidths ()
{
    if (const auto folderOffset { constrainFolderPaneOffset (folderBrowserEditorSplitter.getSplitOffset ()) };
        folderOffset != folderBrowserEditorSplitter.getSplitOffset ())
        folderBrowserEditorSplitter.setSplitOffset (folderOffset);
    if (const auto presetOffset { constrainPresetPaneOffset (presetListEditorSplitter.getSplitOffset ()) };
        presetOffset != presetListEditorSplitter.getSplitOffset ())
        presetListEditorSplitter.setSplitOffset (presetOffset);
}

void MainComponent::saveLayoutChanges ()
{
    const auto splitter1Size { presetListEditorSplitter.getSplitOffset () };
    const auto splitter2Size { folderBrowserEditorSplitter.getSplitOffset () };
    const auto splitter3Size { topAndBottomSplitter.getSplitOffset () };
    guiProperties.setPaneSizes (splitter1Size, splitter2Size, splitter3Size, false);
}

void MainComponent::paint (juce::Graphics& g)
{
    // the children do not cover every pixel, so this has to paint the gaps -
    // without it, a palette change leaves the old background showing through
    g.fillAll (findColour (A8Colours::windowBackground));
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    currentFolderComponent.setBounds (localBounds.removeFromTop (kPathBarHeight));
    bottomStatusWindow.setBounds (localBounds.removeFromBottom (kStatusBarHeight));
    topAndBottomSplitter.setBounds (localBounds);
    midiConfigComponent.setBounds (localBounds.reduced (kPaneMargin));
}
