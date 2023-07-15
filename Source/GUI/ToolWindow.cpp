#include "ToolWindow.h"

ToolWindow::ToolWindow ()
{
//     addAndMakeVisible (numberOfControlPointsLabel);
// 
//     numberOfControlPointsEditor.setInputRestrictions (2, "0123456789");
//     addAndMakeVisible (numberOfControlPointsEditor);
//     numberOfControlPointsEditor.onReturnKey = [this] () { bezierProperties.setNumberOfControlPoints (numberOfControlPointsEditor.getText ().getIntValue (), false); };
}

void ToolWindow::init (juce::ValueTree persistentRootPropertiesVT, juce::ValueTree runtimeRootPropertiesVT)
{
//     bezierProperties.wrap (persistentRootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::yes);
//     //bezierProperties.onNumberOfControlPointsChanged = [] () {};
// 
//     numberOfControlPointsEditor.setText (juce::String(bezierProperties.getNumberOfControlPoints ()), juce::dontSendNotification);
}

void ToolWindow::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::cadetblue);
}

void ToolWindow::resized ()
{
    auto localBounds { getLocalBounds () };

//     localBounds.reduce (5, 3);
//     numberOfControlPointsLabel.setBounds (localBounds.removeFromLeft (90));
// 
//     localBounds.removeFromLeft (5);
//     numberOfControlPointsEditor.setBounds (localBounds.removeFromLeft (75).reduced (0, 3));
}
