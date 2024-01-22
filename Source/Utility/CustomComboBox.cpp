#include "CustomComboBox.h"
#include "DebugLog.h"

void CustomComboBox::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (!mouseEvent.mods.isPopupMenu ())
    {
        if (mouseEvent.mods.isCommandDown ())
        {
            DebugLog ("CustomComboBox", "capturing mouse");
            lastMouseY = mouseEvent.getPosition ().getY ();
            mouseCaptured = true;
            return;
        }
    }
    else
    {
        if (onPopupMenuCallback != nullptr)
            onPopupMenuCallback ();
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

    const auto distanceY { mouseEvent.getPosition ().getY () - lastMouseY };
    const auto dragSpeed = [this, distanceY] ()
    {
            if (distanceY < kSlowThreshold)
                return DragSpeed::slow;
            else if (distanceY < kMediumThreshold)
                return DragSpeed::medium;
            else
                return DragSpeed::fast;
    } ();
    const auto dragDirection { (distanceY >= 0) ? -1 : 1 };
    if (onDragCallback != nullptr)
        onDragCallback (dragSpeed, dragDirection);
    lastMouseY = mouseEvent.getPosition ().getY ();
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
