#include "PresetListComponent.h"

PresetListComponent::PresetListComponent ()
{
    presetListTitleLable.setText ("Preset List", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (presetListTitleLable);
}

void PresetListComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    presetListTitleLable.setBounds (localBounds.removeFromTop (25));
}
