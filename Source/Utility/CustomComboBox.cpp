#include "CustomComboBox.h"
#include "DebugLog.h"

void CustomComboBox::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseDown (mouseEvent, onPopupMenuCallback, nullptr))
        juce::ComboBox::mouseDown (mouseEvent);
}

void CustomComboBox::mouseUp (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseUp (mouseEvent))
        juce::ComboBox::mouseUp (mouseEvent);
}

void CustomComboBox::mouseMove (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseMove (mouseEvent))
        juce::ComboBox::mouseMove (mouseEvent);
}

void CustomComboBox::mouseEnter (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseEnter (mouseEvent))
        juce::ComboBox::mouseEnter (mouseEvent);
}

void CustomComboBox::mouseExit (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseExit (mouseEvent))
        juce::ComboBox::mouseExit (mouseEvent);
}

void CustomComboBox::mouseDrag (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseDrag (mouseEvent, onDragCallback))
        juce::ComboBox::mouseDrag (mouseEvent);
}

void CustomComboBox::mouseDoubleClick (const juce::MouseEvent& mouseEvent)
{
    if (! customComponentMouseHandler.mouseDoubleClick (mouseEvent))
        juce::ComboBox::mouseDoubleClick (mouseEvent);
}

void CustomComboBox::mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel)
{
    // NOTE: there is a bug in the JUCE library regarding scroll wheel and comboboxes, which causes two events for every event.
    if (evenEvent)
    {
        if (! customComponentMouseHandler.mouseWheelMove (mouseEvent, wheel, onDragCallback))
            juce::ComboBox::mouseWheelMove (mouseEvent, wheel);
    }
    evenEvent = ! evenEvent;
}
