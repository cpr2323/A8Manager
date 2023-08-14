#include "CvInputComboBox.h"

CvInputComboBox::CvInputComboBox (ListType listType)
{
    jassert (listType == ListType::includeZero || listType == ListType::dontIncludeZero);
    if (listType == ListType::includeZero)
        startingIndex = 0;
    else
        startingIndex = 1;

    addAndMakeVisible (cvInputComboBox);
    {
        cvInputComboBox.addItem ("Off", 1);
        for (auto channelIndex { startingIndex }; channelIndex < 9; ++channelIndex)
            for (auto columnIndex { 0 }; columnIndex < 3; ++columnIndex)
            {
                const auto menuId { 2 + (channelIndex * 3) + columnIndex };
                cvInputComboBox.addItem (juce::String::charToString ('0' + channelIndex) + juce::String::charToString ('A' + columnIndex), menuId);
            }
    }
    cvInputComboBox.onChange = [this] ()
    {
        if (onChange != nullptr)
            onChange ();
    };
}

void CvInputComboBox::setSelectedItemText (juce::String cvInputString)
{
    auto itemId { 1 };
    if (cvInputString.isEmpty ())
    {
        cvInputComboBox.setText ("--", juce::NotificationType::sendNotification);
        return;
    }
    if (cvInputString.toLowerCase () != "off")
    {
        jassert (juce::String ("012345678").containsChar (cvInputString [0]) && juce::String ("ABC").containsChar (cvInputString [1]));
        itemId = 2 + ((cvInputString [0] - '0') * 3) + cvInputString [1] - 'A';
        jassert (itemId > 1 && itemId < 29);
    }
    cvInputComboBox.setSelectedId (itemId, juce::NotificationType::dontSendNotification);
}

juce::String CvInputComboBox::getSelectedItemText ()
{
    return cvInputComboBox.getItemText (cvInputComboBox.getSelectedItemIndex ());
}

void CvInputComboBox::resized ()
{
    cvInputComboBox.setBounds (getLocalBounds ());
}
