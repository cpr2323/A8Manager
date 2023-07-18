#include "Assimil8orSdImageComponent.h"
#include "../../Utility/RuntimeRootProperties.h"

Assimil8orSdImageComponent::Assimil8orSdImageComponent ()
{
    setOpaque (true);
    addAndMakeVisible (sdImageListBox);
    //     auto treeViewRoot { presetTreeView.getRootItem () };
    //     treeViewRoot->
}

void Assimil8orSdImageComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    sdImageProperties = runtimeRootProperties.getValueTree ().getChildWithName ("SDCardImage");
    jassert (sdImageProperties.isValid ());
}

void Assimil8orSdImageComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orSdImageComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    sdImageListBox.setBounds (localBounds);
}
