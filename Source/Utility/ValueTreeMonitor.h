#pragma once

#include <JuceHeader.h>

class ValueTreeMonitor : public juce::ValueTree::Listener
{
public:
    void assign (juce::ValueTree& vtToListenTo);
private:
    juce::ValueTree vtBeingListened;

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree& vt, juce::ValueTree& child) override;
    void valueTreeChildRemoved (juce::ValueTree& vt, juce::ValueTree& child, int index) override;
    void valueTreeChildOrderChanged (juce::ValueTree& vt, int oldIndex, int newIndex) override;
    void valueTreeParentChanged (juce::ValueTree& vt) override;
    void valueTreeRedirected (juce::ValueTree& vt) override;
};
