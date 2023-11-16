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
    void setMaxTime (int newMaxTime) { maxTime = newMaxTime; }
    void setMaxAmplitude (int newMaxAmplitude) { maxAmplitude = newMaxAmplitude; }
    void setSelected (bool newSelected) { selected = newSelected; updateDisplayState (); }
    void setSize (int newSize) { size = newSize; }

    int  getSize () { return size; }
    bool isActive () { return active; }
    bool isEnabled () { return enabled; }
    bool isSelected () { return selected; }

    int distanceMovedX (int xMove);
    int distanceMovedY (int yMove);
    void recalculateX (int offset, int displayWidth) { curTime = offset + static_cast<int> (displayWidth * xAxis.percentage); }
    void recalculateY (int offset, int displayHeight) { curAmplitude = offset + (displayHeight - static_cast<int> (displayHeight * yAxis.percentage)); }
    void recalculateXPercent (int offset, int displayWidth) { xAxis.percentage = (static_cast<double> (curTime)- static_cast<double> (offset)) / static_cast<double> (displayWidth); }
    void recalculateYPercent (int offset, int displayHeight) { yAxis.percentage = (static_cast<double> (displayHeight) - (static_cast<double> (curAmplitude) - static_cast<double> (offset))) / static_cast<double> (displayHeight); }

    EnvelopeAxis xAxis;
    EnvelopeAxis yAxis;
    int curTime { 0 };
    int curAmplitude { 0 };
    int maxTime { 0 };
    int maxAmplitude { 0 };
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

