#include "Assimil8orValidatorComponent.h"
#include "RenameDialogComponent.h"
#include "LocateFileComponent.h"
#include "../../../Assimil8or/Validator/ValidatorResultListProperties.h"
#include "../../../Utility/RuntimeRootProperties.h"

const auto kValidFileSystemCharacters { juce::String (" !#$%&'()+,-.0123456789;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]_`{}~abcdefghijklmnopqrstuvwxyz") };

Assimil8orValidatorComponent::Assimil8orValidatorComponent ()
{
    setOpaque (true);

    addAndMakeVisible (validatorToolWindow);

    validationResultsListBox.setClickingTogglesRowSelection (false);
    validationResultsListBox.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
    validationResultsListBox.setOutlineThickness (1);
    validationResultsListBox.getHeader ().addColumn ("Status", Columns::resultType, 60, 60, 60, juce::TableHeaderComponent::visible);
    validationResultsListBox.getHeader ().addColumn ("Fix", Columns::fix, 60, 60, 60, juce::TableHeaderComponent::visible);
    validationResultsListBox.getHeader ().addColumn ("Message", Columns::text, 100, 10, 3000, juce::TableHeaderComponent::visible);
    validationResultsListBox.getHeader ().setStretchToFitActive (true);
    addAndMakeVisible (validationResultsListBox);

    audioFormatManager.registerBasicFormats ();
    setupViewList ();
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
    if (locateDialog!= nullptr)
    {
        locateDialog->exitModalState (0);

        // we are shutting down: can't wait for the message manager
        // to eventually delete this
        delete locateDialog;
    }
}

void Assimil8orValidatorComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);

    validatorComponentProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorComponentProperties::WrapperType::owner, ValidatorComponentProperties::EnableCallbacks::yes);
    auto updateFromViewChange = [this] ()
    {
        setupViewList ();
        validatorResultsQuickLookupList.clear ();
        buildQuickLookupList ();
        validationResultsListBox.updateContent ();
        updateHeader ();
    };
    validatorComponentProperties.onViewInfoChange = [this, updateFromViewChange] (bool)
    {
        updateFromViewChange ();
    };
    validatorComponentProperties.onViewWarningChange = [this, updateFromViewChange] (bool)
    {
        updateFromViewChange ();
    };
    validatorComponentProperties.onViewErrorChange = [this, updateFromViewChange] (bool)
    {
        updateFromViewChange ();
    };
    validatorComponentProperties.onConvertAllTrigger = [this] ()
    {
        autoConvertAll ();
    };
    validatorComponentProperties.onLocateAllTrigger = [this] ()
    {
        autoLocateAll ();
    };
    validatorComponentProperties.onRenameAllTrigger = [this] ()
    {
        autoRenameAll ();
    };

    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValidatorProperties::WrapperType::client, ValidatorProperties::EnableCallbacks::yes);
    validatorProperties.onScanStatusChanged = [this] (juce::String scanStatus)
    {
        localCopyOfValidatorResultsList = validatorProperties.getValidatorResultListVT ().createCopy ();
        juce::MessageManager::callAsync ([this, scanStatus] ()
        {
            updateListFromScan (scanStatus);
        });
    };
    localCopyOfValidatorResultsList = validatorProperties.getValidatorResultListVT ().createCopy ();
    updateListFromScan ("idle");

    validatorToolWindow.init (rootPropertiesVT);
}

void Assimil8orValidatorComponent::updateListFromScan (juce::String scanStatus)
{
    validatorResultsQuickLookupList.clear ();
    if (scanStatus == "idle")
        buildQuickLookupList ();

    validationResultsListBox.updateContent ();
    updateHeader ();
    // TODO - this is a crazy work around because when I am getting the initial list, a horizontal scroll bar is appearing.
    //        the only experiment that worked was doing this
    setSize (getWidth (), getHeight () + 1);
    setSize (getWidth (), getHeight () - 1);
}

void Assimil8orValidatorComponent::setupViewList ()
{
    viewList.clearQuick ();
    if (validatorComponentProperties.getViewInfo ())
        viewList.add (ValidatorResultProperties::ResultTypeInfo);
    if (validatorComponentProperties.getViewWarning ())
        viewList.add (ValidatorResultProperties::ResultTypeWarning);
    if (validatorComponentProperties.getViewError ())
        viewList.add (ValidatorResultProperties::ResultTypeError);
}

