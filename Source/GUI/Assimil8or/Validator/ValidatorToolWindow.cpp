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
    
    auto setupDoAllButton = [this] (juce::TextButton& button, juce::String text, std::function<void ()> onClickFunc)
    {
        button.setButtonText (text);
        button.onClick = onClickFunc;
        addAndMakeVisible (button);
    };
    setupDoAllButton (convertAllButton, "Convert All", [this] () { validatorComponentProperties.triggerConvertAll (false); } );
    setupDoAllButton (locateAllButton, "Locate All", [this] () { validatorComponentProperties.triggerLocateAll (false); });
    setupDoAllButton (renameAllButton, "Rename All", [this] () { validatorComponentProperties.triggerRenameAll (false); });
}

void ValidatorToolWindow::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    validatorComponentProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorComponentProperties::WrapperType::client, ValidatorComponentProperties::EnableCallbacks::yes);
    validatorComponentProperties.onEnableConvertAllChange = [this] (bool enabled) { convertAllButton.setEnabled (enabled); };
    validatorComponentProperties.onEnableLocateAllChange = [this] (bool enabled) { locateAllButton.setEnabled (enabled); };
    validatorComponentProperties.onEnableRenameAllChange = [this] (bool enabled) { renameAllButton.setEnabled (enabled); };

    viewInfoButton.setToggleState (validatorComponentProperties.getViewInfo (), juce::NotificationType::dontSendNotification);
    viewWarningButton.setToggleState (validatorComponentProperties.getViewWarning (), juce::NotificationType::dontSendNotification);
    viewErrorButton.setToggleState (validatorComponentProperties.getViewError (), juce::NotificationType::dontSendNotification);
    convertAllButton.setEnabled (validatorComponentProperties.getEnabledConvertAll ());
    locateAllButton.setEnabled (validatorComponentProperties.getEnabledLocateAll ());
    renameAllButton.setEnabled (validatorComponentProperties.getEnabledRenameAll ());
}

void ValidatorToolWindow::paint (juce::Graphics& g)
{
    juce::TextEditor temp;
    g.fillAll (temp.findColour (juce::TextEditor::ColourIds::backgroundColourId).brighter (0.5));
}

void ValidatorToolWindow::resized ()
{
    auto filterButtonBounds { getLocalBounds ().reduced(5, 3) };
    viewErrorButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    viewWarningButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    viewInfoButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    locateAllButton.setBounds (filterButtonBounds.removeFromRight (70));
    filterButtonBounds.removeFromRight (5);
    convertAllButton.setBounds (filterButtonBounds.removeFromRight (70));
    filterButtonBounds.removeFromRight (5);
    renameAllButton.setBounds (filterButtonBounds.removeFromRight (70));
}
