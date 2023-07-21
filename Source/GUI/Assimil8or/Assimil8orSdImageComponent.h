#pragma once

#include <JuceHeader.h>
#include "../../Assimil8or/A8SDCardValidatorProperties.h"

class Assimil8orSdImageComponent : public juce::Component,
                                   private juce::TableListBoxModel
{
public:
    Assimil8orSdImageComponent ();
    ~Assimil8orSdImageComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    A8SDCardValidatorProperties a8SDCardValidatorProperties;
    juce::TableListBox sdImageListBox { {}, this };
    std::vector<juce::ValueTree> quickLookupList;

    void buildQuickLookupList ();

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    void paintRowBackground (juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
};
