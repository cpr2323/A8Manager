#pragma once

#include <JuceHeader.h>

class RenameDialogContent : public juce::Component
{
public:
    RenameDialogContent (juce::File oldFile, int maxNameLength, std::function<void (bool)> theDoneCallback);

private:
    juce::Label oldNameLabel;
    juce::Label newNamePromptLabel;
    juce::TextEditor newNameEditor;
    juce::TextButton okButton;
    juce::TextButton cancelButton;
    std::function<void (bool wasRenamed)> doneCallback;
    bool neverVisible { true };

    void addExtensionIfNeeded (juce::File oldFile, juce::File newFile);
    void checkNameAvailable (juce::File oldFile);
    void closeDialog (bool renamed);
    void doRename (juce::File oldFile);

    void paint (juce::Graphics& g) override;
    void resized () override;
};
