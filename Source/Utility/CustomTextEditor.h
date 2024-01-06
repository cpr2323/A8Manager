#pragma once

#include <JuceHeader.h>
#include "SinglePoleFilter.h"

class CustomTextEditor : public juce::TextEditor
{
public:
    void setValue (double cvAmount);
    void checkValue ();

    double min { 0.0 };
    double max { 0.0 };
    std::function<void (double)> setter;
    std::function<void (double)> updateDataCallback;
    std::function<void (CustomTextEditor&)> highlightErrorCallback;
    std::function<double (double)> snapValueCallback;
    std::function<juce::String (double)> toStringCallback;
    std::function<void (int dragSpeed)> onDrag;
    std::function<void ()> onPopupMenu;

private:
    bool mouseCaptured { false };
    int lastY { 0 };
    int maxYMove { 0 };
    double lastDragSpeed { 0.0 };

    SinglePoleFilter moveFilter;

    void constrainAndSet (double value);;

    void mouseDown (const juce::MouseEvent& mouseEvent) override;
    void mouseUp (const juce::MouseEvent& mouseEvent) override;
    void mouseMove (const juce::MouseEvent& mouseEvent) override;
    void mouseEnter (const juce::MouseEvent& mouseEvent) override;
    void mouseExit (const juce::MouseEvent& mouseEvent) override;
    void mouseDrag (const juce::MouseEvent& mouseEvent) override;
    void mouseDoubleClick (const juce::MouseEvent& mouseEvent) override;
    void mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override;
};
