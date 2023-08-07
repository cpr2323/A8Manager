#pragma once

#include <JuceHeader.h>
#include "ToolWindow.h"
#include "Assimil8or/Editor/Assimil8orEditorComponent.h"
#include "Assimil8or/FileView/FileViewComponent.h"
#include "Assimil8or/FileView/FileViewComponent.h"
#include "Assimil8or/PresetList/PresetListComponent.h"
#include "Assimil8or/Validator/Assimil8orValidatorComponent.h"
#include "Assimil8or/Validator/Assimil8orValidatorComponent.h"
#include "../AppProperties.h"
#include "../Utility/SplitWindowComponent.h"

//  +----------------+-------------+-----------------------------------+
//  |Folder Contents | Preset List | Preset Editor                     |
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

class MainComponent : public juce::Component
{
public:
    MainComponent (juce::ValueTree rootPropertiesVT);
    ~MainComponent () override = default;

    void startFolderScan (juce::File folderToScan);

private:
    AppProperties appProperties;
    ValidatorProperties validatorProperties;

    juce::Component component;
    FileViewComponent fileViewComponent;
    PresetListComponent presetListComponent;
    Assimil8orEditorComponent assimil8orEditorComponent;
    Assimil8orValidatorComponent assimil8orValidatorComponent;

    SplitWindowComponent topAndBottomSplitter;
    SplitWindowComponent presetListEditorSplitter;
    SplitWindowComponent folderBrowserEditorSplitter;

    ToolWindow toolWindow;
    juce::TooltipWindow tooltipWindow;

    void resized () override;
    void paint (juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
