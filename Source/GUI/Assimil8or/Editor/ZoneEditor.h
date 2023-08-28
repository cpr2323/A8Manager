#pragma once

#include <JuceHeader.h>
#include "../../../Assimil8or/Preset/ZoneProperties.h"
#include "../../../AppProperties.h"

class FileDropTargetTextEditor : public juce::TextEditor,
                                 public juce::FileDragAndDropTarget
{
public:
    std::function<bool (const juce::StringArray& files)> onCheckInterest;
    std::function<void (const juce::StringArray& files)> onFilesDropped;
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
        if (onFilesDropped!= nullptr)
            return onFilesDropped (files);
    }
    void fileDragEnter (const juce::StringArray& files, int, int) override
    {
        if (onDragEnter!= nullptr)
            return onDragEnter (files);

    }
    void fileDragMove (const juce::StringArray& files, int, int) override
    {
        if (onDragMove!= nullptr)
            return onDragMove (files);

    }
    void fileDragExit (const juce::StringArray& files) override
    {
        if (onDragExit!= nullptr)
            return onDragExit (files);
    }
    void setHoverOutline (juce::Colour colour)
    {
        outlineColor= colour;
    }
private:
    juce::Colour outlineColor { juce::Colours::transparentWhite };
    void paintOverChildren (juce::Graphics& g)
    {
        TextEditor::paintOverChildren (g);
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
    FileDropTargetTextEditor sampleNameTextEditor; // filename
    juce::Label sampleEndLabel;
    juce::TextEditor sampleEndTextEditor; // int
    juce::Label sampleStartLabel;
    juce::TextEditor sampleStartTextEditor; // int

    int64_t sampleLength { 0 };
    void setupZoneComponents ();
    void setupZonePropertiesCallbacks ();
    void updateSampleFileInfo (juce::String sample);
    juce::String formatLoopLength (double loopLength);
    bool isSupportedAudioFile (juce::File file);
    void loadSample (juce::String sampleFileName);

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
