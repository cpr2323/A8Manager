#include "CustomComponentMouseHandler.h"
#include "DebugLog.h"

#define LOG_MOUSE_DRAG_INFO 0
#if LOG_MOUSE_DRAG_INFO
#define LogMouseDragInfo(text) DebugLog ("CustomTextEditor", text);
#else
#define LogMouseDragInfo(text) ;
#endif

const bool kEventHandled { true };
const bool kEventNotHandled { false };

bool CustomComponentMouseHandler::mouseDown (const juce::MouseEvent& mouseEvent, OnPopupMenuCallback onPopupMenuCallback)
{
    if (! mouseEvent.mods.isPopupMenu ())
    {
        if (mouseEvent.mods.isCommandDown ())
        {
            LogMouseDragInfo ("capturing mouse");
            lastMouseY = mouseEvent.getPosition ().getY ();
            mouseCaptured = true;
            return kEventHandled;
        }
    }
    else
    {
        LogMouseDragInfo ("invoking popup menu");
        if (onPopupMenuCallback != nullptr)
            onPopupMenuCallback ();
        return kEventHandled;
    }

    LogMouseDragInfo ("mouseDown");
    return kEventNotHandled;
}

bool CustomComponentMouseHandler::mouseUp (const juce::MouseEvent&)
{
    if (! mouseCaptured)
    {
        LogMouseDragInfo ("mouseUp");
        return kEventNotHandled;
    }

    LogMouseDragInfo ("releasing mouse");
    mouseCaptured = false;
    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseMove (const juce::MouseEvent&)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseEnter (const juce::MouseEvent&)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseExit (const juce::MouseEvent&)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseDrag (const juce::MouseEvent& mouseEvent, OnDragCallback onDragCallback)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    const auto distanceY { mouseEvent.getPosition ().getY () - lastMouseY };
    lastMouseY = mouseEvent.getPosition ().getY ();
    //DebugLog ("CustomTextEditor::mouseDrag - ", "distanceY: " + juce::String (distanceY));
    const auto dragSpeed = [this, dragDistance = std::abs (distanceY)] ()
    {
        if (dragDistance < kSlowThreshold)
            return DragSpeed::slow;
        else if (dragDistance < kMediumThreshold)
            return DragSpeed::medium;
        else
            return DragSpeed::fast;
    } ();
    const auto dragDirection { (distanceY >= 0) ? -1 : 1 }; // 1 indicates dragging upwards (increment value), -1 indicates dragging downwards (decrement value)
    if (onDragCallback != nullptr)
        onDragCallback (dragSpeed, dragDirection);
    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseDoubleClick (const juce::MouseEvent&)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    return kEventHandled;
}

// TODO - also use for scrolling
bool CustomComponentMouseHandler::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    return kEventHandled;
}
