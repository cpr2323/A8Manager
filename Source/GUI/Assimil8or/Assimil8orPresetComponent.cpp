#include "Assimil8orPresetComponent.h"
#include "../../Utility/RuntimeRootProperties.h"

Assimil8orPresetComponent::Assimil8orPresetComponent ()
{
    setOpaque (true);
    addAndMakeVisible (presetTreeView);
//     auto treeViewRoot { presetTreeView.getRootItem () };
//     treeViewRoot->
}

void Assimil8orPresetComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    assimil8orData = runtimeRootProperties.getValueTree ().getChildWithName ("Assimil8or");
    jassert (assimil8orData.isValid ());
}

void Assimil8orPresetComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orPresetComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    presetTreeView.setBounds (localBounds);
}
