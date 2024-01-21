#pragma once

#include <JuceHeader.h>
#include "DebugLog.h"
#include "DragSpeed.h"
#include "ErrorHelpers.h"
#include "SinglePoleFilter.h"

#define LOG_MOUSE_DRAG_INFO 0
#if LOG_MOUSE_DRAG_INFO
#define LogMouseDragInfo(text) DebugLog ("CustomTextEditor", text);
#else
#define LogMouseDragInfo(text) ;
#endif

template <typename T>
class CustomTextEditor : public juce::TextEditor
{
public:
    CustomTextEditor ()
    {
        onTextChange = [this] () { checkValue (); };
    }
    virtual ~CustomTextEditor () = default;
    void setValue (T value)
    {
        constrainAndSet (value);
    }

    void checkValue ()
    {
        jassert (getMinValueCallback != nullptr);
        jassert (getMaxValueCallback != nullptr);
        //DebugLog ("CustomTextEditor", "minValue: " + juce::String(getMinValueCallback ()) + ", maxValue: " + juce::String (getMaxValueCallback ()));
        // check if this current value is valid, if it is then also call the extended error check
        if (ErrorHelpers::setColorIfError (*this, getMinValueCallback (), getMaxValueCallback ()) && highlightErrorCallback != nullptr)
            highlightErrorCallback ();
    }

    // required
    std::function<T ()> getMinValueCallback;
    std::function<T ()> getMaxValueCallback;
    std::function<juce::String (T)> toStringCallback;
    std::function<void (T)> updateDataCallback;
    // optional
    std::function<void ()> highlightErrorCallback;
    std::function<T (T)> snapValueCallback;
    std::function<void (DragSpeed dragSpeed, int direction)> onDragCallback;
    std::function<void ()> onPopupMenuCallback;

private:
    bool mouseCaptured { false };
    int lastMouseY { 0 };

    void constrainAndSet (T value)
    {
        jassert (getMinValueCallback != nullptr);
        jassert (getMaxValueCallback != nullptr);
        jassert (updateDataCallback != nullptr);
        jassert (toStringCallback != nullptr);
        const auto newValue = [this, value] ()
        {
            //DebugLog ("CustomTextEditor", "input value: " + juce::String (value));
            const auto clampedValue { std::clamp (value, getMinValueCallback (), getMaxValueCallback ()) };
            //DebugLog ("CustomTextEditor", "clamped value: " + juce::String(clampedValue));
            if (snapValueCallback == nullptr)
                return clampedValue;
            else
                return  snapValueCallback (clampedValue);
        } ();
        //DebugLog ("CustomTextEditor", "final value: " + juce::String (newValue));
        updateDataCallback (newValue);
        setText (toStringCallback (newValue));
    }

    void mouseDown (const juce::MouseEvent& mouseEvent) override
    {
        if (!mouseEvent.mods.isPopupMenu ())
        {
            if (mouseEvent.mods.isCommandDown ())
            {
                LogMouseDragInfo ("capturing mouse");
                lastMouseY = mouseEvent.getPosition ().getY ();
                mouseCaptured = true;
                return;
            }
        }
        else
        {
            LogMouseDragInfo ("invoking popup menu");
            if (onPopupMenuCallback != nullptr)
                onPopupMenuCallback ();
            return;
        }
        LogMouseDragInfo ("mouseUp");
        juce::TextEditor::mouseDown (mouseEvent);
    }

    void mouseUp (const juce::MouseEvent& mouseEvent) override
    {
        if (mouseCaptured == true)
        {
            LogMouseDragInfo ("releasing mouse");
            mouseCaptured = false;
        }
        else
        {
            LogMouseDragInfo ("mouseUp");
            juce::TextEditor::mouseUp (mouseEvent);
        }
    }

    void mouseMove (const juce::MouseEvent& mouseEvent) override
    {
        if (! mouseCaptured)
            juce::TextEditor::mouseMove (mouseEvent);
    }

    void mouseEnter (const juce::MouseEvent& mouseEvent) override
    {
        if (! mouseCaptured)
            juce::TextEditor::mouseEnter (mouseEvent);
    }

    void mouseExit (const juce::MouseEvent& mouseEvent) override
    {
        if (! mouseCaptured)
            juce::TextEditor::mouseExit (mouseEvent);
    }

    void mouseDrag (const juce::MouseEvent& mouseEvent) override
    {
        if (! mouseCaptured)
        {
            juce::TextEditor::mouseDrag (mouseEvent);
            return;
        }
        const auto distanceY { mouseEvent.getPosition ().getY () - lastMouseY };
        const auto dragSpeed = [this, distanceY] ()
        {
            const auto kSlowThreshold { 10.0f };
            const auto kMediumThreshold { 50.0f };

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

    void mouseDoubleClick (const juce::MouseEvent& mouseEvent) override
    {
        if (! mouseCaptured)
            juce::TextEditor::mouseDoubleClick (mouseEvent);
    }

    void mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override
    {
        if (! mouseCaptured)
            juce::TextEditor::mouseWheelMove (mouseEvent, wheel);
    }
};

class CustomTextEditorInt : public CustomTextEditor<int>
{
public:
    CustomTextEditorInt ()
    {
        onFocusLost = [this] () { setValue (getText ().getIntValue ()); };
        onReturnKey = [this] () { setValue (getText ().getIntValue ()); };
    }
};

class CustomTextEditorInt64 : public CustomTextEditor<juce::int64>
{
public:
    CustomTextEditorInt64 ()
    {
        onFocusLost = [this] () { setValue (getText ().getLargeIntValue ()); };
        onReturnKey = [this] () { setValue (getText ().getLargeIntValue ()); };
    }
};

class CustomTextEditorDouble : public CustomTextEditor<double>
{
public:
    CustomTextEditorDouble ()
    {
        onFocusLost = [this] ()
            {
                DebugLog ("CustomTextEditorDouble::onFocusLost", juce::SystemStats::getStackBacktrace());
                setValue (getText ().getDoubleValue ());
            };
        onReturnKey = [this] () { setValue (getText ().getDoubleValue ()); };
    }
};