#pragma once

#include <JuceHeader.h>
#include "AREnvelopeProperties.h"

class EnvelopeAnchor
{
public:
    void setTime (double newTime) { time = newTime; }
    void setAmplitude (double newAmplitude) { amplitude = newAmplitude; }
    double getTime () { return time; }
    double getAmplitude () { return amplitude; }

    void setX (double newX) { curX = newX; }
    void setY (double newY) { curY = newY; }
    double getX () { return curX; }
    double getY () { return curY; }

    void setMaxTime (double newMaxTime) { maxTime = newMaxTime; }
    double getMaxTime () { return maxTime; }

    void setActive (bool isActive) { active = isActive; }
    bool getActive () { return active; }

private:
    bool active { false };
    double time { 0.0 };
    double amplitude { 0.0 };
    double maxTime { 1.0 };

    double curX { 0.0f };
    double curY { 0.0f };
};

class AREnvelopeComponent : public juce::Component
{
public:
    AREnvelopeComponent ();
 
    juce::ValueTree getPropertiesVT () { return arEnvelopeProperties.getValueTree (); }

private:
    AREnvelopeProperties arEnvelopeProperties;
    double editorWidth { 0.0 };
    double editorHeight { 0.0 };
    EnvelopeAnchor startAnchor;
    EnvelopeAnchor attackAnchor;
    EnvelopeAnchor releaseAnchor;
    EnvelopeAnchor* curActiveAnchor { nullptr };
    int dragStartAnchorX { 0 };

    void attackPercentChanged (double attackPercent);
    void recalcAnchorPositions ();
    void releasePercentChanged (double releasePercent);

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void paint (juce::Graphics& g) override;
    void resized () override;
};
