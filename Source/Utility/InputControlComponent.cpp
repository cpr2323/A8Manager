#include "InputControlComponent.h"
#include "DebugLog.h"


void InputControlComponent::setClientComponent (juce::Component* theClientComponent)
{
    jassert (theClientComponent != nullptr);
    clientComponent = theClientComponent;
    clientComponent->addMouseListener (this, false);
}

void InputControlComponent::mouseUp (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseUp");
    doingDrag = false;
}

void InputControlComponent::mouseDoubleClick (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseDoubleClick");
}

void InputControlComponent::mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel)
{
    DebugLog ("InputControlComponent", "mouseWheelMove");
}

void InputControlComponent::mouseMove (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseMove");
}

void InputControlComponent::mouseEnter (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseEnter");
}

void InputControlComponent::mouseExit (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseExit");
}

void InputControlComponent::mouseDown (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseDown");
    if (! mouseEvent.mods.isPopupMenu ())
    {
        if (mouseEvent.mods.isCtrlDown ())
        {
            lastY = mouseEvent.getPosition ().getY ();
            doingDrag = true;
        }
    }
    else
    {
        if (onPopupMenu != nullptr)
            onPopupMenu ();
    }
}

void InputControlComponent::mouseDrag (const juce::MouseEvent& mouseEvent)
{
    DebugLog ("InputControlComponent", "mouseDrag");
    if (doingDrag)
    {
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
        // DebugLog("InputControlComponent", juce::String (positiveDiff) + ", " + juce::String (finalDragSpeed));
        if (onDrag != nullptr)
            onDrag (finalDragSpeed);
        lastY = mouseEvent.getPosition ().getY ();
    }
}