void Assimil8orValidatorComponent::updateHeader ()
{
    validationResultsListBox.getHeader ().setColumnName (Columns::text, "Message (" + juce::String (validatorResultsQuickLookupList.size ()) + " of " +
                                                                        juce::String (totalInfoItems + totalWarningItems + totalErrorItems) + " items | " +
                                                                        "Info: " + juce::String (totalInfoItems) +
                                                                        " | Warning : "+ juce::String (totalWarningItems) +
                                                                        " | Error : "+ juce::String (totalErrorItems) +")");
    validationResultsListBox.repaint ();
}

void Assimil8orValidatorComponent::buildQuickLookupList ()
{
    totalInfoItems = 0;
    totalWarningItems = 0;
    totalErrorItems = 0;
    renameFilesCount = 0;
    renameFoldersCount = 0;
    convertCount = 0;
    missingFileCount = 0;

    if (! localCopyOfValidatorResultsList.isValid ())
        return;

    ValidatorResultListProperties validatorResultListProperties (localCopyOfValidatorResultsList,
                                                                 ValidatorResultListProperties::WrapperType::client, ValidatorResultListProperties::EnableCallbacks::no);
    // iterate over the state message list, adding each one to the quick list
    validatorResultListProperties.forEachResult ([this] (juce::ValueTree validatorResultVT)
    {
        ValidatorResultProperties validatorResultProperties (validatorResultVT, ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        totalInfoItems += (validatorResultProperties.getType () == ValidatorResultProperties::ResultTypeInfo ? 1 : 0);
        totalWarningItems += (validatorResultProperties.getType () == ValidatorResultProperties::ResultTypeWarning? 1 : 0);
        totalErrorItems += (validatorResultProperties.getType () == ValidatorResultProperties::ResultTypeError ? 1 : 0);
        validatorResultProperties.forEachFixerEntry ([this, &validatorResultVT] (juce::ValueTree fixerEntryVT)
        {
            FixerEntryProperties fixerEntryProperties (validatorResultVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);
            if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFile)
                ++renameFilesCount;
            else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFolder)
                ++renameFoldersCount;
            else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeConvert)
                ++convertCount;
            else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeFileNotFound)
                ++missingFileCount;
            else
                jassertfalse;
            return true;
        });
        if (viewList.contains (validatorResultProperties.getType ()))
            validatorResultsQuickLookupList.emplace_back (validatorResultVT);
        return true;
    });
    validatorComponentProperties.enableConvertAll (convertCount, false);
    validatorComponentProperties.enableLocateAll (missingFileCount, false);
    validatorComponentProperties.enableRenameAll (renameFilesCount > 0 || renameFoldersCount > 0, false);
}

void Assimil8orValidatorComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

juce::String Assimil8orValidatorComponent::getCellTooltip (int rowNumber, int columnId)
{
    if (columnId != Columns::text)
        return {};

    if (rowNumber >= validatorResultsQuickLookupList.size ())
        return {};

    ValidatorResultProperties validatorResultProperties (validatorResultsQuickLookupList [rowNumber],
                                                         ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
    auto getPrefix = [this, &validatorResultProperties] () -> juce::String
    {
        if (! validatorResultProperties.getValueTree ().hasProperty ("fullFileName"))
            return {};

        juce::File file (validatorResultProperties.getValueTree ().getProperty ("fullFileName").toString ());
        return file.getParentDirectory ().getFileName () + file.getSeparatorString () + file.getFileName () + "\r\n";
    };
    return getPrefix () + validatorResultProperties.getText ();
}
void Assimil8orValidatorComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    validatorToolWindow.setBounds (localBounds.removeFromTop (25));
    validationResultsListBox.setBounds (localBounds);
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

void Assimil8orValidatorComponent::handleLocatedFiles (std::vector<std::tuple <juce::File, juce::File>>& locatedFiles)
{
    std::vector<juce::File> copiedFiles;
    for (auto [sourceFile, destinationFile] : locatedFiles)
    {
        if (sourceFile.copyFileTo (destinationFile) == true)
        {
            copiedFiles.emplace_back (destinationFile);
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Copy Failed",
                "Unable to rename '" + sourceFile.getFileName () + "' to '" + destinationFile.getFileName () + "'", {}, nullptr,
                juce::ModalCallbackFunction::create ([this] (int) {}));
        }
    }
    std::vector<juce::File> newList;
    std::set_difference (filesToLocate.begin (), filesToLocate.end (), copiedFiles.begin (), copiedFiles.end (), std::back_inserter (newList));
    filesToLocate = newList;
    if (filesToLocate.size () > 0)
    {
        locateFilesInitialDirectory = dynamic_cast<LocateFileComponent*>(locateDialog->getContentComponent ())->getCurFolder();
        triggerAsyncUpdate ();
    }
}

