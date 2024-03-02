#pragma once

#include <JuceHeader.h>
#include "EditManager.h"
#include "LoopPoints/LoopPointsView.h"
#include "SampleManager/SampleProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/FileSelectLabel.h"

class ZoneEditor : public juce::Component,
                   public juce::FileDragAndDropTarget
{
public:
    ZoneEditor ();
    ~ZoneEditor () = default;

    void init (juce::ValueTree zonePropertiesVT, juce::ValueTree uneditedZonePropertiesVT, juce::ValueTree rootPropertiesVT, EditManager* theEditManager);
    // TODO - can we move this to the EditManager, as it just calls editManager->assignSamples (parentChannelIndex, startingZoneIndex, files); in the ZoneEditor
    void receiveSampleLoadRequest (juce::File sampleFile);
    // TODO - is there a VTW that could manage this setting?
    void setLoopLengthIsEnd (bool loopLengthIsEnd);

    // TODO - can we make this local, since we should be able to access the edits through the EditManager
    std::function<void (int zoneIndex)> displayToolsMenu;

private:
    AppProperties appProperties;
    AudioPlayerProperties audioPlayerProperties;
    ZoneProperties zoneProperties;
    ZoneProperties uneditedZoneProperties;
    ZoneProperties minZoneProperties;
    ZoneProperties maxZoneProperties;
    SampleProperties sampleProperties;
    // TODO - I want to remove ChannelProperties!
    ChannelProperties parentChannelProperties;
    // TODO - I think we might be able to get rid of currentSampleFileName too, but I am not sure yet
    juce::String currentSampleFileName;
    EditManager* editManager { nullptr };
    int zoneIndex { -1 };
    int parentChannelIndex { -1 };

    // Loop Length is always stored as loop length, but the UI can be toggled to display it, and take input for it, as if it is Loop End
    bool treatLoopLengthAsEndInUi { false };

    bool draggingFiles { false };
    int dropIndex { 0 };

    LoopPointsView loopPointsView;
    juce::Label sourceLabel;
    juce::TextButton sourceSamplePointsButton;
    juce::TextButton sourceLoopPointsButton;
    juce::Label playModeLabel;
    juce::TextButton oneShotPlayButton;
    juce::TextButton loopPlayButton;
    juce::TextButton toolsButton;
    juce::Rectangle<int> samplePointsBackground;
    juce::Rectangle<int> loopPointsBackground;
    juce::Rectangle<int>* activePointBackground { &samplePointsBackground };

    juce::Label levelOffsetLabel;
    CustomTextEditorDouble levelOffsetTextEditor; // double
    juce::Label loopLengthLabel;
    CustomTextEditorDouble loopLengthTextEditor; // double
    juce::Label loopStartLabel;
    CustomTextEditorInt64 loopStartTextEditor; // int
    juce::Label minVoltageLabel;
    CustomTextEditorDouble minVoltageTextEditor; // double
    juce::Label pitchOffsetLabel;
    CustomTextEditorDouble pitchOffsetTextEditor; // double
    juce::Label sampleNameLabel;
    FileSelectLabel sampleNameSelectLabel; // filename
    juce::Label sampleEndLabel;
    CustomTextEditorInt64 sampleEndTextEditor; // int
    juce::Label sampleStartLabel;
    CustomTextEditorInt64 sampleStartTextEditor; // int

    void setEditComponentsEnabled (bool enabled);
    juce::PopupMenu createZoneEditMenu (std::function <void (ZoneProperties&, SampleProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter,
                                        std::function<bool (ZoneProperties&)> canCloneToZoneCallback, std::function<bool (ZoneProperties&)> canCloneToAllCallback);
    juce::String formatLoopLength (double loopLength);
    bool handleSamplesInternal (int zoneIndex, juce::StringArray files);
    void setupZoneComponents ();
    void setupZonePropertiesCallbacks ();
    double snapLoopLength (double rawValue);
    void updateLoopPointsView ();
    void updateSampleFileInfo (juce::String sample);
    void updateSamplePositionInfo ();

    void levelOffsetDataChanged (double levelOffset);
    void levelOffsetUiChanged (double levelOffset);
    void loopLengthDataChanged (std::optional<double> loopLength);
    void loopLengthUiChanged (double loopLength);
    void loopStartDataChanged (std::optional <juce::int64> loopStart);
    void loopStartUiChanged (juce::int64 loopStart);
    void minVoltageDataChanged (double minVoltage);
    void minVoltageUiChanged (double minVoltage);
    void pitchOffsetDataChanged (double pitchOffset);
    void pitchOffsetUiChanged (double pitchOffset);
    void sampleDataChanged (juce::String sample);
    void sampleUiChanged (juce::String sample);
    void sampleStartDataChanged (std::optional <juce::int64> sampleStart);
    void sampleStartUiChanged (juce::int64 sampleStart);
    void sampleEndDataChanged (std::optional <juce::int64> sampleEnd);
    void sampleEndUiChanged (juce::int64 sampleEnd);
    void setDropIndex (const juce::StringArray& files, int x, int y);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void fileDragEnter (const juce::StringArray& files, int, int) override;
    void fileDragMove (const juce::StringArray& files, int, int) override;
    void fileDragExit (const juce::StringArray& files) override;

    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized () override;
};
