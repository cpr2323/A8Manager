#include "MainComponent.h"
#include "../Utility/PersistentRootProperties.h"

const auto toolWindowHeight { 30 };

//  +----------------+-------------+-----------------------------------+
//  |Current Path                                                      |
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
//  |  Tool Bar                                                        |
//  +------------------------------------------------------------------+

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (1117, 609);

    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);

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

    folderBrowserEditorSplitter.setComponents (&fileViewComponent, &presetListEditorSplitter);
    folderBrowserEditorSplitter.setHorizontalSplit (false);

    topAndBottomSplitter.setComponents (&folderBrowserEditorSplitter, &assimil8orValidatorComponent);
    topAndBottomSplitter.setHorizontalSplit (true);

    presetListEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };
    folderBrowserEditorSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };
    topAndBottomSplitter.onLayoutChange = [this] () { saveLayoutChanges (); };

    restoreLayout ();

    addAndMakeVisible (currentFolderComponent);
    addAndMakeVisible (topAndBottomSplitter);
    addChildComponent (midiConfigComponent);
    addAndMakeVisible (bottomStatusWindow);

    fileViewComponent.onAudioFileSelected = [this] (juce::File audioFile) { assimil8orEditorComponent.receiveSampleLoadRequest (audioFile); };
}

void MainComponent::restoreLayout ()
{
    const auto [pane1Size, pane2Size, pane3Size] {guiProperties.getPaneSizes ()};
    presetListEditorSplitter.setSplitOffset (pane1Size);
    folderBrowserEditorSplitter.setSplitOffset (pane2Size);
    topAndBottomSplitter.setSplitOffset (pane3Size);
}

void MainComponent::saveLayoutChanges ()
{
    const auto splitter1Size { presetListEditorSplitter.getSplitOffset () };
    const auto splitter2Size { folderBrowserEditorSplitter.getSplitOffset () };
    const auto splitter3Size { topAndBottomSplitter.getSplitOffset () };
    guiProperties.setPaneSizes (splitter1Size, splitter2Size, splitter3Size, false);
}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.setColour (juce::Colours::red);
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    currentFolderComponent.setBounds (localBounds.removeFromTop (30));
    bottomStatusWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
    localBounds.reduce (3, 3);
    topAndBottomSplitter.setBounds (localBounds);
    midiConfigComponent.setBounds (localBounds);
}
