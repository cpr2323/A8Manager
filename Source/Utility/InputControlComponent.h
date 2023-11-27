#pragma once

#include <JuceHeader.h>

class InputControlComponent : //public juce::KeyListener,
                              public juce::MouseListener
{
public:
    void setClientComponent (juce::Component* theClientComponent);
    std::function<void (int dragSpeed)> onDrag;
    std::function<void ()> onPopupMenu;

private:
    juce::Component* clientComponent { nullptr };
    bool doingDrag { false };
    int lastY { 0 };

//     bool hitTest (int x, int y) override;
//     bool keyPressed (const juce::KeyPress& key) override;
//     bool keyStateChanged (bool isKeyDown) override;
//     void modifierKeysChanged (const juce::ModifierKeys& modifiers) override;
// 
//     void focusGained (FocusChangeType cause) override;
//     void focusGainedWithDirection (FocusChangeType cause, FocusChangeDirection direction) override;
//     void focusLost (FocusChangeType cause) override;
//     void focusOfChildComponentChanged (FocusChangeType cause) override;
// 
    void mouseMove (const juce::MouseEvent& event) override;
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& mouseEvent) override;
    void mouseDrag (const juce::MouseEvent& mouseEvent) override;
    void mouseUp (const juce::MouseEvent& mouseEvent) override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
};
