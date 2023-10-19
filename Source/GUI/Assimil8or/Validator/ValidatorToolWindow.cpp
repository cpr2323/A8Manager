#include "ValidatorToolWindow.h"

ValidatorToolWindow::ValidatorToolWindow ()
{
    auto setupFilterButton = [this] (juce::TextButton& button, juce::String text, juce::String tooltip)
    {
        button.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
        button.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green.darker (0.5f));
        button.setClickingTogglesState (true);
        button.setTooltip (tooltip);
        button.setToggleable (true);
        button.setButtonText (text);
        button.setToggleState (true, juce::NotificationType::dontSendNotification);
        button.onClick = [this] ()
            {
                setupFilterList ();
                validatorResultsQuickLookupList.clear ();
                buildQuickLookupList ();
                validationResultsListBox.updateContent ();
                updateHeader ();
            };
        addAndMakeVisible (button);
    };

    setupFilterButton (infoFilterButton, "I", "Toggles viewing of Info messages");
    setupFilterButton (warningFilterButton, "W", "Toggles viewing of Warning messages");
    setupFilterButton (errorFilterButton, "E", "Toggles viewing of Error messages");
    renameAllButton.setButtonText ("Rename All");
    renameAllButton.onClick = [this] ()
    {
        autoRenameAll ();
    };
    addAndMakeVisible (renameAllButton);
}

void ValidatorToolWindow::init (juce::ValueTree rootPropertiesVT)
{
}

void ValidatorToolWindow::paint (juce::Graphics& g)
{
    juce::TextEditor temp;
    g.fillAll (temp.findColour (juce::TextEditor::ColourIds::backgroundColourId).brighter (0.5));
}

void ValidatorToolWindow::resized ()
{
}
