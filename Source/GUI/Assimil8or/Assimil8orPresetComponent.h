#pragma once

#include <JuceHeader.h>
#include "../../Assimil8or/Preset/PresetProperties.h"

class Assimil8orPresetComponent : public juce::Component
{
public:
    Assimil8orPresetComponent ();
    ~Assimil8orPresetComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    PresetProperties presetProperties;
    juce::TextEditor nameEditor;

    void resized () override;
    void paint (juce::Graphics& g) override;
    void refreshName (juce::String name);
    void updateName (juce::String name);
};
