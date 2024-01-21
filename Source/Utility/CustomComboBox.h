#pragma once

#include <JuceHeader.h>
#include "DragSpeed.h"

class CustomComboBox : public juce::ComboBox
{
public:
    std::function<void (DragSpeed dragSpeed, int direction)> onDragCallback;
    std::function<void ()> onPopupMenuCallback;

private:
    bool mouseCaptured { false };
    int lastMouseY { 0 };

    void mouseDown (const juce::MouseEvent& mouseEvent) override;
    void mouseUp (const juce::MouseEvent& mouseEvent) override;
    void mouseMove (const juce::MouseEvent& mouseEvent) override;
    void mouseEnter (const juce::MouseEvent& mouseEvent) override;
    void mouseExit (const juce::MouseEvent& mouseEvent) override;
    void mouseDrag (const juce::MouseEvent& mouseEvent) override;
    void mouseDoubleClick (const juce::MouseEvent& mouseEvent) override;
    void mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override;
};
