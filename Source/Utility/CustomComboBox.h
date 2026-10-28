#pragma once

#include <JuceHeader.h>
#include "CustomComponentMouseHandler.h"

class CustomComboBox : public juce::ComboBox
{
public:
    OnDragCallback onDragCallback;
    std::function<void ()> onPopupMenuCallback;

private:
    CustomComponentMouseHandler customComponentMouseHandler;
    bool evenEvent { true };

    // the range of a combo box is the list of items it holds, and it can only ever land on one of them
    DragRange getDragRange () const { return { 0.0, static_cast<double> (getNumItems () - 1), 1.0 }; }

    void mouseDown (const juce::MouseEvent& mouseEvent) override;
    void mouseUp (const juce::MouseEvent& mouseEvent) override;
    void mouseMove (const juce::MouseEvent& mouseEvent) override;
    void mouseEnter (const juce::MouseEvent& mouseEvent) override;
    void mouseExit (const juce::MouseEvent& mouseEvent) override;
    void mouseDrag (const juce::MouseEvent& mouseEvent) override;
    void mouseDoubleClick (const juce::MouseEvent& mouseEvent) override;
    void mouseWheelMove (const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override;
};
