#pragma once

#include <JuceHeader.h>
#include "EditManager.h"
#include "LoopPoints/LoopPointsView.h"
#include "SampleManager/SampleProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioManager.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "oolib/GUI/CustomTextEditor.h"
#include "oolib/GUI/FileSelectLabel.h"
#include "../../Theme/UiComponents.h"

/*
    The ONCE / LOOP audition buttons. They say STOP while their sample is playing,
    and take the accent fill then - cyan means playing, as it does in SquidManager.
*/
class ZonePlayButton : public juce::TextButton
{
public:
    void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        const auto enabled { isEnabled () };
        const auto playing { getButtonText () == "STOP" };
        const auto hovered { enabled && (isMouseOver || isMouseDown) };
        const auto area { getLocalBounds ().toFloat ().reduced (0.5f) };

        // a control that cannot be used is dimmed as a whole
        g.beginTransparencyLayer (enabled ? 1.0f : 0.4f);
        g.setColour (findColour (playing ? A8Colours::accent : A8Colours::buttonBackground));
        g.fillRoundedRectangle (area, 2.0f);
        g.setColour (findColour (playing ? A8Colours::accentEdge
                                         : (hovered ? A8Colours::accentDeep : A8Colours::outline)));
        g.drawRoundedRectangle (area, 2.0f, 1.0f);
        g.setFont (A8Type::transport ().withPointHeight (8.5f));
        g.setColour (findColour (playing ? A8Colours::accentInk
                                         : (hovered ? A8Colours::text : A8Colours::textDim)));
        g.drawText (getButtonText (), getLocalBounds (), juce::Justification::centred, false);
        g.endTransparencyLayer ();
    }
};

class ZoneEditor : public juce::Component,
                   public juce::FileDragAndDropTarget
{
public:
    ZoneEditor ();
    ~ZoneEditor () = default;

    void init (juce::ValueTree zonePropertiesVT, juce::ValueTree uneditedZonePropertiesVT, juce::ValueTree rootPropertiesVT);
    // TODO - can we move this to the EditManager, as it just calls editManager->assignSamples (parentChannelIndex, startingZoneIndex, files); in the ZoneEditor
    void receiveSampleLoadRequest (juce::File sampleFile);
    // TODO - is there a VTW that could manage this setting?
    void setLoopLengthIsEnd (bool loopLengthIsEnd);
    void setStereoRightChannelMode (bool isStereoRightChannelMode);

    // TODO - can we make this local, since we should be able to access the edits through the EditManager
    std::function<void (int zoneIndex)> displayToolsMenu;

private:
    class ClickListener : public juce::MouseListener
    {
    public:
        std::function<void ()> onClick;
    private:
        void mouseDown (const juce::MouseEvent&) override
        {
            if (onClick != nullptr)
                onClick ();
        }
    };

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
    AudioManager* audioManager { nullptr };
    int zoneIndex { -1 };
    int parentChannelIndex { -1 };
    bool isStereoRightChannelMode { false };

    // Loop Length is always stored as loop length, but the UI can be toggled to display it, and take input for it, as if it is Loop End
    bool treatLoopLengthAsEndInUi { false };

    int draggingFilesCount { 0 };
    bool supportedFile { false };
    juce::String dropMsg;
    juce::StringArray dropDetails;

    int dropIndex { 0 };

    LoopPointsView loopPointsView;
    ZonePlayButton oneShotPlayButton;
    ZonePlayButton loopPlayButton;
    MenuButton toolsButton { "ZONE TOOLS", ActionButton::Size::small };
    // the labels are dimmed next to the values, and a palette change has to reach them again
    std::vector<juce::Label*> parameterLabels;
    bool sampleFileMissing { false };
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
    juce::TextButton leftChannelSelectButton;
    juce::TextButton rightChannelSelectButton;
    juce::Label sampleNameLabel;
    FileSelectLabel sampleNameSelectLabel; // filename
    juce::Label sampleEndLabel;
    CustomTextEditorInt64 sampleEndTextEditor; // int
    juce::Label sampleStartLabel;
    CustomTextEditorInt64 sampleStartTextEditor; // int

    ClickListener selectSamplePointsClickListener;
    ClickListener selectLoopPointsClickListener;

    AudioPlayerProperties::SamplePointsSelector samplePointsSelector { AudioPlayerProperties::SamplePointsSelector::SamplePoints };

    void setEditComponentsEnabled (bool enabled);
    juce::PopupMenu createZoneEditMenu (juce::PopupMenu existingPopupMenu, std::function <void (ZoneProperties&, SampleProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter,
                                        std::function<bool (ZoneProperties&)> canCloneToZoneCallback, std::function<bool (ZoneProperties&)> canCloneToAllCallback);
    juce::String formatLoopLength (double loopLength);
    auto getSampleAdjustMenu (std::function<juce::int64 ()> getSampleOffset, std::function<juce::int64 ()> getMinSampleOffset, std::function<juce::int64 ()>getMaxSampleOffset, std::function<void (juce::int64)> setSampleOffset);
    bool handleSamplesInternal (int zoneIndex, juce::StringArray files);
    void setActiveSamplePoints (AudioPlayerProperties::SamplePointsSelector samplePointsSelector, bool forceSetup);
    void setupZoneComponents ();
    void setupZonePropertiesCallbacks ();
    double snapLoopLength (double rawValue);
    void updateLoopPointsView ();
    void updateSampleFileInfo (juce::String sample);
    void updateSamplePositionInfo ();
    void updateSideSelectButtons (int side);

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
    void resetDropInfo ();
    void sampleDataChanged (juce::String sample);
    void sampleUiChanged (juce::String sample);
    void sampleStartDataChanged (std::optional <juce::int64> sampleStart);
    void sampleStartUiChanged (juce::int64 sampleStart);
    void sampleEndDataChanged (std::optional <juce::int64> sampleEnd);
    void sampleEndUiChanged (juce::int64 sampleEnd);
    void sideDataChanged (int side);
    void sideUiChanged (int side);
    void setDropIndex (const juce::StringArray& files, int x, int y);
    void updateDropInfo (const juce::StringArray& files);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void fileDragEnter (const juce::StringArray& files, int, int) override;
    void fileDragMove (const juce::StringArray& files, int, int) override;
    void fileDragExit (const juce::StringArray& files) override;

    void applyExplicitColours ();
    void lookAndFeelChanged () override;
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized () override;
};