// handleAsyncUpdate handles displaying the locate dialog, and copying any missing files it can locate, and then redisplaying the dialog if there are more to be located
void Assimil8orValidatorComponent::handleAsyncUpdate ()
{
    juce::DialogWindow::LaunchOptions options;
    auto locateComponent { std::make_unique<LocateFileComponent> (filesToLocate, locateFilesInitialDirectory, [this] (std::vector<std::tuple <juce::File, juce::File>> locatedFiles)
    {
        handleLocatedFiles (locatedFiles);
        locateDialog->exitModalState (0);
    },
    [this] ()
    {
        locateDialog->exitModalState (0);
    }) };
    options.content.setOwned (locateComponent.release ());

    juce::Rectangle<int> area (0, 0, 500, 400);

    options.content->setSize (area.getWidth (), area.getHeight ());
    options.dialogTitle = "Locate Missing Files";
    options.dialogBackgroundColour = juce::Colour (juce::Colours::grey);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = true;
    options.componentToCentreAround = this->getParentComponent ();

    locateDialog = options.launchAsync ();
}

void Assimil8orValidatorComponent::rename (juce::File file, int maxLength)
{
    // bring up a dialog showing the old name, a field for typing the new name (length constrained), and an ok/cancel button
    juce::DialogWindow::LaunchOptions options;
    auto renameContent { std::make_unique<RenameDialogContent> (file, maxLength, [this] (bool wasRenamed)
    {
        if (wasRenamed)
            directoryDataProperties.triggerStartScan (false);
    }) };
    options.content.setOwned (renameContent.release ());

    juce::Rectangle<int> area (0, 0, 380, 110);

    options.content->setSize (area.getWidth (), area.getHeight ());
    options.dialogTitle = "Rename";
    options.dialogBackgroundColour = juce::Colour (juce::Colours::lightgrey);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;
    options.componentToCentreAround = this;

    renameDialog = options.launchAsync ();
}

void Assimil8orValidatorComponent::autoRename (juce::File fileToRename, bool doRescan)
{
    auto getNewFile = [&fileToRename] (juce::String newName)
    {
        return fileToRename.getParentDirectory ().getChildFile (newName).withFileExtension (fileToRename.getFileExtension ());
    };
    const auto kMaxFileNameLength { 47 };
    const auto kMaxFileNameWithoutExtension { kMaxFileNameLength - 4 };

    // remove illegal characters
    auto fileName { fileToRename.getFileNameWithoutExtension ().retainCharacters (kValidFileSystemCharacters) };

    // if name is still too long, try the 'remove vowels' algorithm
    if (fileName.length() > kMaxFileNameWithoutExtension)
    {
        auto removeVowels = [this] (juce::String longString)
        {
            const auto lowerCaseVowels { juce::String ("aeiou") };
            const auto capitolLetter { juce::String ("ABCDEFGHIJKLMNOPQRSTUVWXYZ") };
            juce::String shortString { longString.substring (0,1) };
            for (auto stringIndex { 1 }; stringIndex < longString.length() - 2; ++stringIndex)
            {
                if (lowerCaseVowels.containsChar (longString [stringIndex]) && ! capitolLetter.containsChar (longString [stringIndex - 1]))
                    continue;
                shortString += longString [stringIndex];
            }
            shortString += longString.substring (longString.length () - 1, 1);
            return shortString;
        };
        if (const auto noVowelsFileName { removeVowels (fileName)}; noVowelsFileName.length () != 0)
            fileName = noVowelsFileName;
    }

    // if name is still too long truncate to max length
    if (fileName.length () > kMaxFileNameWithoutExtension)
        fileName = fileName.substring (0, kMaxFileNameWithoutExtension);

    // if name is still too long, or new file name already exists, truncate to max length and start appending an integer value to the name
    auto suffixValue { 1 };
    if (fileName.length () > kMaxFileNameWithoutExtension || getNewFile (fileName).exists ())
    {
        const auto prefix { fileName.substring (0, kMaxFileNameWithoutExtension) };
        while (getNewFile (fileName).exists ())
        {
            const auto suffixString { juce::String (suffixValue) };
            const auto trimAmount { juce::jmax (0, prefix.length () + suffixString.length () - kMaxFileNameWithoutExtension) };
            fileName = prefix.substring (0, prefix.length () - trimAmount) + suffixString;
            ++suffixValue;
        }
    }

    jassert (fileName != fileToRename.getFileNameWithoutExtension ());

    if (fileToRename.moveFileTo (getNewFile(fileName)) != true)
    {
        // TODO report error
    }

    if (doRescan)
        directoryDataProperties.triggerStartScan (false);
}

