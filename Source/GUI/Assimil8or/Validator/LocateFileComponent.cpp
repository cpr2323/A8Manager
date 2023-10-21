#include "LocateFileComponent.h"

LocateFileComponent::LocateFileComponent ()
{
    curFolderLabel.setText ("empty", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (curFolderLabel);
    addAndMakeVisible (directoryViewerComponent);
    addAndMakeVisible (missingFileComponent);

    lookHereButton.setButtonText ("Look Here");
    addAndMakeVisible (lookHereButton);
    cancelButton.setButtonText ("Cancel");
    addAndMakeVisible (cancelButton);
}

void LocateFileComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    localBounds.reduce (3, 3);
    curFolderLabel.setBounds (localBounds.removeFromTop(20));
    localBounds.removeFromTop (5);
    auto bottomRow { localBounds.removeFromBottom (20) };
    localBounds.removeFromBottom (5);
    bottomRow.removeFromLeft (5);
    lookHereButton.setBounds (bottomRow.removeFromLeft(60));
    bottomRow.removeFromLeft (5);
    cancelButton.setBounds (bottomRow.removeFromLeft (60));
    directoryViewerComponent.setBounds (localBounds.removeFromLeft ((localBounds.getWidth () / 2) - 2));
    localBounds.removeFromLeft (4);
    missingFileComponent.setBounds (localBounds);
}
