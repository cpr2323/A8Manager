#pragma once

#include <JuceHeader.h>
#include "../../GuiControlProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/MidiSetup/MidiSetup.h"

// Mode : Omni, Uni, Multi - 0,1,2
// Assignment : One set of values for Omni/Uni and another for Multi. The unit seems to remember the setting for each of these
//              Dynamic, Dynamic II, Chroma 8, Chroma VZ, Chroma KZ, SP-1200, SP-1200 X - (0-6)
//              Multi, Multi VZ, Multi KZ, No Trigger (10-13)
// Basic Channel : (1 - 16)
// Rcv Program Change : Off (0), Exists (1), Any (2)
// Xmt Program Change : Off (0), On (1)
// Col A CC : <= CV in Jacks, -1
//            <= CC# (0-95), 0 - 95
//            <= CC# 16-23, 96
//            <= CC# 24-31, 97
//            <= CC# 104-111, 98
//            <= CC# 112-119, - 99
//            <= Pressure, 100
//            <= Velocity, 101
//            <= Note #, 102
//            0..5V => Tx 16+, 103
//            0..5V => Tx 24+, 104
//            0..5V => Tx 104+, 105
//            0..5V => Tx 112+, 106
//            -5..5V => Tx 16+, 107
//            -5..5V => Tx 24+, 108
//            -5..5V => Tx 104+, 109
//            -5..5V => Tx 112+, 110
//            0..5V => multi 16, 111
//            0..5V => multi 24, 112
//            0..5V => multi 104+, 113
//            0..5V => multi 112+, 114
//            -5..5V => multi 16, 115
//            -5..5V => multi 24, 116
//            -5..5V => multi 104+, 117
//            -5..5V => multi 112+, 118
// Col B CC : [same as Col A CC]
// Col C CC : [same as Col A CC]
// Pitchwheel : +/- (0-12)
// Velocity Depth : (0-127)
// Notification: On/Off

class MidiConfigComponent : public juce::Component
{
public:
    MidiConfigComponent ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    class MidiSetupComponent : public juce::Component
    {

    };
    class MidiConfigComponentDialog : public juce::Component
    {
    public:
        MidiConfigComponentDialog ();
        void init (juce::ValueTree rootPropertiesVT);
        void loadMidiSetups ();

    private:
        AppProperties appProperties;
        GuiControlProperties guiControlProperties;
        juce::TabbedComponent midiSetupTabs { juce::TabbedButtonBar::Orientation::TabsAtTop };
        juce::TextButton saveButton;
        juce::TextButton cancelButton;
        std::array<MidiSetupComponent, 9> midiSetupComponents;
        std::array<MidiSetupProperties, 9> midiSetupPropertiesList;
        std::array<MidiSetupProperties, 9> unEditedMidiSetupPropertiesList;

        void cancelClicked ();
        void closeDialog ();
        void saveClicked ();

        void resized () override;
        void paint (juce::Graphics& g) override;
    };

    GuiControlProperties guiControlProperties;
    MidiConfigComponentDialog midiConfigComponentDialog;

    void resized () override
    {
        constexpr auto kContentWidth { 400 };
        constexpr auto kContentHeight { 400 };
        midiConfigComponentDialog.setBounds (getWidth () / 2 - kContentWidth / 2, 27, kContentWidth, kContentHeight);
    }
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::grey.withAlpha (0.5f));
    }
    void handleShowChange (bool show);
};
