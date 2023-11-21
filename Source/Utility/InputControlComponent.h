#pragma once

#include <JuceHeader.h>

class InputControlComponent : public juce::Component
{
public:
    std::function<void (int dragSpeed)> onDrag;
    std::function<void ()> onPopupMenu;

private:
    bool doingDrag { false };
    int lastY { 0 };

    void mouseUp (const juce::MouseEvent& mouseEvent) override;
    void mouseDown (const juce::MouseEvent& mouseEvent) override;
    void mouseDrag (const juce::MouseEvent& mouseEvent) override;
    void paint (juce::Graphics& g) override;
};
