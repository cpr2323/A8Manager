#pragma once

#include <JuceHeader.h>

// the fine end of the acceleration: how far the mouse travels, in pixels, to change a value by one increment when dragging slowly
const auto kPixelsPerIncrement { 4.0 };
// the coarse end of the acceleration: how far the mouse travels, in pixels, to cross an entire range when dragging as fast as possible
const auto kFastTraversalPixels { 200.0 };
// no single change may move a value by more than this fraction of its range. a change only reads as a jump relative to the range it is in,
// so this is the one measure of 'too large a change' that means the same thing for a value with sixteen steps and one with half a million
const auto kMaxRangeFractionPerChange { 0.01 };
// drag velocity, in pixels per second, that maps to the fine (0.0) and the coarse (1.0) end of the acceleration
const auto kMinDragVelocity { 150.0 };
const auto kMaxDragVelocity { 1200.0 };
// the window, in milliseconds, that drag velocity is measured across. measuring across a window, rather than between two mouse events, is
// what keeps the speed steady when the OS delivers a burst of coalesced events, or timestamps several events identically
const auto kVelocityWindow { 50.0 };
// the smallest window, in milliseconds, that a new velocity measurement is trusted from
const auto kMinVelocityWindow { 15.0 };
// a pause longer than this, in milliseconds, restarts the speed measurement, as does reversing direction
const auto kDragPauseTime { 100.0 };
// how much drag travel, in pixels, one wheel notch is worth
const auto kWheelNotchTravel { 6.0 };
// time between wheel events, in milliseconds, that maps to the coarse (0.0) and the fine (1.0) end of the acceleration
const auto kMinWheelInterval { 30.0 };
const auto kMaxWheelInterval { 150.0 };

// what the component being dragged knows about the value it is changing. the range sets how quickly the value can be moved across it, and
// the increment sets both the smallest change that is useful and the amounts the value is allowed to land on
struct DragRange
{
    double minValue { 0.0 };
    double maxValue { 0.0 };
    double increment { 1.0 };
};

// valueDelta is a signed amount to add to the value being dragged, and is always a whole number of increments
using OnDragCallback = std::function<void (double valueDelta)>;
using OnPopupMenuCallback = std::function<void ()>;
using CompleteEditCallback = std::function<void ()>;

class CustomComponentMouseHandler
{
public:
    bool mouseDown (const juce::MouseEvent& mouseEvent, OnPopupMenuCallback onPopupMenuCallback, CompleteEditCallback completeEditCallback);
    bool mouseUp (const juce::MouseEvent&);
    bool mouseMove (const juce::MouseEvent&);
    bool mouseEnter (const juce::MouseEvent&);
    bool mouseExit (const juce::MouseEvent&);
    bool mouseDrag (const juce::MouseEvent& mouseEvent, DragRange dragRange, OnDragCallback onDragCallback);
    bool mouseDoubleClick (const juce::MouseEvent&);
    bool mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&, DragRange dragRange, OnDragCallback onDragCallback);

private:
    struct DragSample
    {
        double eventTime { 0.0 };
        int mouseY { 0 };
    };

    bool mouseCaptured { false };
    int lastMouseY { 0 };
    double lastDragEventTime { 0.0 };
    double dragVelocity { 0.0 };
    std::vector<DragSample> dragSamples;
    int lastDragDirection { 0 };
    double valueRemainder { 0.0 };
    double lastWheelEventTime { 0.0 };
    int lastWheelDirection { 0 };

    void startDrag (int mouseY);
    void restartMeasurement ();
    float updateDragSpeed (int curMouseY, double curEventTime);
    double getValueDelta (DragRange dragRange, double travel, float dragSpeed);
};
