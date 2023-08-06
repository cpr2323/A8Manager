#pragma once

#include <JuceHeader.h>

class PresetListComponent : public juce::Component
{
public:
    PresetListComponent ();

private:
    juce::Label presetListTitleLable;

    void resized () override;
};
