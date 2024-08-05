#pragma once

#include <JuceHeader.h>

class CustomTextButton : public juce::TextButton
{
public:
    std::function<void ()> onPopupMenuCallback;

private:
    void mouseDown (const juce::MouseEvent& mouseEvent) override;
};
