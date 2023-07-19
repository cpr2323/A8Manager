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
    sdCardImage = runtimeRootProperties.getValueTree ().getChildWithName ("SDCardImage");
    jassert (sdCardImage.isValid ());
    sdCardImage.addListener (this);
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
void Assimil8orSdImageComponent::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == sdCardImage)
    {
        if (property.toString () == "scanStatus")
        {
            juce::Logger::outputDebugString ("tool window received scanning update - " + sdCardImage.getProperty ("scanStatus").toString ());
            if (sdCardImage.getProperty ("scanStatus").toString () == "idle")
            {
                int xc = 5;
            }
        }
    }
}
