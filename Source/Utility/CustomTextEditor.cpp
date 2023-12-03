#include "CustomTextEditor.h"
#include "DebugLog.h"

void CustomTextEditor::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (! mouseEvent.mods.isPopupMenu ())
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
        DebugLog ("CustomTextEditor", "invoking popup menu");
        if (onPopupMenu != nullptr)
            onPopupMenu ();
        return;
    }
    DebugLog ("CustomTextEditor", "mouseUp");
    juce::TextEditor::mouseDown (mouseEvent);
}

void CustomTextEditor::mouseUp (const juce::MouseEvent& mouseEvent)
{
    if (mouseCaptured == true)
    {
        DebugLog ("CustomTextEditor", "releasing mouse");
        mouseCaptured = false;
    }
    else
    {
        DebugLog ("CustomTextEditor", "mouseUp");
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

    auto finalDragSpeed { 1.0 };
    const auto moveDiff { mouseEvent.getPosition ().getY () - lastY };
    const auto absDiff { std::abs (moveDiff) };
    if (absDiff > maxYMove)
        maxYMove = absDiff;
    const auto constrainedAbsDiff { std::max (std::min (absDiff, 50), 1) }; // a number between 1 and 50
     finalDragSpeed = std::min (0.1, constrainedAbsDiff / 50.0); // a number between 0.1 and 1
    if (finalDragSpeed > 0.95)
        finalDragSpeed = 1.0;
//     if (finalDragSpeed <= lastDragSpeed)
//         moveFilter.setCurValue (finalDragSpeed);
//     else
//         moveFilter.doFilter (finalDragSpeed);
//     if (finalDragSpeed < 0.0)
//         finalDragSpeed = 0.1;
    if (finalDragSpeed < 0.03)
        finalDragSpeed = 0.01;
    finalDragSpeed = finalDragSpeed * (moveDiff < 0.0 ? 1.0 : -1.0);
    lastDragSpeed = finalDragSpeed;
    DebugLog ("CustomTextEditor", "[moveDiff: " + juce::String(moveDiff) + ", maxYMove: " + juce::String (maxYMove));
    DebugLog ("CustomTextEditor", "finalDragSpeed: " + juce::String(finalDragSpeed * 100));
    if (onDrag != nullptr)
        onDrag (finalDragSpeed * 100);
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
