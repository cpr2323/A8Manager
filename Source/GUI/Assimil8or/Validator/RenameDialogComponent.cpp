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
    newNameEditor.onTextChange = [this, oldFile] () { checkNameAvailable (oldFile); };
    addAndMakeVisible (newNameEditor);

    okButton.setButtonText ("OK");
    okButton.setEnabled (false);
    addAndMakeVisible (okButton);
    cancelButton.setButtonText ("Cancel");
    addAndMakeVisible (cancelButton);

    cancelButton.onClick = [this] () { closeDialog (false); };
    okButton.onClick = [this, oldFile] () { doRename (oldFile); };

    newNameEditor.setWantsKeyboardFocus (true);
}

void RenameDialogContent::doRename (juce::File oldFile)
{
    jassert (! newNameEditor.getText ().trim ().isEmpty ());
    auto newFile { oldFile.getParentDirectory ().getChildFile (newNameEditor.getText ().trim ()) };
    addExtensionIfNeeded (oldFile, newFile);

    // try to do rename
    if (oldFile.moveFileTo (newFile) == true)
    {
        closeDialog (true);
    }
    else
    {
        // rename failed
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Rename Failed",
                                                "Unable to rename '" + oldFile.getFileName () + "' to '" + newFile.getFileName () + "'", {}, nullptr,
                                                juce::ModalCallbackFunction::create ([this] (int) {}));
    }
}

void RenameDialogContent::addExtensionIfNeeded (juce::File oldFile, juce::File newFile)
{
    // if the filename entered is not a directory, and does not have an extension, then get the extension from the old file name
    if (! oldFile.isDirectory () && newFile.getFileExtension () == "")
        newFile = newFile.withFileExtension (oldFile.getFileExtension ());
}

void RenameDialogContent::checkNameAvailable (juce::File oldFile)
{
    const auto newFileName { newNameEditor.getText ().trim () };
    auto newFile { oldFile.getParentDirectory ().getChildFile (newFileName) };

    addExtensionIfNeeded (oldFile, newFile);
    okButton.setEnabled (newFileName.isNotEmpty () && ! newFile.exists ());
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
    newNameEditor.setBounds (newNameRow.reduced (5, 2));
}
