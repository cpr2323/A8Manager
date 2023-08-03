#include "Assimil8orValidatorComponent.h"
#include "RenameDialogComponent.h"
#include "../../../Assimil8or/Validator/ValidatorResultListProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

Assimil8orValidatorComponent::Assimil8orValidatorComponent ()
{
    setOpaque (true);

    scanStatusListBox.setClickingTogglesRowSelection (false);
    scanStatusListBox.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
    scanStatusListBox.setOutlineThickness (1);
    scanStatusListBox.getHeader ().addColumn ("Status", Columns::resultType, 60, 10, 60, juce::TableHeaderComponent::visible);
    scanStatusListBox.getHeader ().addColumn ("Fix", Columns::fix, 60, 10, 60, juce::TableHeaderComponent::visible);
    scanStatusListBox.getHeader ().addColumn ("Message (0 items)", Columns::text, 100, 10, 3000, juce::TableHeaderComponent::visible);
    addAndMakeVisible (scanStatusListBox);

    auto setupFilterButton = [this] (juce::TextButton& button, juce::String text)
    {
        button.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
        button.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green.darker (0.5f));
        button.setClickingTogglesState (true);
        button.setToggleable (true);
        button.setButtonText (text);
        button.setToggleState (true, juce::NotificationType::dontSendNotification);
        button.onClick = [this] ()
        {
            setupFilterList ();
            validatorResultsQuickLookupList.clear ();
            buildQuickLookupList ();
            scanStatusListBox.updateContent ();
        };
        addAndMakeVisible (button);
    };
    setupFilterButton (idleFilterButton, "I");
    setupFilterButton (warningFilterButton, "W");
    setupFilterButton (errorFilterButton, "E");

    setupFilterList ();
    validatorResultsQuickLookupList.clear ();
    buildQuickLookupList ();
    scanStatusListBox.updateContent ();
    audioFormatManager.registerBasicFormats ();
}

Assimil8orValidatorComponent::~Assimil8orValidatorComponent ()
{
    if (renameDialog != nullptr)
    {
        renameDialog->exitModalState (0);

        // we are shutting down: can't wait for the message manager
        // to eventually delete this
        delete renameDialog;
    }
}

void Assimil8orValidatorComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorProperties::WrapperType::client, ValidatorProperties::EnableCallbacks::yes);
    validatorProperties.onScanStatusChanged = [this] (juce::String scanStatus)
    {
        validatorResultsQuickLookupList.clear ();
        if (scanStatus == "idle")
            buildQuickLookupList ();

        scanStatusListBox.getHeader ().setColumnName (Columns::text, "Message (" + juce::String (validatorResultsQuickLookupList.size ()) + " items)");
        scanStatusListBox.repaint ();
    };
}

void Assimil8orValidatorComponent::setupFilterList ()
{
    filterList.clearQuick ();
    if (idleFilterButton.getToggleState ())
        filterList.add (ValidatorResultProperties::ResultTypeInfo);
    if (warningFilterButton.getToggleState ())
        filterList.add (ValidatorResultProperties::ResultTypeWarning);
    if (errorFilterButton.getToggleState ())
        filterList.add (ValidatorResultProperties::ResultTypeError);
}

void Assimil8orValidatorComponent::buildQuickLookupList ()
{
    ValidatorResultListProperties validatorResultListProperties (validatorProperties.getValidatorResultListVT (),
        ValidatorResultListProperties::WrapperType::client, ValidatorResultListProperties::EnableCallbacks::no);
    // iterate over the state message list, adding each one to the quick list
    validatorResultListProperties.forEachResult ([this] (juce::ValueTree validatorResultVT)
    {
        ValidatorResultProperties validatorResultProperties (validatorResultVT, ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        if (filterList.contains (validatorResultProperties.getType ()))
            validatorResultsQuickLookupList.emplace_back (validatorResultVT);
        return true;
    });
}

void Assimil8orValidatorComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orValidatorComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    scanStatusListBox.setBounds (localBounds);
    scanStatusListBox.getHeader ().setColumnWidth (Columns::text, scanStatusListBox.getWidth () - 2 -
                                                   scanStatusListBox.getHeader ().getColumnWidth (Columns::resultType) -
                                                   scanStatusListBox.getHeader ().getColumnWidth (Columns::fix));
    auto filterButtonBounds { getLocalBounds ().removeFromBottom (45).withTrimmedBottom (15).withTrimmedRight (15) };
    errorFilterButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    warningFilterButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    idleFilterButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
}

