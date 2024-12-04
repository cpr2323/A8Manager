#pragma once

#include <JuceHeader.h>

class FileSelectLabel : public juce::Label
{
public:
    FileSelectLabel ();
    ~FileSelectLabel ();
    void setOutline (juce::Colour colour);
    void setFileFilter (juce::String newFileFilterString);

    std::function<void (const juce::StringArray& files)> onFilesSelected;
    std::function<void ()> onPopupMenuCallback;

private:
    juce::Colour outlineColor { juce::Colours::transparentWhite };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::String fileFilterString;

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

    void browseForSample ();
    void paintOverChildren (juce::Graphics& g) override;
};

