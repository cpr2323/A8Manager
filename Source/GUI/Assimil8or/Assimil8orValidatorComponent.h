#pragma once

#include <JuceHeader.h>
#include "../../Assimil8or/Validator/ValidatorProperties.h"

class Assimil8orValidatorComponent : public juce::Component,
                                  private juce::TableListBoxModel
{
public:
    Assimil8orValidatorComponent ();
    ~Assimil8orValidatorComponent () = default;

    void init (juce::ValueTree rootPropertiesVT);

private:
    ValidatorProperties validatorProperties;
    juce::TableListBox scanStatusListBox { {}, this };
    std::vector<juce::ValueTree> scanStatusQuickLookupList;
    juce::StringArray filterList {"idle"};
    juce::TextButton idleFilterButton;
    juce::TextButton warningFilterButton;
    juce::TextButton errorFilterButton;

    void buildQuickLookupList ();
    void setupFilterList ();

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    void paintRowBackground (juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void cellDoubleClicked (int rowNumber, int columnId, const juce::MouseEvent& mouseEvent) override;
};
