#pragma once

#include <JuceHeader.h>
#include "DebugLog.h"
#include "CustomComponentMouseHandler.h"
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
        setSelectAllWhenFocused (true);
        onTextChange = [this] () { checkValue (); };
        textColor = juce::Colours::white;
        applyColourToAllText (textColor, true);

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
        //DebugLog ("CustomTextEditor", "minValue: " + juce::String (getMinValueCallback ()) + ", maxValue: " + juce::String (getMaxValueCallback ()));
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
    OnDragCallback onDragCallback;
    OnPopupMenuCallback onPopupMenuCallback;

private:
    CustomComponentMouseHandler customComponentMouseHandler;
    juce::Colour textColor;

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
            //DebugLog ("CustomTextEditor", "clamped value: " + juce::String (clampedValue));
            // TODO - I think this should be handled by the toStringCallback AND in the Preset Writer
            if (snapValueCallback == nullptr)
                return clampedValue;
            else
                return  snapValueCallback (clampedValue);
        } ();
        //DebugLog ("CustomTextEditor", "final value: " + juce::String (newValue));
        updateDataCallback (newValue);
        setText (toStringCallback (newValue));
    }

    void focusLost (FocusChangeType)
    {
        setHighlightedRegion ({});
    }

    void enablementChanged () override
    {
        auto getNewTextColour = [this] ()
        {
            if (isEnabled ())
                return textColor;
            else
                return textColor.withAlpha (0.5f);
        };
        applyColourToAllText (getNewTextColour (), true);
        juce::TextEditor::enablementChanged ();
    }

    void mouseDown (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseDown (mouseEvent, onPopupMenuCallback, [this] () { jassert (onFocusLost != nullptr); onFocusLost (); }))
            juce::TextEditor::mouseDown (mouseEvent);
    }

    void mouseUp (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseUp (mouseEvent))
            juce::TextEditor::mouseUp (mouseEvent);
    }

    void mouseMove (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseMove (mouseEvent))
            juce::TextEditor::mouseMove (mouseEvent);
    }

    void mouseEnter (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseEnter (mouseEvent))
            juce::TextEditor::mouseEnter (mouseEvent);
    }

    void mouseExit (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseExit (mouseEvent))
            juce::TextEditor::mouseExit (mouseEvent);
    }

    void mouseDrag (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseDrag (mouseEvent, onDragCallback))
            juce::TextEditor::mouseDrag (mouseEvent);
    }

    void mouseDoubleClick (const juce::MouseEvent& mouseEvent) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseDoubleClick (mouseEvent))
            juce::TextEditor::mouseDoubleClick (mouseEvent);
    }

    void mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override
    {
        if (! isEnabled ())
            return;
        if (! customComponentMouseHandler.mouseWheelMove (mouseEvent, wheel, onDragCallback))
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