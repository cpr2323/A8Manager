#pragma once

#include <JuceHeader.h>
#include "MidiSetupEditorComponent.h"
#include "../../GuiControlProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/MidiSetup/MidiSetupProperties.h"

class MidiConfigDialogComponent : public juce::Component,
                                         juce::Timer
{
public:
    MidiConfigDialogComponent ();
    void init (juce::ValueTree rootPropertiesVT);
    void handleShowChange (bool show);

private:
    AppProperties appProperties;
    GuiControlProperties guiControlProperties;
    juce::ValueTree midiSetupPropertiesListVT { "MidiSetupPropertiesList" };
    juce::ValueTree uneditedMidiSetupPropertiesListVT { "MidiSetupPropertiesList" };

    juce::TabbedComponent midiSetupTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    juce::TextButton saveButton;
    juce::TextButton cancelButton;
    std::array<MidiSetupEditorComponent, 9> midiSetupEditorComponents;
    bool anyMidiSetupsEdited { false };

    void cancelClicked ();
    void closeDialog ();
    void loadMidiSetups ();
    void saveClicked ();

    void timerCallback () override;
    void resized () override;
    void paint (juce::Graphics& g) override;
};