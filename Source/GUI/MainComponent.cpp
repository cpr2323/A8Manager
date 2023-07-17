#include "MainComponent.h"

const auto toolWindowHeight { 30 };

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (800, 600);

    assimil8orPresetComponent.init (rootPropertiesVT);
    addAndMakeVisible (assimil8orPresetComponent);
    toolWindow.init (rootPropertiesVT);
    addAndMakeVisible (toolWindow);
}

MainComponent::~MainComponent ()
{
}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    toolWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
    assimil8orPresetComponent.setBounds (localBounds);
}
