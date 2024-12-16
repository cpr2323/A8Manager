#pragma once

#include <JuceHeader.h>
#include "MidiConfigDialogComponent.h"
#include "../../GuiControlProperties.h"

// Component that holds the dialog component, and blocks interaction with the rest of the window (faking modality)
class MidiConfigComponent : public juce::Component
{
public:
    MidiConfigComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    GuiControlProperties guiControlProperties;
    MidiConfigDialogComponent midiConfigComponentDialog;

    void handleShowChange (bool show);

    void resized () override;
    void paint (juce::Graphics& g) override;
};