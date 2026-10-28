#include "CustomComponentMouseHandler.h"
#include "DebugLog.h"

#define LOG_MOUSE_DRAG_INFO 0
#if LOG_MOUSE_DRAG_INFO
#define LogMouseDragInfo(text) DebugLog ("CustomComponentMouseHandler", text);
#else
#define LogMouseDragInfo(text) ;
#endif

const bool kEventHandled { true };
const bool kEventNotHandled { false };

bool CustomComponentMouseHandler::mouseDown (const juce::MouseEvent& mouseEvent, OnPopupMenuCallback onPopupMenuCallback, CompleteEditCallback completeEditCallback)
{
    if (! mouseEvent.mods.isPopupMenu ())
    {
        if (mouseEvent.mods.isCommandDown ())
        {
            LogMouseDragInfo ("capturing mouse");
            startDrag (mouseEvent.getPosition ().getY ());
            mouseCaptured = true;
            return kEventHandled;
        }
    }
    else
    {
        LogMouseDragInfo ("invoking popup menu");
        if (onPopupMenuCallback != nullptr)
        {
            if (completeEditCallback != nullptr)
                completeEditCallback ();
            onPopupMenuCallback ();
        }
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

void CustomComponentMouseHandler::startDrag (int mouseY)
{
    lastMouseY = mouseY;
    lastDragEventTime = juce::Time::getMillisecondCounterHiRes ();
    lastDragDirection = 0;
    restartMeasurement ();
}

void CustomComponentMouseHandler::restartMeasurement ()
{
    dragVelocity = 0.0;
    dragSamples.clear ();
    valueRemainder = 0.0;
}

float CustomComponentMouseHandler::updateDragSpeed (int curMouseY, double curEventTime)
{
    // velocity is measured in pixels per second, across a window of time, so that what matters is the speed of the drag, and not the way the
    // OS happened to deliver the mouse events. a single event can report a large distance over an unmeasurably short time, and on its own
    // that reads as an enormously fast drag
    dragSamples.push_back ({ curEventTime, curMouseY });
    while (dragSamples.size () > 1 && curEventTime - dragSamples.front ().eventTime > kVelocityWindow)
        dragSamples.erase (dragSamples.begin ());
    const auto oldestSample { dragSamples.front () };
    const auto sampleWindow { curEventTime - oldestSample.eventTime };
    // until enough of a window has been collected to measure across, the previous measurement is the best one available
    if (sampleWindow >= kMinVelocityWindow)
        dragVelocity = (std::abs (curMouseY - oldestSample.mouseY) * 1000.0) / sampleWindow;
    return static_cast<float> (std::clamp ((dragVelocity - kMinDragVelocity) / (kMaxDragVelocity - kMinDragVelocity), 0.0, 1.0));
}

double CustomComponentMouseHandler::getValueDelta (DragRange dragRange, double travel, float dragSpeed)
{
    const auto range { std::abs (dragRange.maxValue - dragRange.minValue) };
    // how much the value changes per pixel of travel. the fine end of the acceleration comes from the increment of the value, and the coarse
    // end from its range, which is what lets one set of numbers suit a value with a handful of steps and a value with half a million of them
    const auto fineGain { dragRange.increment / kPixelsPerIncrement };
    const auto coarseGain { std::max (fineGain, range / kFastTraversalPixels) };
    // the gain is interpolated geometrically, so that each equal increase in speed multiplies it by the same amount, which is how
    // acceleration is perceived
    const auto gain { fineGain * std::pow (coarseGain / fineGain, static_cast<double> (dragSpeed)) };

    // a fast drag has to read as the value moving quickly, rather than as the value jumping, so no single change covers more than a small
    // fraction of the range, no matter how fast the drag is
    const auto maxValueChange { std::max (dragRange.increment, range * kMaxRangeFractionPerChange) };
    const auto valueChange { std::clamp ((gain * travel) + valueRemainder, -maxValueChange, maxValueChange) };

    // the value can only land on whole increments, so whatever is left over is carried into the next event, rather than being rounded away.
    // without this, a drag slow enough to ask for less than one increment per event would either stall, or move a whole increment anyway
    const auto numIncrements { std::trunc (valueChange / dragRange.increment) };
    valueRemainder = valueChange - (numIncrements * dragRange.increment);
    return numIncrements * dragRange.increment;
}

bool CustomComponentMouseHandler::mouseDrag (const juce::MouseEvent& mouseEvent, DragRange dragRange, OnDragCallback onDragCallback)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    const auto curMouseY { mouseEvent.getPosition ().getY () };
    const auto travel { static_cast<double> (lastMouseY - curMouseY) }; // dragging upwards is a positive change in value
    lastMouseY = curMouseY;
    const auto curEventTime { juce::Time::getMillisecondCounterHiRes () };
    const auto elapsedTime { curEventTime - lastDragEventTime };
    lastDragEventTime = curEventTime;
    if (travel == 0.0)
        return kEventHandled;

    // reversing direction, or pausing, starts the measurement over, so that the drag resumes at its finest, instead of carrying on at the
    // speed it had built up before
    const auto dragDirection { (travel > 0.0) ? 1 : -1 };
    const auto measurementRestarted { dragDirection != lastDragDirection || elapsedTime > kDragPauseTime };
    if (measurementRestarted)
    {
        restartMeasurement ();
        lastDragDirection = dragDirection;
    }
    const auto dragSpeed { measurementRestarted ? 0.0f : updateDragSpeed (curMouseY, curEventTime) };
    const auto valueDelta { getValueDelta (dragRange, travel, dragSpeed) };
    LogMouseDragInfo ("travel: " + juce::String (travel) + ", speed: " + juce::String (dragSpeed) + ", delta: " + juce::String (valueDelta));
    if (valueDelta != 0.0 && onDragCallback != nullptr)
        onDragCallback (valueDelta);
    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseDoubleClick (const juce::MouseEvent&)
{
    if (! mouseCaptured)
        return kEventNotHandled;

    return kEventHandled;
}

bool CustomComponentMouseHandler::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& mwd, DragRange dragRange, OnDragCallback onDragCallback)
{
    if (std::abs (mwd.deltaY) < std::numeric_limits<float>::epsilon ())
        return kEventHandled;

    const auto wheelDirection { ((mwd.deltaY > 0) ? 1 : -1) * (mwd.isReversed ? -1 : 1) }; // 1 indicates scrolling upwards (increment value)
    const auto curEventTime { juce::Time::getMillisecondCounterHiRes () };
    const auto elapsedTime { curEventTime - lastWheelEventTime };
    lastWheelEventTime = curEventTime;
    // the closer together the wheel events are, the faster the wheel is being spun. reversing direction starts over at the finest change,
    // to match the behavior of reversing a drag
    const auto wheelSpeed = [this, elapsedTime, wheelDirection] ()
    {
        if (wheelDirection != lastWheelDirection)
        {
            restartMeasurement ();
            return 0.0f;
        }

        return static_cast<float> (std::clamp ((kMaxWheelInterval - elapsedTime) / (kMaxWheelInterval - kMinWheelInterval), 0.0, 1.0));
    } ();
    lastWheelDirection = wheelDirection;

    // one notch of the wheel is worth a set amount of drag travel, so that the wheel accelerates the same way a drag does. a notch always
    // changes the value by at least one increment, which is what makes the wheel useful for nudging
    const auto valueDelta { getValueDelta (dragRange, kWheelNotchTravel * wheelDirection, wheelSpeed) };
    if (onDragCallback != nullptr)
        onDragCallback (valueDelta != 0.0 ? valueDelta : dragRange.increment * wheelDirection);
    return kEventHandled;
}
