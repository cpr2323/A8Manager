#pragma once

#include <JuceHeader.h>
#include "EnvelopeAnchorComponent.h"

#if 0
class EnvelopeComponent : public juce::Component
{
public:
    EnvelopeComponent ();

    void initAnchors (long numberOfEnvelopeAnchors);
    void setAnchorXPercent (long anchorIndex, double xPercent);
    void setAnchorYPercent (long anchorIndex, double yPercent);

    double getAnchorXPercent (long anchorIndex);
    double getAnchorYPercent (long anchorIndex);

    void addAnchor (EnvelopeAnchorComponent* anchor, int index = -1);
    EnvelopeAnchorComponent* deleteAnchor (int index);

    void recalculateCoordinates ();

protected:
    juce::OwnedArray<EnvelopeAnchorComponent> envelopeAnchors;

private:
    const int kDefaultBorderWidth { 1 };
    const int kDefaultHandleSize { 9 };
    const juce::Colour kDefaultBackgroundColour { juce::Colours::lightgrey };
    const juce::Colour kDefaultBorderColour { juce::Colours::black };
    const juce::Colour kDefaultLineColour { juce::Colours::cyan };
    const juce::Colour kDefaultGlowColour { juce::Colours::cyan };
    const juce::Colour kDefaultNormalHandleColour { juce::Colours::royalblue }  ;
    const juce::Colour kDefaultActiveHandleColour { juce::Colours::red };
    const juce::Colour kDefaultDisabledHandleColour { juce::Colours::dimgrey };

    EnvelopeAnchorComponent* curActiveAnchor { nullptr };
    EnvelopeAnchorComponent* draggingAnchor { nullptr };
    juce::Colour backgroundColour { kDefaultBackgroundColour };
    juce::Colour borderColour { kDefaultBorderColour };
    juce::Colour lineColour { kDefaultLineColour };
    juce::Colour glowColour { kDefaultGlowColour };
    juce::Colour normalHandleColour { kDefaultNormalHandleColour };
    juce::Colour activeHandleColour { kDefaultActiveHandleColour };
    juce::Colour disabledHandleColourkDefaultDisabledHandleColour;
    juce::Image backgroundImage;
    bool resetWhenDone { false };
    long envelopeDisplayAreaWidth { 0 };
    long envelopeDisplayAreaHeight { 0 };
    long borderWidth { kDefaultBorderWidth };
    long anchorSize { kDefaultHandleSize };
    long offset { 0 };
    int startX { 0 };
    int startY { 0 };
    int startRectSelectX { -1 };
    int startRectSelectY { -1 };
    int curRectSelectX { -1 };
    int curRectSelectY { -1 };

    void paintBackground (juce::Graphics& g);
    void rightMouseClickOnAnchor (const juce::MouseEvent& e);
    void rightMouseClickOnBackground (const juce::MouseEvent& e);
    void mouseDownOnAnchor (const juce::MouseEvent& e);
    void mouseDownOnBackground (const juce::MouseEvent& e);
    void mouseUpDraggingAnchor (const juce::MouseEvent& e);
    void mouseUpRectangleSelection (const juce::MouseEvent& e);
    void mouseDragAnchor (const juce::MouseEvent& e);
    void mouseDragRectangleSelection (const juce::MouseEvent& e);
    void connectAnchorNeighbors (int index);
    void disconnectAnchorNeighbors (int index);

    virtual void anchorChanged () {};

    void paint (juce::Graphics& g) override;
    void resized () override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
};
#endif
