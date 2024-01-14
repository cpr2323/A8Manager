#pragma once

#include <JuceHeader.h>
#include "DebugLog.h"
#include "ErrorHelpers.h"
#include "SinglePoleFilter.h"

#define LOG_MOUSE_DRAG_INFO 1
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
        DebugLog ("CustomTextEditor", "minValue: " + juce::String(getMinValueCallback ()) + ", maxValue: " + juce::String (getMaxValueCallback ()));
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
    std::function<void (int dragSpeed)> onDrag;
    std::function<void ()> onPopupMenu;

private:
    bool mouseCaptured { false };
    int lastY { 0 };
    int maxYMove { 0 };
    double lastDragSpeed { 0.0 };

    SinglePoleFilter moveFilter;

    void constrainAndSet (T value)
    {
        jassert (getMinValueCallback != nullptr);
        jassert (getMaxValueCallback != nullptr);
        jassert (updateDataCallback != nullptr);
        jassert (toStringCallback != nullptr);
        const auto newValue = [this, value] ()
        {
            const auto clampedValue { std::clamp (value, getMinValueCallback (), getMaxValueCallback ())};
            if (snapValueCallback == nullptr)
                return clampedValue;
            else
                return  snapValueCallback (clampedValue);
        } ();
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
                lastY = mouseEvent.getPosition ().getY ();
                mouseCaptured = true;
                return;
            }
        }
        else
        {
            LogMouseDragInfo ("invoking popup menu");
            if (onPopupMenu != nullptr)
                onPopupMenu ();
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
        LogMouseDragInfo ("[moveDiff: " + juce::String (moveDiff) + ", maxYMove: " + juce::String (maxYMove));
        LogMouseDragInfo ("finalDragSpeed: " + juce::String (finalDragSpeed * 100));
        if (onDrag != nullptr)
            onDrag (static_cast<int> (finalDragSpeed * 100));
        lastY = mouseEvent.getPosition ().getY ();
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
        onFocusLost = [this] () { setValue (getText ().getDoubleValue ()); };
        onReturnKey = [this] () { setValue (getText ().getDoubleValue ()); };
    }
};