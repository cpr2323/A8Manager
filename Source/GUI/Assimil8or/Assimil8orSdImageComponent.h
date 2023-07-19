#pragma once

#include <JuceHeader.h>

class Assimil8orSdImageComponent : public juce::Component, public juce::ValueTree::Listener
{
public:
    Assimil8orSdImageComponent ();
    ~Assimil8orSdImageComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::ValueTree sdCardImage;
    juce::TableListBox sdImageListBox;

    void resized () override;
    void paint (juce::Graphics& g) override;
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
