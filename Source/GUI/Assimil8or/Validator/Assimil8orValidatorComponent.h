#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Validator/ValidatorProperties.h"
#include "../../../Assimil8or/Validator/ValidatorResultProperties.h"

class Assimil8orValidatorComponent : public juce::Component,
                                     private juce::TableListBoxModel
{
public:
    Assimil8orValidatorComponent ();
    ~Assimil8orValidatorComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    enum Columns
    {
        resultType = 1,
        fix,
        text
    };
    ValidatorProperties validatorProperties;
    juce::TableListBox scanStatusListBox { {}, this };
    std::vector<juce::ValueTree> validatorResultsQuickLookupList;
    juce::StringArray filterList { ValidatorResultProperties::ResultTypeInfo };
    juce::TextButton idleFilterButton;
    juce::TextButton warningFilterButton;
    juce::TextButton errorFilterButton;
    SafePointer<juce::DialogWindow> renameDialog;
    juce::AudioFormatManager audioFormatManager;
    std::unique_ptr<juce::FileChooser> fileChooser;
    int totalItems { 0 };

    void buildQuickLookupList ();
    void convert (juce::File file);
    void locate (juce::File file);
    void rename (juce::File file, int maxLength);
    void setupFilterList ();
    void udpateHeader ();

    void resized () override;
    void paint (juce::Graphics& g) override;
    int getNumRows () override;
    void paintRowBackground (juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void cellClicked (int rowNumber, int columnId, const juce::MouseEvent& mouseEvent) override;
};
