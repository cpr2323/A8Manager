#pragma once

#include <JuceHeader.h>
#include "../../../Utility/CustomComboBox.h"
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

    int getNumItems ();
    int getSelectedItemIndex ();
    void setSelectedItemIndex (int itemIndex);
    void setSelectedItemText (juce::String cvInputString);
    juce::String getSelectedItemText ();
    void setTooltip (juce::String toolTip);
    std::function<void ()> onChange;
    std::function<void (int dragSpeed)> onDragCallback;
    std::function<void ()> onPopupMenuCallback;

private:
    CustomComboBox cvInputComboBox;
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
