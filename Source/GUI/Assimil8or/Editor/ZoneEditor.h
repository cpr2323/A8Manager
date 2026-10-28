#pragma once

#include <JuceHeader.h>
#include "EditManager.h"
#include "LoopPoints/LoopPointsView.h"
#include "SamplePointAdjust.h"
#include "SampleManager/SampleProperties.h"
#include "../../../AppProperties.h"
#include "../../../Assimil8or/Audio/AudioManager.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "../../../Utility/CustomTextEditor.h"
#include "../../../Utility/FileSelectLabel.h"

/*
    The TRIGGER / LATCH audition buttons. They act on press, as the gate input of the module
    does, and say when they are released, so TRIGGER can hold the gate for as long as it is held.
    A lit button shows its 'on' colour: LATCH while it holds the gate.
*/
class ZonePlayButton : public juce::TextButton
{
public:
    ZonePlayButton ()
    {
        setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkcyan);
    }

    std::function<void ()> onPress;
    std::function<void ()> onRelease;

    void setLit (bool shouldBeLit)
    {
        setToggleState (shouldBeLit, juce::NotificationType::dontSendNotification);
    }

private:
    bool pressed { false };

    void mouseDown (const juce::MouseEvent& e) override
    {
        juce::TextButton::mouseDown (e);
        if (! isEnabled () || ! e.mods.isLeftButtonDown ())
            return;
        pressed = true;
        if (onPress != nullptr)
            onPress ();
    }
    void mouseUp (const juce::MouseEvent& e) override
    {
        juce::TextButton::mouseUp (e);
        if (! pressed)
            return;
        pressed = false;
        if (onRelease != nullptr)
            onRelease ();
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

    // the TRIGGER and LATCH buttons, for the keyboard; each does nothing while its button is disabled
    void pressTrigger ();
    void releaseTrigger ();
    void pressLatch ();

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
    ZonePlayButton triggerButton;
    ZonePlayButton latchButton;
    // the gate this zone is giving the audition voice: high while TRIGGER is held, or while LATCH holds it
    bool triggerHeld { false };
    bool latched { false };
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
    void setPlayButtonsEnabled (bool enabled);
    bool isPlayingThisZone ();
    void triggerVoice ();
    void unlatch ();
    juce::PopupMenu createZoneEditMenu (juce::PopupMenu existingPopupMenu, std::function <void (ZoneProperties&, SampleProperties&)> setter, std::function <void ()> resetter, std::function <void ()> reverter,
                                        std::function<bool (ZoneProperties&)> canCloneToZoneCallback, std::function<bool (ZoneProperties&)> canCloneToAllCallback);
    juce::String formatLoopLength (double loopLength);
    enum class SamplePoint { sampleStart, sampleEnd, loopStart, loopEnd };
    SamplePointAdjust getSamplePointAdjust (SamplePoint samplePoint);
    // the adjustments for one of the four points, added to the end of adjustMenu
    juce::PopupMenu createSamplePointAdjustMenu (SamplePoint samplePoint, juce::PopupMenu adjustMenu = {});
    void showLoopPointsViewMenu (bool isStartSide);
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

    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized () override;
};
