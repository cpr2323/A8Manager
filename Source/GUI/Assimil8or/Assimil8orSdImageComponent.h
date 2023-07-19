#pragma once

#include <JuceHeader.h>

class Assimil8orSdImageComponent : public juce::Component,
                                   private juce::ValueTree::Listener,
                                   private juce::TableListBoxModel
{
public:
    Assimil8orSdImageComponent ();
    ~Assimil8orSdImageComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    juce::ValueTree sdCardImage;
    juce::TableListBox sdImageListBox { {}, this };
    juce::ValueTree validationStatusProperties;
    std::vector<juce::ValueTree> quickLookupList;

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    void paintRowBackground (juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
