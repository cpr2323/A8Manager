#include "RenameDialogComponent.h"

RenameDialogContent::RenameDialogContent (juce::File oldFile, int maxNameLength)
{
    oldNameLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    oldNameLabel.setText ("Current Name: " + oldFile.getFileName (), juce::NotificationType::dontSendNotification);
    addAndMakeVisible (oldNameLabel);
    newNamePromptLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    newNamePromptLabel.setText ("New Name:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (newNamePromptLabel);

    newNameEditor.setIndents (0, 0);
    newNameEditor.setInputRestrictions (maxNameLength, {});
    newNameEditor.onReturnKey = [this, oldFile] () { doRename (oldFile); };

    addAndMakeVisible (newNameEditor);
    okButton.setButtonText ("OK");
    addAndMakeVisible (okButton);
    cancelButton.setButtonText ("Cancel");
    addAndMakeVisible (cancelButton);

    cancelButton.onClick = [this] () { closeDialog (); };
    okButton.onClick = [this, oldFile] () { doRename (oldFile); };
}

void RenameDialogContent::doRename (juce::File oldFile)
{
    auto newFile { oldFile.getParentDirectory ().getChildFile (newNameEditor.getText ()) };
    if (!oldFile.isDirectory () && newFile.getFileExtension () == "")
        newFile = newFile.withFileExtension (oldFile.getFileExtension ());

    // try to do rename
    if (oldFile.moveFileTo (newFile) == true)
    {
        closeDialog ();
    }
    else
    {
        // rename failed
        jassertfalse;
    }
}

void RenameDialogContent::closeDialog ()
{
    if (juce::DialogWindow* dw = findParentComponentOfClass<juce::DialogWindow> ())
        dw->exitModalState (0);
    delete this;
}

void RenameDialogContent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::lightgrey);
}

void RenameDialogContent::resized ()
{
    auto localBounds { getLocalBounds () };
    auto bottomRow { localBounds.removeFromBottom (35).withTrimmedBottom (5) };
    okButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
    okButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    okButton.setBounds (bottomRow.removeFromLeft (65).withTrimmedLeft (5));
    cancelButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
    cancelButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    cancelButton.setBounds (bottomRow.removeFromLeft (65).withTrimmedLeft (5));

    oldNameLabel.setBounds (localBounds.removeFromTop (35).withTrimmedTop (5));
    localBounds.removeFromTop (5);
    auto newNameRow { localBounds.removeFromTop (35).withTrimmedTop (5) };
    newNamePromptLabel.setBounds (newNameRow.removeFromLeft (80).withTrimmedLeft (5));
    newNameEditor.setBounds (newNameRow.withTrimmedLeft (5));
}
