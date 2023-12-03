#pragma once

#include <JuceHeader.h>
#include "SinglePoleFilter.h"

class CustomTextEditor : public juce::TextEditor
{
public:
    std::function<void (int dragSpeed)> onDrag;
    std::function<void ()> onPopupMenu;

private:
    bool mouseCaptured { false };
    int lastY { 0 };
    int maxYMove { 0 };
    double lastDragSpeed { 0.0 };

    SinglePoleFilter moveFilter;

    void mouseDown (const juce::MouseEvent& mouseEvent) override;
    void mouseUp (const juce::MouseEvent& mouseEvent) override;
    void mouseMove (const juce::MouseEvent& mouseEvent) override;
    void mouseEnter (const juce::MouseEvent& mouseEvent) override;
    void mouseExit (const juce::MouseEvent& mouseEvent) override;
    void mouseDrag (const juce::MouseEvent& mouseEvent) override;
    void mouseDoubleClick (const juce::MouseEvent& mouseEvent) override;
    void mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override;
};
