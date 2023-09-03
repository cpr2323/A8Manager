#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "../../../AppProperties.h"

class FileSelectLabel : public juce::Label,
                        public juce::FileDragAndDropTarget
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
    std::function<bool (const juce::StringArray& files)> onCheckInterest;
    std::function<void (const juce::StringArray& files)> onFilesSelected;
    std::function<void (const juce::StringArray& files)> onDragEnter;
    std::function<void (const juce::StringArray& files)> onDragMove;
    std::function<void (const juce::StringArray& files)> onDragExit;

    bool isInterestedInFileDrag (const juce::StringArray& files) override
    {
        if (onCheckInterest != nullptr)
            return onCheckInterest (files);
        return false;
    }
    void filesDropped (const juce::StringArray& files, int, int) override
    {
        if (onFilesSelected!= nullptr)
            onFilesSelected (files);
    }
    void fileDragEnter (const juce::StringArray& files, int, int) override
    {
        if (onDragEnter!= nullptr)
            onDragEnter (files);

    }
    void fileDragMove (const juce::StringArray& files, int, int) override
    {
        if (onDragMove!= nullptr)
            onDragMove (files);

    }
    void fileDragExit (const juce::StringArray& files) override
    {
        if (onDragExit!= nullptr)
            onDragExit (files);
    }
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
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) mutable
        {
            if (fc.getURLResults ().size () > 0 && fc.getURLResults () [0].isLocalFile () && onFilesSelected != nullptr)
            {
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

class ZoneEditor : public juce::Component
{
public:
    ZoneEditor ();
    ~ZoneEditor () = default;

    void init (juce::ValueTree zonePropertiesVT, juce::ValueTree rootPropertiesVT);
    void setLoopLengthIsEnd (bool loopLengthIsEnd);
    void receiveSampleLoadRequest (juce::File sampleFile);
    
    std::function<void(juce::String)> onSampleChange;

private:
    AppProperties appProperties;
    ZoneProperties zoneProperties;
    ZoneProperties minZoneProperties;
    ZoneProperties maxZoneProperties;
    juce::AudioFormatManager audioFormatManager;
    bool loopLengthIsEnd { false };

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

    juce::TextButton deleteButton;

    int64_t sampleLength { 0 };

    juce::String formatLoopLength (double loopLength);
    bool handleSelectedFile (juce::File fileNameAndPath);
    bool isSupportedAudioFile (juce::File file);
    void loadSample (juce::String sampleFileName);
    void setupZoneComponents ();
    void setupZonePropertiesCallbacks ();
    double snapLoopLength (double rawValue);
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

    void paint (juce::Graphics& g) override;
    void resized () override;
};
