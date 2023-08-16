#pragma once

#include <JuceHeader.h>

class CvInputComboBox : public juce::Component
{
public:
    enum ListType
    {
        includeZero,
        dontIncludeZero
    };
    CvInputComboBox (ListType listType);
    virtual ~CvInputComboBox ();

    void setSelectedItemText (juce::String cvInputString);
    juce::String getSelectedItemText ();
    std::function<void ()> onChange;

private:
    juce::ComboBox cvInputComboBox;
    int startingIndex { 0 };

    class NoArrowComboBoxLnF : public juce::LookAndFeel_V4
    {
    public:
        void drawComboBox (juce::Graphics& g, int width, int height, bool,
                           int, int, int, int, juce::ComboBox& box)
        {
            auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent> () != nullptr ? 0.0f : 3.0f;
            juce::Rectangle<int> boxBounds (0, 0, width, height);

            g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
            g.fillRoundedRectangle (boxBounds.toFloat (), cornerSize);

            g.setColour (box.findColour (juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle (boxBounds.toFloat ().reduced (0.5f, 0.5f), cornerSize, 1.0f);
        }
        void positionComboBoxText (juce::ComboBox& box, juce::Label& label)
        {
            label.setBounds (1, 1, box.getWidth (), box.getHeight () - 2);
            label.setFont (getComboBoxFont (box));
        }

    };
    NoArrowComboBoxLnF noArrowComboBoxLnF;
    void resized () override;
};

class CvInputGlobalComboBox : public CvInputComboBox
{
public:
    CvInputGlobalComboBox () : CvInputComboBox (CvInputComboBox::ListType::dontIncludeZero) {};
};

class CvInputChannelComboBox : public CvInputComboBox
{
public:
    CvInputChannelComboBox () : CvInputComboBox (CvInputComboBox::ListType::includeZero) {};
};
