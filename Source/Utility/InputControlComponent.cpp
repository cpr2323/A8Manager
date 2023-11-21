#include "InputControlComponent.h"

void InputControlComponent::mouseUp (const juce::MouseEvent& mouseEvent)
{
    doingDrag = false;
}

void InputControlComponent::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (! mouseEvent.mods.isPopupMenu ())
    {
        lastY = mouseEvent.getPosition ().getY ();
        doingDrag = true;
    }
    else
    {
        if (onPopupMenu != nullptr)
            onPopupMenu ();
    }
}

void InputControlComponent::mouseDrag (const juce::MouseEvent& mouseEvent)
{
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
        juce::Logger::outputDebugString (juce::String (positiveDiff) + ", " + juce::String (finalDragSpeed));
        if (onDrag != nullptr)
            onDrag (finalDragSpeed);
        lastY = mouseEvent.getPosition ().getY ();
    }
}

void InputControlComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);
}
