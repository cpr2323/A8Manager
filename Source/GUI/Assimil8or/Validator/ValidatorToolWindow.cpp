#include "ValidatorToolWindow.h"
#include "../../../Utility/RuntimeRootProperties.h"

ValidatorToolWindow::ValidatorToolWindow ()
{
    auto setupFilterButton = [this] (juce::TextButton& button, juce::String text, juce::String tooltip, std::function<void()> clickFunction)
    {
        button.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
        button.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green.darker (0.5f));
        button.setClickingTogglesState (true);
        button.setTooltip (tooltip);
        button.setToggleable (true);
        button.setButtonText (text);
        button.setToggleState (true, juce::NotificationType::dontSendNotification);
        button.onClick = clickFunction;
        addAndMakeVisible (button);
    };

    setupFilterButton (viewInfoButton, "I", "Toggles viewing of Info messages", [this] () { validatorComponentProperties.setViewInfo (viewInfoButton.getToggleState (), false);  });
    setupFilterButton (viewWarningButton, "W", "Toggles viewing of Warning messages", [this] () { validatorComponentProperties.setViewWarning (viewWarningButton.getToggleState (), false);  });
    setupFilterButton (viewErrorButton, "E", "Toggles viewing of Error messages", [this] () { validatorComponentProperties.setViewError (viewErrorButton.getToggleState (), false);  });
    renameAllButton.setButtonText ("Rename All");
    renameAllButton.onClick = [this] () { validatorComponentProperties.triggerRenameAll (false); };
    addAndMakeVisible (renameAllButton);
}

void ValidatorToolWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    validatorComponentProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorComponentProperties::WrapperType::client, ValidatorComponentProperties::EnableCallbacks::no);

    viewInfoButton.setToggleState (validatorComponentProperties.getViewInfo (), juce::NotificationType::dontSendNotification);
    viewWarningButton.setToggleState (validatorComponentProperties.getViewWarning (), juce::NotificationType::dontSendNotification);
    viewErrorButton.setToggleState (validatorComponentProperties.getViewError (), juce::NotificationType::dontSendNotification);
}

void ValidatorToolWindow::paint (juce::Graphics& g)
{
    juce::TextEditor temp;
    g.fillAll (temp.findColour (juce::TextEditor::ColourIds::backgroundColourId).brighter (0.5));
}

void ValidatorToolWindow::resized ()
{
    auto filterButtonBounds { getLocalBounds () };
    viewErrorButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    viewWarningButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    viewInfoButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    renameAllButton.setBounds (viewInfoButton.getX () - 70 - 5, viewInfoButton.getY (), 70, 20);
}
