#pragma once

#include <JuceHeader.h>
#include "../AppProperties.h"

class ToolWindow : public juce::Component
{
public:
    ToolWindow ();
    void init (juce::ValueTree persistentRootPropertiesVT, juce::ValueTree runtimeRootPropertiesVT);

private:
    AppProperties appProperties;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton fileMenuButton;

    void loadFile ();

    void paint (juce::Graphics& g) override;
    void resized () override;
};
