#pragma once

#include <JuceHeader.h>
#include "../../Assimil8or/ValidatorProperties.h"

class Assimil8orSdCardComponent : public juce::Component,
                                   private juce::TableListBoxModel
{
public:
    Assimil8orSdCardComponent ();
    ~Assimil8orSdCardComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    ValidatorProperties validatorProperties;
    juce::TableListBox scanStatusListBox { {}, this };
    std::vector<juce::ValueTree> scanStatusQuickLookupList;

    void buildQuickLookupList ();

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    void paintRowBackground (juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
};
