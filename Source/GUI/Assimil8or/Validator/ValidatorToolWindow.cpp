#include "ValidatorToolWindow.h"
#include "oolib/Properties/RuntimeRootProperties.h"

ValidatorToolWindow::ValidatorToolWindow ()
{
    addAndMakeVisible (paneHeader);

    // a filter that is on takes the selection tint, as the pane ALL buttons do
    auto setupFilterButton = [this] (ChromeButton& button, juce::String tooltip, std::function<void ()> clickFunction)
    {
        button.setClickingTogglesState (true);
        button.setTooltip (tooltip);
        button.setToggleable (true);
        button.setToggleState (true, juce::NotificationType::dontSendNotification);
        button.onClick = clickFunction;
        addAndMakeVisible (button);
    };

    setupFilterButton (viewInfoButton, "Toggles viewing of Info messages", [this] () { validatorComponentProperties.setViewInfo (viewInfoButton.getToggleState (), false);  });
    setupFilterButton (viewWarningButton, "Toggles viewing of Warning messages", [this] () { validatorComponentProperties.setViewWarning (viewWarningButton.getToggleState (), false);  });
    setupFilterButton (viewErrorButton, "Toggles viewing of Error messages", [this] () { validatorComponentProperties.setViewError (viewErrorButton.getToggleState (), false);  });

    auto setupDoAllButton = [this] (ChromeButton& button, juce::String tooltip, std::function<void ()> onClickFunc)
    {
        button.setTooltip (tooltip);
        button.onClick = onClickFunc;
        addAndMakeVisible (button);
    };
    setupDoAllButton (convertAllButton, "Convert every file that needs it to a format the Assimil8or can use", [this] () { validatorComponentProperties.triggerConvertAll (false); } );
    setupDoAllButton (locateAllButton, "Find every missing sample", [this] () { validatorComponentProperties.triggerLocateAll (false); });
    setupDoAllButton (renameAllButton, "Rename every file and folder whose name is too long", [this] () { validatorComponentProperties.triggerRenameAll (false); });
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

void ValidatorToolWindow::paint ([[maybe_unused]] juce::Graphics& g)
{
    // the header paints the whole strip
}

void ValidatorToolWindow::resized ()
{
    paneHeader.setBounds (getLocalBounds ());

    // the pane tools live in the header strip, right aligned: the view filters, then the fix everything tools
    constexpr auto kToolGap { 6 };
    auto toolRow { paneHeader.getFreeBounds () };
    auto placeTool = [&toolRow] (ChromeButton& tool, int gapAfter)
    {
        tool.setBounds (toolRow.removeFromRight (tool.getIdealWidth ()));
        toolRow.removeFromRight (gapAfter);
    };
    placeTool (viewErrorButton, kToolGap);
    placeTool (viewWarningButton, kToolGap);
    placeTool (viewInfoButton, kToolGap * 3);
    placeTool (locateAllButton, kToolGap);
    placeTool (convertAllButton, kToolGap);
    placeTool (renameAllButton, kToolGap);
}
