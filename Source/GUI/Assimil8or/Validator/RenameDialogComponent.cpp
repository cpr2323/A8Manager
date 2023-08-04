#include "RenameDialogComponent.h"

RenameDialogContent::RenameDialogContent (juce::File oldFile, int maxNameLength, std::function<void (bool)> theDoneCallback)
{
    doneCallback = theDoneCallback;

    oldNameLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    oldNameLabel.setText ("Current Name: " + oldFile.getFileName (), juce::NotificationType::dontSendNotification);
    addAndMakeVisible (oldNameLabel);
    newNamePromptLabel.setColour (juce::Label::ColourIds::textColourId, juce::Colours::black);
    newNamePromptLabel.setText ("New Name:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible (newNamePromptLabel);

    newNameEditor.setIndents (2, 0);
    newNameEditor.setInputRestrictions (maxNameLength, {});
    newNameEditor.setJustification (juce::Justification::centredLeft);
    newNameEditor.onReturnKey = [this, oldFile] () { doRename (oldFile); };
    addAndMakeVisible (newNameEditor);

    okButton.setButtonText ("OK");
    addAndMakeVisible (okButton);
    cancelButton.setButtonText ("Cancel");
    addAndMakeVisible (cancelButton);

    cancelButton.onClick = [this] () { closeDialog (false); };
    okButton.onClick = [this, oldFile] () { doRename (oldFile); };

    newNameEditor.setWantsKeyboardFocus (true);
}

void RenameDialogContent::doRename (juce::File oldFile)
{
    if (newNameEditor.getText ().trim ().isEmpty ())
        return;

    auto newFile { oldFile.getParentDirectory ().getChildFile (newNameEditor.getText ().trim ()) };
    if (! oldFile.isDirectory () && newFile.getFileExtension () == "")
        newFile = newFile.withFileExtension (oldFile.getFileExtension ());

    // try to do rename
    if (oldFile.moveFileTo (newFile) == true)
    {
        closeDialog (true);
    }
    else
    {
        // rename failed
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Rename Failed",
                                                "Unable to rename '" + oldFile.getFileName() + "' to '" + newFile.getFileName () + "'", {}, nullptr,
                                                juce::ModalCallbackFunction::create ([this] (int) {}));
    }
}

void RenameDialogContent::closeDialog (bool renamed)
{
    if (doneCallback != nullptr)
        doneCallback (renamed);
    if (juce::DialogWindow * dw { findParentComponentOfClass<juce::DialogWindow> () })
        dw->exitModalState (0);
    delete this;
}

void RenameDialogContent::paint (juce::Graphics& g)
{
    if (isVisible () && neverVisible)
    {
        neverVisible = true;
        newNameEditor.grabKeyboardFocus ();
    }
    g.fillAll (juce::Colours::lightgrey);
}

void RenameDialogContent::resized ()
{
    auto localBounds { getLocalBounds () };
    auto bottomRow { localBounds.removeFromBottom (28).withTrimmedBottom (5) };
    okButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
    okButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    okButton.setBounds (bottomRow.removeFromLeft (65).withTrimmedLeft (5));
    cancelButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
    cancelButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    cancelButton.setBounds (bottomRow.removeFromLeft (65).withTrimmedLeft (5));

    oldNameLabel.setBounds (localBounds.removeFromTop (35).withTrimmedTop (5));

    //localBounds.removeFromTop (5);
    auto newNameRow { localBounds.removeFromTop (35).withTrimmedTop (5) };
    newNamePromptLabel.setBounds (newNameRow.removeFromLeft (80).withTrimmedLeft (5));
    newNameEditor.setBounds (newNameRow.reduced(5,2));
}
