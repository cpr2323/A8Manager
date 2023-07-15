#pragma once

#include <JuceHeader.h>

//#include "../Bezier/BezierProperties.h"

class ToolWindow : public juce::Component
{
public:
    ToolWindow ();
    void init (juce::ValueTree persistentRootPropertiesVT, juce::ValueTree runtimeRootPropertiesVT);

private:
//    BezierProperties bezierProperties;
//    juce::Label numberOfControlPointsLabel {"ControlPoints", "Control Points :" };
//    juce::TextEditor numberOfControlPointsEditor;

    void paint (juce::Graphics& g) override;
    void resized () override;
};
