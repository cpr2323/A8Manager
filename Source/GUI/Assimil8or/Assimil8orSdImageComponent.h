#pragma once

#include <JuceHeader.h>

class Assimil8orSdImageComponent : public juce::Component
{
public:
    Assimil8orSdImageComponent ();
    ~Assimil8orSdImageComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::ValueTree sdImageProperties;
    juce::TableListBox sdImageListBox;

    void resized () override;
    void paint (juce::Graphics& g) override;
};
