#pragma once

#include <JuceHeader.h>
#include "ValidatorComponentProperties.h"
#include "ValidatorToolWindow.h"
#include "../../../Assimil8or/Validator/ValidatorProperties.h"
#include "../../../Assimil8or/Validator/ValidatorResultProperties.h"
#include "../../../Utility/DirectoryDataProperties.h"

class Assimil8orValidatorComponent : public juce::Component,
                                     private juce::TableListBoxModel,
                                     private juce::AsyncUpdater
{
public:
    Assimil8orValidatorComponent ();
    ~Assimil8orValidatorComponent ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    enum Columns
    {
        resultType = 1,
        fix,
        text
    };
    ValidatorProperties validatorProperties;
    ValidatorComponentProperties validatorComponentProperties;
    DirectoryDataProperties directoryDataProperties;

    ValidatorToolWindow validatorToolWindow;
    juce::TableListBox validationResultsListBox { {}, this };
    std::vector<juce::ValueTree> validatorResultsQuickLookupList;
    juce::StringArray viewList { ValidatorResultProperties::ResultTypeInfo };
    SafePointer<juce::DialogWindow> renameDialog;
    SafePointer<juce::DialogWindow> locateDialog;
    juce::AudioFormatManager audioFormatManager;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ValueTree localCopyOfValidatorResultsList;
    std::vector<juce::File> filesToLocate;
    int totalInfoItems { 0 };
    int totalWarningItems { 0 };
    int totalErrorItems { 0 };
    int renameFilesCount { 0 };
    int renameFoldersCount { 0 };
    int convertCount { 0 };
    int missingFileCount { 0 };

    void autoRename (juce::File fileToRename, bool doRescan);
    void autoConvertAll ();
    void autoLocateAll ();
    void autoRenameAll ();
    void buildQuickLookupList ();
    void convert (juce::File file);
    void handleLocatedFiles (std::vector<std::tuple <juce::File, juce::File>>& locatedFiles);
    void locate (juce::File file);
    void rename (juce::File file, int maxLength);
    void setupViewList ();
    void updateHeader ();
    void updateListFromScan (juce::String scanStatus);

    void cellClicked (int rowNumber, int columnId, const juce::MouseEvent& mouseEvent) override;
    juce::String getCellTooltip (int rowNumber, int columnId) override;
    int getNumRows () override;
    void handleAsyncUpdate () override;
    void paintRowBackground (juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void paint (juce::Graphics& g) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void resized () override;
};
