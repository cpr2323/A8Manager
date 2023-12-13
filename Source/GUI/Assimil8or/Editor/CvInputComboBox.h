#pragma once

#include <JuceHeader.h>
#include "../../../Utility/NoArrowComboBoxLnF.h"

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
    void setTooltip (juce::String toolTip);
    std::function<void ()> onChange;

private:
    juce::ComboBox cvInputComboBox;
    int startingIndex { 0 };
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
