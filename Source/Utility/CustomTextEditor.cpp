#include "CustomTextEditor.h"
#include "DebugLog.h"

void CustomTextEditor::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (!mouseEvent.mods.isPopupMenu ())
    {
        if (mouseEvent.mods.isCommandDown ())
        {
            DebugLog ("CustomTextEditor", "capturing mouse");
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
    juce::TextEditor::mouseDown (mouseEvent);
}

void CustomTextEditor::mouseUp (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("CustomTextEditor", "mouseUp");
    if (mouseCaptured == true)
    {
        DebugLog ("CustomTextEditor", "releasing mouse");
        mouseCaptured = false;
    }
    else
    {
        juce::TextEditor::mouseUp (mouseEvent);
    }
}

void CustomTextEditor::mouseMove (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::TextEditor::mouseMove (mouseEvent);
}

void CustomTextEditor::mouseEnter (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::TextEditor::mouseEnter (mouseEvent);
}

void CustomTextEditor::mouseExit (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::TextEditor::mouseExit (mouseEvent);
}

void CustomTextEditor::mouseDrag (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
    {
        juce::TextEditor::mouseDrag (mouseEvent);
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
    DebugLog ("CustomTextEditor", juce::String (positiveDiff) + ", " + juce::String (finalDragSpeed));
    if (onDrag != nullptr)
        onDrag (finalDragSpeed);
    lastY = mouseEvent.getPosition ().getY ();
}

void CustomTextEditor::mouseDoubleClick (const juce::MouseEvent& mouseEvent)
{
    if (!mouseCaptured)
        juce::TextEditor::mouseDoubleClick (mouseEvent);
}

void CustomTextEditor::mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel)
{
    if (!mouseCaptured)
        juce::TextEditor::mouseWheelMove (mouseEvent, wheel);
}
