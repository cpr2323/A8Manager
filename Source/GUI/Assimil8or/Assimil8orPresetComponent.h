#pragma once

#include <JuceHeader.h>

class Assimil8orPresetComponent : public juce::Component
{
public:
    Assimil8orPresetComponent ();
    ~Assimil8orPresetComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::ValueTree assimil8orData;
    juce::TreeView presetTreeView { "Assimil8orPreset" };

    void resized () override;
    void paint (juce::Graphics& g) override;
};