int Assimil8orValidatorComponent::getNumRows ()
{
    return (int) validatorResultsQuickLookupList.size ();
}

void Assimil8orValidatorComponent::paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    if (rowNumber >= validatorResultsQuickLookupList.size ())
        return;

    if (rowIsSelected)
    {
        g.fillAll (juce::Colours::lightblue);
    }
    else
    {
        auto unSelectedBackgroundColour { juce::Colours::lightgrey };
        if (rowNumber % 2)
            unSelectedBackgroundColour = unSelectedBackgroundColour.interpolatedWith (juce::Colours::black, 0.1f);
        g.fillAll (unSelectedBackgroundColour);
    }
}

void Assimil8orValidatorComponent::paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (rowNumber < validatorResultsQuickLookupList.size ())
    {
        g.setColour (juce::Colours::lightsteelblue);
        g.fillRect (width - 1, 0, 1, height);
        ValidatorResultProperties validatorResultProperties (validatorResultsQuickLookupList [rowNumber],
            ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        auto textColor { juce::Colours::black };
        if (validatorResultProperties.getType () == ValidatorResultProperties::ResultTypeWarning)
            textColor = juce::Colours::orange.darker (0.3f);
        else if (validatorResultProperties.getType () == ValidatorResultProperties::ResultTypeError)
            textColor = juce::Colours::red.darker (0.3f);

        juce::String outputText {"  "};
        switch (columnId)
        {
        case Columns::resultType:
        {
            outputText += validatorResultProperties.getType ();
        }
        break;
        case Columns::fix:
        {
            if (validatorResultProperties.getNumFixerEntries () != 0)
            {
                outputText += "Fix";
            }
        }
        break;
        case Columns::text:
        {
            outputText += validatorResultProperties.getText ();
        }
        break;
        }
        g.setColour (textColor);
        g.drawText (outputText, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
    }
}

juce::Component* Assimil8orValidatorComponent::refreshComponentForCell (int rowNumber, [[maybe_unused]] int columnId, bool rowIsSelected,
    juce::Component* existingComponentToUpdate)
{
    if (rowIsSelected)
    {
        jassert (rowNumber < validatorResultsQuickLookupList.size ());
    }
    else if (rowNumber < validatorResultsQuickLookupList.size ())
    {
        if (existingComponentToUpdate != nullptr)
            delete existingComponentToUpdate;
        return nullptr;
    }

    jassert (existingComponentToUpdate == nullptr);
    return nullptr;
}

void Assimil8orValidatorComponent::rename (juce::File file, int maxLength)
{
    // bring up a dialog showing the old name, a field for typing the new name (length constrained), and an ok/cancel button
    juce::DialogWindow::LaunchOptions options;
    auto renameContent { std::make_unique<RenameDialogContent> (file, maxLength, [this] (bool wasRenamed)
        {
            if (wasRenamed)
                validatorProperties.startAsyncScan (false);
        }) };
    options.content.setOwned (renameContent.release ());

    juce::Rectangle<int> area (0, 0, 300, 200);

    options.content->setSize (area.getWidth (), area.getHeight ());
    options.dialogTitle = "Rename";
    options.dialogBackgroundColour = juce::Colour (juce::Colours::whitesmoke);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;
    options.componentToCentreAround = this;

    renameDialog = options.launchAsync ();
}

void Assimil8orValidatorComponent::convert (juce::File file)
{
    if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file)); reader != nullptr)
    {
        auto tempFile { juce::File::createTempFile (".wav") };
        auto fileStream { std::make_unique<juce::FileOutputStream> (tempFile) };
        fileStream->setPosition (0);
        fileStream->truncate ();

        auto sampleRate { reader->sampleRate };
        auto numChannels { reader->numChannels };
        auto bitsPerSample { reader->bitsPerSample };

        if (bitsPerSample < 8)
            bitsPerSample = 8;
        else if (bitsPerSample > 32)
            bitsPerSample = 32;
        jassert (numChannels != 0);
        if (numChannels > 2)
            numChannels = 2;
        if (reader->sampleRate > 192000)
        {
            // we need to do sample rate conversion
            jassertfalse;
        }

        juce::WavAudioFormat wavAudioFormat;
        if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (fileStream.get (),
                                                              sampleRate, numChannels, bitsPerSample, {}, 0) }; writer != nullptr)
        {
            // audioFormatWriter will delete the file stream when done
            fileStream.release ();
            // copy the whole thing
            // TODO - two things
            //   a) this needs to be done in a thread
            //   b) we should locally read into a buffer and then write that, so we can display progress if needed
            if (writer->writeFromAudioReader (*reader.get (), 0, -1) == true)
            {
                // close the writer and reader, so that we can manipulate the files
                writer.reset ();
                reader.reset ();

                // TODO - should we rename the original, until we have succeeded in copying of the new file, and only then delete it
                if (file.deleteFile () == true)
                {
                    if (tempFile.moveFileTo (file) == false)
                    {
                        // failure to move temp file to new file
                        jassertfalse;
                    }
                    validatorProperties.startAsyncScan (false);
                }
                else
                {
                    // failure to delete original file
                    jassertfalse;
                }
            }
            else
            {
                // failure to copy all data
                jassertfalse;
            }
        }
        else
        {
            //failure to create writer
            jassertfalse;
        }
    }
    else
    {
        // failure to create reader
        jassertfalse;
    }
}

