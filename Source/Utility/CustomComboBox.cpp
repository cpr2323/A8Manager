#include "CustomComboBox.h"
#include "DebugLog.h"

void CustomComboBox::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (!mouseEvent.mods.isPopupMenu ())
    {
        if (mouseEvent.mods.isCtrlDown ())
        {
            DebugLog ("CustomComboBox", "capturing mouse");
            lastY = mouseEvent.getPosition ().getY ();
            mouseCaptured = true;
            return;
        }
    }
    else
    {
        if (onPopupMenu != nullptr)
            onPopupMenu ();
        return;
    }
    juce::ComboBox::mouseDown (mouseEvent);
}

void CustomComboBox::mouseUp (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("CustomComboBox", "mouseUp");
    if (mouseCaptured == true)
    {
        DebugLog ("CustomComboBox", "releasing mouse");
        mouseCaptured = false;
    }
    else
    {
        juce::ComboBox::mouseUp (mouseEvent);
    }
}

void CustomComboBox::mouseMove (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::ComboBox::mouseMove (mouseEvent);
}

void CustomComboBox::mouseEnter (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::ComboBox::mouseEnter (mouseEvent);
}

void CustomComboBox::mouseExit (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::ComboBox::mouseExit (mouseEvent);
}

void CustomComboBox::mouseDrag (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
    {
        juce::ComboBox::mouseDrag (mouseEvent);
        return;
    }

    auto yDiff { (mouseEvent.getPosition ().getY () - lastY) * -1 };
    const auto signage { yDiff >= 0 ? 1 : -1 };
    auto positiveDiff { std::min (std::abs (yDiff), 20) };
    auto dragSpeed { 0 };
    if (positiveDiff < 2)
        dragSpeed = 1;
    else if (positiveDiff < 4)
        dragSpeed = 2;
    else if (positiveDiff < 6)
        dragSpeed = 5;
    else
        dragSpeed = 10;
    const auto finalDragSpeed { dragSpeed * signage };
    DebugLog ("CustomComboBox", juce::String (positiveDiff) + ", " + juce::String (finalDragSpeed));
    if (onDrag != nullptr)
        onDrag (finalDragSpeed);
    lastY = mouseEvent.getPosition ().getY ();
}

void CustomComboBox::mouseDoubleClick (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::ComboBox::mouseDoubleClick (mouseEvent);
}

void CustomComboBox::mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel)
{
    if (!mouseCaptured)
        juce::ComboBox::mouseWheelMove (mouseEvent, wheel);
}