void Assimil8orValidatorComponent::autoRenameAll ()
{
    ValueTreeHelpers::forEachChildOfType (validatorResultsQuickLookupList [0].getParent (), ValidatorResultProperties::ValidatorResultTypeId, [this] (juce::ValueTree vrpVT)
    {
        ValidatorResultProperties validatorResultProperties (vrpVT, ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        validatorResultProperties.forEachFixerEntry ([this] (juce::ValueTree fixerEntryVT)
        {
            FixerEntryProperties fixerEntryProperties (fixerEntryVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);
            if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFile || fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFolder)
            {
                auto file { juce::File (fixerEntryProperties.getFileName ()) };
                autoRename (file, false);
            }
            return true;
        });
        return true;
    });
    directoryDataProperties.triggerStartScan (false);
}

void Assimil8orValidatorComponent::autoConvertAll ()
{
    ValueTreeHelpers::forEachChildOfType (validatorResultsQuickLookupList [0].getParent (), ValidatorResultProperties::ValidatorResultTypeId, [this] (juce::ValueTree vrpVT)
    {
        ValidatorResultProperties validatorResultProperties (vrpVT, ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        validatorResultProperties.forEachFixerEntry ([this] (juce::ValueTree fixerEntryVT)
        {
            FixerEntryProperties fixerEntryProperties (fixerEntryVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);
            if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeConvert)
            {
                auto file { juce::File (fixerEntryProperties.getFileName ()) };
                convert (file);
            }
            return true;
        });
        return true;
    });
    directoryDataProperties.triggerStartScan (false);
}

void Assimil8orValidatorComponent::autoLocateAll ()
{
    // build list of files that need to be located
    filesToLocate.clear ();
    ValueTreeHelpers::forEachChildOfType (validatorResultsQuickLookupList [0].getParent (), ValidatorResultProperties::ValidatorResultTypeId, [this] (juce::ValueTree vrpVT)
    {
        ValidatorResultProperties validatorResultProperties (vrpVT, ValidatorResultProperties::WrapperType::client, ValidatorResultProperties::EnableCallbacks::no);
        validatorResultProperties.forEachFixerEntry ([this] (juce::ValueTree fixerEntryVT)
        {
            FixerEntryProperties fixerEntryProperties (fixerEntryVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);
            if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeFileNotFound)
            {
                auto file { juce::File (fixerEntryProperties.getFileName ()) };
                filesToLocate.emplace_back (file);
            }
            return true;
        });
        return true;
    });
    // handleAsyncUpdate handles displaying the locate dialog, and copying any missing files it can locate, and then redisplaying the dialog if there are more to be located
    locateFilesInitialDirectory = directoryDataProperties.getRootFolder();
    triggerAsyncUpdate ();
}

