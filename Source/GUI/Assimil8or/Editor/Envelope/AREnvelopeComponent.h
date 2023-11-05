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

    void setActive (bool isActive) { active = isActive; }
    bool getActive () { return active; }

private:
    bool active { false };
    double time { 0.0 };
    double amplitude { 0.0 };

    float curX { 0.0f };
    float curY { 0.0f };
};

class AREnvelopeComponent : public juce::Component
{
public:
    AREnvelopeComponent ();
 
    juce::ValueTree getPropertiesVT () { return arEnvelopeProperties.getValueTree (); }

private:
    AREnvelopeProperties arEnvelopeProperties;
    juce::Rectangle<float> editorArea;
    EnvelopeAnchor startAnchor;
    EnvelopeAnchor attackAnchor;
    EnvelopeAnchor releaseAnchor;
    EnvelopeAnchor* curActiveAnchor { nullptr };

    void attackPercentChanged (double attackPercent);
    void releasePercentChanged (double releasePercent);

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void paint (juce::Graphics& g) override;
    void resized () override;
};
