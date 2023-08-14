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

    void setSelectedItemText (juce::String cvInputString);
    juce::String getSelectedItemText ();
    std::function<void ()> onChange;

private:
    juce::ComboBox cvInputComboBox;
    int startingIndex { 0 };

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
