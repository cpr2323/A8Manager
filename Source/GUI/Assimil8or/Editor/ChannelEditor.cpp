#include "ChannelEditor.h"

ChannelEditor::ChannelEditor ()
{
//     juce::Label aliasingLabel;
//     juce::TextEditor aliasingTextEdit; // integer
//     juce::Label aliasingModLabel;
//     CvInputChannelComboBox aliasingModComboBox;
//     juce::TextEditor aliasingModTextEditor;
//     juce::Label attackLabel;
//     juce::TextEditor attackTextEditor; // double
//     juce::Label attackFromCurrentLabel;
//     juce::ToggleButton attackFromCurrentCheckBox; // false = start from zero, true = start from last value
//     juce::Label attackModLabel;
//     CvInputChannelComboBox attackModComboBox;
//     juce::TextEditor attackModTextEditor;
//     juce::Label autoTriggerLabel;
//     juce::ToggleButton autoTriggerCheckBox; //
//     juce::Label bitsLabel;
//     juce::TextEditor bitsTextEditor; // double
//     juce::Label bitsModLabel;
//     CvInputChannelComboBox bitsModComboBox;
//     juce::TextEditor bitsModTextEditor;
//     juce::Label channelModeLabel;
//     juce::ComboBox channelModeComboBox; // 4 Channel Modes: 0 = Master, 1 = Link, 2 = Stereo/Right, 3 = Cycle
//     juce::Label expAMLabel;
//     juce::TextEditor expAMTextEditor;
//     juce::Label expFMLabel;
//     juce::TextEditor expFMTextEditor;
//     juce::Label levelLabel;
//     juce::TextEditor LevelTextEditor;
//     juce::Label linAMLabel;
//     juce::TextEditor linAMTextEditor;
//     juce::Label linAMisExtEnvLabel;
//     juce::ToggleButton linAMisExtEnvCheckBox; // 
//     juce::Label linFMLabel;
//     juce::TextEditor linFMTextEditor;
//     juce::Label loopLengthModLabel;
//     CvInputChannelComboBox loopLengthModComboBox;
//     juce::TextEditor loopLengthModTextEditor;
//     juce::Label loopModeLabel;
//     juce::ComboBox loopModeComboBox; // 0 = No Loop, 1 = Loop, 2 = Loop and Release
//     juce::Label loopStartModLabel;
//     CvInputChannelComboBox loopStartModComboBox;
//     juce::TextEditor loopStartModTextEditor;
//     juce::Label mixLevelLabel;
//     juce::TextEditor mixLevelTextEditor;
//     juce::Label mixModLabel;
//     CvInputChannelComboBox mixModComboBox;
//     juce::TextEditor mixModTextEditor;
//     juce::Label mixModIsFaderLabel;
//     juce::ToggleButton mixModIsFaderCheckBox; // 
//     juce::Label panLabel;
//     juce::TextEditor panTextEditor;
//     juce::Label panModLabel;
//     CvInputChannelComboBox panModComboBox;
//     juce::TextEditor panModTextEditor;
//     juce::Label phaseCVLabel;
//     CvInputChannelComboBox phaseCVComboBox;
//     juce::TextEditor phaseCVTextEditor;
//     juce::Label pitchLabel;
//     juce::TextEditor pitchTextEditor;
//     juce::Label pitchCVLabel;
//     CvInputChannelComboBox pitchCVComboBox;
//     juce::TextEditor pitchCVTextEditor;
//     juce::Label playModeLabel;
//     juce::ComboBox playModeComboBox; // 2 Play Modes: 0 = Gated, 1 = One Shot, Latch / Latch may not be a saved preset option.
//     juce::Label pMIndexLabel;
//     juce::TextEditor pMIndexTextEditor;
//     juce::Label pMIndexModLabel;
//     CvInputChannelComboBox pMIndexModComboBox;
//     juce::TextEditor pMIndexModTextEditor;
//     juce::Label pMSourceLabel;
//     juce::ComboBox pMSourceComboBox; // Channel 1 is 0, 2 is 1, etc. Left Input is 8, Right Input is 9, and PhaseCV is 10
//     juce::Label releaseLabel;
//     juce::TextEditor releaseTextEditor;
//     juce::Label releaseModLabel;
//     CvInputChannelComboBox releaseModComboBox;
//     juce::TextEditor releaseModTextEditor;
//     juce::Label reverseLabel;
//     juce::ToggleButton reverseCheckBox; // 
//     juce::Label sampleEndModLabel;
//     CvInputChannelComboBox sampleEndModComboBox;
//     juce::TextEditor sampleEndModTextEditor;
//     juce::Label xfadeGroupLabel;
//     juce::ComboBox xfadeGroupComboBox; // Off, A, B, C, D
//     juce::Label zoneRTLabel;
//     juce::ComboBox zoneRTComboBox; // 0 = Gate Rise, 1 = Continuous, 2 = Advance, 3 = Random
}

void ChannelEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::teal);
    g.drawRect (getLocalBounds ());
}
