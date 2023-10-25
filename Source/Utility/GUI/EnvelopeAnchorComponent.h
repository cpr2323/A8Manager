#pragma once

#include <JuceHeader.h>
#include "EnvelopeAxis.h"

class EnvelopeAnchorComponent
{
public:
    EnvelopeAnchorComponent ();

    void display (juce::Graphics& g, int xOffset, int yOffset);

    void setActive (bool newActive) { active = newActive; updateDisplayState (); }
    void setEnabled (bool newEnabled) { enabled = newEnabled; updateDisplayState (); }
    void setMaxX (int newMaxX) { maxX = newMaxX; }
    void setMaxY (int newMaxY) { maxY = newMaxY; }
    void setSelected (bool newSelected) { selected = newSelected; updateDisplayState (); }
    void setSize (int newSize) { size = newSize; }

    int  getSize () { return size; }
    bool isActive () { return active; }
    bool isEnabled () { return enabled; }
    bool isSelected () { return selected; }

    int distanceMovedX (int xMove);
    int distanceMovedY (int yMove);
    void recalculateX (int offset, int displayWidth) { curX = offset + (long)(displayWidth * xAxis.percentage); }
    void recalculateY (int offset, int displayHeight) { curY = offset + (displayHeight - (long)(displayHeight * yAxis.percentage)); }
    void recalculateXPercent (int offset, int displayWidth) { xAxis.percentage = ((double) curX - (double) offset) / (double) displayWidth; }
    void recalculateYPercent (int offset, int displayHeight) { yAxis.percentage = ((double) displayHeight - ((double) curY - (double) offset)) / (double) displayHeight; }

    EnvelopeAxis xAxis;
    EnvelopeAxis yAxis;
    int curX { 0 };
    int curY { 0 };
    int maxX { 0 };
    int maxY { 0 };
    EnvelopeAnchorComponent* anchorToLeft { nullptr };
    EnvelopeAnchorComponent* anchorToRight { nullptr };
    juce::Colour color;

private:
    int size { 7 };
    bool enabled { true };
    bool active { false };
    bool selected { false };

    void updateDisplayState ();
    void drawCircle (juce::Graphics& g, float centerX, float centerY, float startSize, float size, juce::Colour color);
};

