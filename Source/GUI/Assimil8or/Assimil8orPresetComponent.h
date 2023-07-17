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

    void resized () override;
    void paint (juce::Graphics& g) override;
};