void Assimil8orValidatorComponent::convert (juce::File file)
{
    auto errorDialog = [this] (juce::String message)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Conversion Failed", message, {}, nullptr,
                                                juce::ModalCallbackFunction::create ([this] (int) {}));
    };

    if (std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file)); reader != nullptr)
    {
        auto tempFile { juce::File::createTempFile (".wav") };
        auto tempFileStream { std::make_unique<juce::FileOutputStream> (tempFile) };
        tempFileStream->setPosition (0);
        tempFileStream->truncate ();

        auto sampleRate { reader->sampleRate };
        auto numChannels { reader->numChannels };
        auto bitsPerSample { reader->bitsPerSample };

        if (bitsPerSample < 8)
            bitsPerSample = 8;
        else if (bitsPerSample > 24) // the wave writer supports int 8/16/24
            bitsPerSample = 24;
        jassert (numChannels != 0);
        if (numChannels > 1)
            numChannels = 1;
#define ONLY_MONO_TEST 0
#if ONLY_MONO_TEST
        if (numChannels > 1)
            numChannels = 1;
#else
        if (numChannels > 2)
            numChannels = 2;
#endif
        if (reader->sampleRate > 192000)
        {
            // we need to do sample rate conversion
            jassertfalse;
        }

        juce::WavAudioFormat wavAudioFormat;
        if (std::unique_ptr<juce::AudioFormatWriter> writer { wavAudioFormat.createWriterFor (tempFileStream.get (),
                                                              sampleRate, numChannels, bitsPerSample, {}, 0) }; writer != nullptr)
        {
            // audioFormatWriter will delete the file stream when done
            tempFileStream.release ();

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
                        errorDialog ("Failure to move converted file to original file");
                        jassertfalse;
                    }
                    directoryDataProperties.triggerStartScan (false);
                }
                else
                {
                    // failure to delete original file
                    errorDialog ("Failure to delete original file");
                    jassertfalse;
                }
            }
            else
            {
                // failure to convert
                errorDialog ("Failure to write new file");
                jassertfalse;
            }
        }
        else
        {
            //failure to create writer
            errorDialog ("Failure to create writer");
            jassertfalse;
        }
    }
    else
    {
        // failure to create reader
        errorDialog ("Failure to create reader");
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
            const auto sourceFile { fc.getURLResults () [0].getLocalFile () };
            // TODO - this should probably be in a thread
            if (sourceFile.copyFileTo (file) == false)
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Copy Failed",
                                                       "Unable to rename '" + sourceFile.getFileName () + "' to '" + file.getFileName () + "'", {}, nullptr,
                                                       juce::ModalCallbackFunction::create ([this] (int) {}));
            }
            directoryDataProperties.triggerStartScan (false);
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
        
        if (validatorResultProperties.getNumFixerEntries () == 1)
        {
            // Handle one fix

            juce::ValueTree fixerEntryVT;
            // TODO - add an indexed getter, since using a for loop to get the first item seems like overkill
            validatorResultProperties.forEachFixerEntry ([this, &fixerEntryVT] (juce::ValueTree feVT)
            {
                fixerEntryVT = feVT;
                return false; // exit after the first one (since that is the one we want)
            });
            FixerEntryProperties fixerEntryProperties (fixerEntryVT, FixerEntryProperties::WrapperType::client, FixerEntryProperties::EnableCallbacks::no);

            // just do the fix
            if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFile)
            {
                juce::PopupMenu pm;
                auto file { juce::File(fixerEntryProperties.getFileName ()) }; 
                pm.addItem (file.getParentDirectory ().getFileName () + file.getSeparatorString () + file.getFileName (), false, false, {});
                pm.addItem ("Rename", true, false, [this, kMaxFileNameLength, file = fixerEntryProperties.getFileName ()] () { rename (file, kMaxFileNameLength); });
                pm.addItem ("Auto Rename", true, false, [this, kMaxFileNameLength, file = fixerEntryProperties.getFileName ()] () { autoRename (juce::File (file), true); });
                pm.showMenuAsync ({}, [this] (int) {});
            }
            else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeRenameFolder)
            {
                rename (juce::File (fixerEntryProperties.getFileName ()), kMaxFolderNameLength);
            }
            else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeConvert)
            {
                convert (juce::File (fixerEntryProperties.getFileName ()));
            }
            else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeFileNotFound)
            {
                locate (juce::File (fixerEntryProperties.getFileName ()));
            }
            else
            {
                jassertfalse;
            }
        }
        else if (validatorResultProperties.getNumFixerEntries () > 0)
        {
            // Handle multiple fixes
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
                else if (fixerEntryProperties.getType () == FixerEntryProperties::FixerTypeFileNotFound)
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
