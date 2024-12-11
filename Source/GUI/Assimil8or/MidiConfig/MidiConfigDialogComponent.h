#pragma once

#include <JuceHeader.h>
#include "MidiSetupEditorComponent.h"
#include "../../GuiControlProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/MidiSetup/MidiSetupProperties.h"

class MidiConfigDialogComponent : public juce::Component
{
public:
    MidiConfigDialogComponent ();
    void init (juce::ValueTree rootPropertiesVT);
    void loadMidiSetups ();

private:
    AppProperties appProperties;
    GuiControlProperties guiControlProperties;
    juce::TabbedComponent midiSetupTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    juce::TextButton saveButton;
    juce::TextButton cancelButton;
    std::array<MidiSetupEditorComponent, 9> midiSetupEditorComponents;
    std::array<MidiSetupProperties, 9> midiSetupPropertiesList;
    std::array<MidiSetupProperties, 9> unEditedMidiSetupPropertiesList;

    void cancelClicked ();
    void closeDialog ();
    void saveClicked ();

    void resized () override;
    void paint (juce::Graphics& g) override;
};