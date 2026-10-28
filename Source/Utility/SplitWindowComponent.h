#pragma once

#include <JuceHeader.h>

class StretchableLayoutResizerBarWithCallback : public juce::StretchableLayoutResizerBar
{
public:
    StretchableLayoutResizerBarWithCallback (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical)
        : StretchableLayoutResizerBar (layoutToUse, itemIndexInLayout, isBarVertical) {}

    std::function<void ()> onLayoutChange;

private:
    void hasBeenMoved () override
    {
        StretchableLayoutResizerBar::hasBeenMoved ();

        if (onLayoutChange != nullptr)
            onLayoutChange ();
    }
};

class MouseProxy : public juce::MouseListener
{
public:
    std::function<void ()> onMouseEnter;
    std::function<void ()> onMouseExit;
    std::function<void (const juce::MouseEvent& me)> onMouseDrag;
private:
    void mouseDrag (const juce::MouseEvent& me) override
    {
        if (onMouseDrag)
            onMouseDrag (me);
    }
    void mouseEnter (const juce::MouseEvent&) override
    {
        if (onMouseEnter)
            onMouseEnter ();
    }
    void mouseExit (const juce::MouseEvent&) override
    {
        if (onMouseExit)
            onMouseExit ();
    }
};

class SplitWindowComponent : public juce::Component
{
public:
    SplitWindowComponent ();
    ~SplitWindowComponent ();

    void setComponents (juce::Component* firstComponent, juce::Component* secondComponent);
    bool getHorizontalSplit ();
    void setHorizontalSplit (bool horizontalSplit);
    void setSplitOffset (int newSplitOffset);
    int getSplitOffset ();

    /*
        Given the offset a drag is asking for, return the offset to use. This lets the owner keep a pane
        from being dragged narrower than it can be used at, or off the edge of the window. When unset,
        the pointer position is used as it is.
    */
    std::function<int (int proposedSplitOffset)> constrainSplitOffset;
    std::function<void ()> onLayoutChange;

private:
    juce::Component* firstComponent { nullptr };
    juce::Component* secondComponent { nullptr };
    juce::Rectangle<int> resizeBarBounds;
    bool horizontalSplit { true };
    int splitOffset { 0 };
    bool mouseOver { false };

    void resized () override;
    void paint (juce::Graphics& g) override;
    void mouseMove (const juce::MouseEvent& me) override;
    void mouseDrag (const juce::MouseEvent& me) override;
    void mouseExit (const juce::MouseEvent& me) override;
};