void Assimil8orValidatorComponent::locate (juce::File file)
{
    // bring up a file browser to locate
    fileChooser.reset (new juce::FileChooser ("Please locate the missing file...", {}, {}));
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this, file] (const juce::FileChooser& fc) mutable
    {
        if (fc.getURLResults ().size () == 1 && fc.getURLResults () [0].isLocalFile ())
        {
            // copy selected file to missing file location
            auto sourceFile { fc.getURLResults () [0].getLocalFile () };
            // TODO - this should probably be in a thread
            if (sourceFile.copyFileTo (file) == false)
            {
                // TODO - handle error
                jassertfalse;
            }
            validatorProperties.startAsyncScan (false);
        }
    }, nullptr);
}

void Assimil8orValidatorComponent::cellClicked (int rowNumber, int columnId, const juce::MouseEvent&)
{
    if (rowNumber >= validatorResultsQuickLookupList.size ())
        return;

    const auto kMaxFileNameLength { 47 };
    const auto kMaxFolderNameLength { 31 };
    if (columnId == Columns::fix)
    {
        ValidatorResultProperties validatorResultProperties (validatorResultsQuickLookupList [rowNumber],
            ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        if (validatorResultProperties.getNumFixerEntries () > 0)
        {
            if (validatorResultProperties.getNumFixerEntries () == 1)
            {
                juce::ValueTree fixerEntryVT;
                validatorResultProperties.forEachFixerEntry ([this, &fixerEntryVT] (juce::ValueTree feVT)
                {
                    fixerEntryVT = feVT;
                    return false; // exit after the first one (since there is only one)
                });
                FixerEntryProperties fixerEntryProperties (fixerEntryVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);

                // just do the fix
                if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFile)
                {
                    rename (juce::File (fixerEntryProperties.getFileName ()), kMaxFileNameLength);
                }
                else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFolder)
                {
                    rename (juce::File (fixerEntryProperties.getFileName ()), kMaxFolderNameLength);
                }
                else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeConvert)
                {
                    convert (juce::File (fixerEntryProperties.getFileName ()));
                }
                else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeNotFound)
                {
                    locate (juce::File (fixerEntryProperties.getFileName ()));
                }
                else
                {
                    jassertfalse;
                }
            }
            else
            {
                juce::PopupMenu pm;
                validatorResultProperties.forEachFixerEntry ([this, &pm, kMaxFileNameLength, kMaxFolderNameLength] (juce::ValueTree fixerEntryVT)
                {
                    FixerEntryProperties fixerEntryProperties (fixerEntryVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);
                    auto file {juce::File (fixerEntryProperties.getFileName ())};
                    if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFile)
                    {
                        pm.addItem ("Rename " + file.getFileName (), true, false, [this, kMaxFileNameLength, file] () { rename (file, kMaxFileNameLength); });
                    }
                    else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFolder)
                    {
                        pm.addItem ("Rename " + file.getFileName (), true, false, [this, kMaxFolderNameLength, file] () { rename (file, kMaxFolderNameLength); });
                    }
                    else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeConvert)
                    {
                        pm.addItem ("Convert " + file.getFileName (), true, false, [this, file] () { convert (file); });
                    }
                    else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeNotFound)
                    {
                        pm.addItem ("Find " + file.getFileName (), true, false, [this, file] () { locate (file); });
                    }
                    else
                    {
                        jassertfalse;
                    }

                    return true;
                });
                pm.showMenuAsync ({}, [this] (int) {});
            }
        }
    }
}
