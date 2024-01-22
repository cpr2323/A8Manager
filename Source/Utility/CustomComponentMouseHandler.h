#pragma once

#include <JuceHeader.h>

const auto kSlowThreshold { 5.0f };
const auto kMediumThreshold { 10.0f };

enum class DragSpeed { slow, medium, fast };

using OnDragCallback = std::function<void (DragSpeed, int)>;
using OnPopupMenuCallback = std::function<void ()>;

class CustomComponentMouseHandler
{
public:
    bool mouseDown (const juce::MouseEvent& mouseEvent, OnPopupMenuCallback onPopupMenuCallback);
    bool mouseUp (const juce::MouseEvent&);
    bool mouseMove (const juce::MouseEvent&);
    bool mouseEnter (const juce::MouseEvent&);
    bool mouseExit (const juce::MouseEvent&);
    bool mouseDrag (const juce::MouseEvent& mouseEvent, OnDragCallback onDragCallback);
    bool mouseDoubleClick (const juce::MouseEvent&);
    // TODO - also use for scrolling
    bool mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&);

private:
    bool mouseCaptured { false };
    int lastMouseY { 0 };
};