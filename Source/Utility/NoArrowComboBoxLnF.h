#pragma once

#include <JuceHeader.h>

class NoArrowComboBoxLnF : public juce::LookAndFeel_V4
{
public:
    void drawComboBox (juce::Graphics& g, int width, int height, bool,
        int, int, int, int, juce::ComboBox& box)
    {
        auto cornerSize { box.findParentComponentOfClass<juce::ChoicePropertyComponent> () != nullptr ? 0.0f : 3.0f };
        juce::Rectangle<int> boxBounds (0, 0, width, height);

        g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (boxBounds.toFloat (), cornerSize);

        g.setColour (box.findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (boxBounds.toFloat ().reduced (0.5f, 0.5f), cornerSize, 1.0f);
    }
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label)
    {
        label.setBounds (0, 0, box.getWidth (), box.getHeight ());
        label.setFont (getComboBoxFont (box));
    }
};
