#pragma once

#include <JuceHeader.h>
#include "LoopPoints/LoopPointsView.h"
#include "SamplePool/SamplePool.h"
#include "../../../Assimil8or/Audio/AudioPlayerProperties.h"
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "../../../AppProperties.h"

class FileSelectLabel : public juce::Label
{
public:
    FileSelectLabel ()
    {
        mouseEavesDropper.onMouseDown = [this] (const juce::MouseEvent&) { browseForSample (); };
        addMouseListener (&mouseEavesDropper, true);
    }
    ~FileSelectLabel ()
    {
        removeMouseListener (&mouseEavesDropper);
    }
    std::function<void (const juce::StringArray& files)> onFilesSelected;

    void setOutline (juce::Colour colour)
    {
        outlineColor = colour;
    }
private:
    juce::Colour outlineColor { juce::Colours::transparentWhite };
    std::unique_ptr<juce::FileChooser> fileChooser;

    class MouseEavesDropper : public juce::MouseListener
    {
    public:
        std::function<void (const juce::MouseEvent& event)> onMouseDown;
    private:
        void mouseDown (const juce::MouseEvent& event)
        {
            if (onMouseDown != nullptr)
                onMouseDown (event);
        }
    };
    MouseEavesDropper mouseEavesDropper;

    void browseForSample ()
    {
        fileChooser.reset (new juce::FileChooser ("Please select the Assimil8or Preset file you want to load...", {}, "*.wav"));
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () > 0 && fc.getURLResults () [0].isLocalFile () && onFilesSelected != nullptr)
            {
                for (auto urlResult : fc.getURLResults ())
                {
                    if (! urlResult.isLocalFile ())
                        return;
                    juce::File fileToLoad (urlResult.getLocalFile ().getFullPathName ());
                    if (fileToLoad.isDirectory ())
                        return;
                }

                juce::StringArray files;
                for (auto urlResult : fc.getURLResults ())
                    files.add (urlResult.getLocalFile ().getFullPathName ());
                onFilesSelected (files);
            }
        }, nullptr);
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        juce::Label::paintOverChildren (g);
        g.setColour (outlineColor);
        g.drawRect (getLocalBounds ());
    }
};

class ZoneEditor : public juce::Component,
                   public juce::FileDragAndDropTarget
{
public:
    ZoneEditor ();
    ~ZoneEditor () = default;

    void init (juce::ValueTree zonePropertiesVT, juce::ValueTree rootPropertiesVT, SamplePool* theSamplePool);
    void checkSampleExistence ();
    bool isSupportedAudioFile (juce::File file);
    void loadSample (juce::String sampleFileName);
    void receiveSampleLoadRequest (juce::File sampleFile);
    void setLoopLengthIsEnd (bool loopLengthIsEnd);

    std::function<void (juce::String)> onSampleChange;
    std::function<bool (double)> isMinVoltageInRange;
    std::function<double (double)> clampMinVoltage;
    std::function<void (int zoneIndex)> displayToolsMenu;
    std::function<bool (int zoneIndex, const juce::StringArray& files)> handleSamples;

private:
    AppProperties appProperties;
    AudioPlayerProperties audioPlayerProperties;
    ZoneProperties zoneProperties;
    ZoneProperties minZoneProperties;
    ZoneProperties maxZoneProperties;
    juce::AudioFormatManager audioFormatManager;
    juce::AudioBuffer<float> sampleAudioBuffer;
    SamplePool* samplePool;

    bool loopLengthIsEnd { false };
    int64_t sampleLength { 0 };

    bool draggingFiles { false };
    int dropIndex { 0 };

    LoopPointsView loopPointsView { sampleAudioBuffer };
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
    juce::TextEditor levelOffsetTextEditor; // double
    juce::Label loopLengthLabel;
    juce::TextEditor loopLengthTextEditor; // double
    juce::Label loopStartLabel;
    juce::TextEditor loopStartTextEditor; // int
    juce::Label minVoltageLabel;
    juce::TextEditor minVoltageTextEditor; // double
    juce::Label pitchOffsetLabel;
    juce::TextEditor pitchOffsetTextEditor; // double
    juce::Label sampleNameLabel;
    FileSelectLabel sampleNameSelectLabel; // filename
    juce::Label sampleEndLabel;
    juce::TextEditor sampleEndTextEditor; // int
    juce::Label sampleStartLabel;
    juce::TextEditor sampleStartTextEditor; // int

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
    void loopStartDataChanged (std::optional <int64_t> loopStart);
    void loopStartUiChanged (int64_t loopStart);
    void minVoltageDataChanged (double minVoltage);
    void minVoltageUiChanged (double minVoltage);
    void pitchOffsetDataChanged (double pitchOffset);
    void pitchOffsetUiChanged (double pitchOffset);
    void sampleDataChanged (juce::String sample);
    void sampleUiChanged (juce::String sample);
    void sampleStartDataChanged (std::optional <int64_t> sampleStart);
    void sampleStartUiChanged (int64_t sampleStart);
    void sampleEndDataChanged (std::optional <int64_t> sampleEnd);
    void sampleEndUiChanged (int64_t sampleEnd);
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
