#include "MainComponent.h"

#define ENABLE_EDITOR 1
const auto toolWindowHeight { 30 };

MainComponent::MainComponent (juce::ValueTree rootPropertiesVT)
{
    setSize (800, 600);
    
    assimil8orValidatorComponent.init (rootPropertiesVT);
    assimil8orPresetComponent.init (rootPropertiesVT);
#if ENABLE_EDITOR
    contentTab.addTab ("Validator", juce::Colours::darkgrey, &assimil8orValidatorComponent, false);
    contentTab.addTab ("Editor", juce::Colours::darkgrey, &assimil8orPresetComponent, false);
    addAndMakeVisible (contentTab);
#else
    addAndMakeVisible (assimil8orValidatorComponent);
#endif
    toolWindow.init (rootPropertiesVT);
    addAndMakeVisible (toolWindow);
}

void MainComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
}

void MainComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    toolWindow.setBounds (localBounds.removeFromBottom (toolWindowHeight));
#if ENABLE_EDITOR
    contentTab.setBounds (localBounds);
#else
    assimil8orValidatorComponent.setBounds (localBounds);
#endif
}
