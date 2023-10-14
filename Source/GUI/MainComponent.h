#pragma once

#include <JuceHeader.h>
#include "CurrentFolderComponent.h"
#include "GuiProperties.h"
#include "ToolWindow.h"
#include "Assimil8or/Editor/Assimil8orEditorComponent.h"
#include "Assimil8or/FileView/FileViewComponent.h"
#include "Assimil8or/FileView/FileViewComponent.h"
#include "Assimil8or/PresetList/PresetListComponent.h"
#include "Assimil8or/Validator/Assimil8orValidatorComponent.h"
#include "../Utility/SplitWindowComponent.h"

class MainComponent : public juce::Component
{
public:
    MainComponent (juce::ValueTree rootPropertiesVT);
    ~MainComponent () = default;

private:
    Assimil8orEditorComponent assimil8orEditorComponent;
    Assimil8orValidatorComponent assimil8orValidatorComponent;
    GuiProperties guiProperties;
    CurrentFolderComponent currentFolderComponent;
    FileViewComponent fileViewComponent;
    PresetListComponent presetListComponent;
    SplitWindowComponent topAndBottomSplitter;
    SplitWindowComponent presetListEditorSplitter;
    SplitWindowComponent folderBrowserEditorSplitter;
    ToolWindow toolWindow;

    juce::TooltipWindow tooltipWindow;

    void restoreLayout ();
    void saveLayoutChanges ();

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
