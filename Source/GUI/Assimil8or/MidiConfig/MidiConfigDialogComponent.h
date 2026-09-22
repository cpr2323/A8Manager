#pragma once

#include <JuceHeader.h>
#include "MidiSetupEditorComponent.h"
#include "../../Theme/UiComponents.h"
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

    LedTabbedComponent midiSetupTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
    ActionButton saveButton { "SAVE" };
    ActionButton cancelButton { "CANCEL" };
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