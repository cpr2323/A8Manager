#pragma once

#include <JuceHeader.h>

class AudioManager;

// The adjustments offered for a sample point wherever it can be edited - the zone editor's fields, the loop
// tuner and the waveform: to the nearest zero crossing, or to where the sample value matches the other end of
// its pair, searching either way from where the point is. Each place says how to read and write the point, and
// the menus here do the rest.
struct SamplePointAdjust
{
    AudioManager* audioManager { nullptr };
    // the audio being searched, or nullptr when there is none
    std::function<juce::AudioBuffer<float>* ()> getAudioBuffer;
    std::function<int ()> getChannel;
    std::function<juce::int64 ()> getPosition;
    // how far a search may go either way
    std::function<juce::int64 ()> getMinPosition;
    std::function<juce::int64 ()> getMaxPosition;
    // the other end of the point's pair
    std::function<juce::int64 ()> getOppositePosition;
    // moving the point takes the other end with it (a Loop Start, with a Loop Length rather than a Loop End),
    // so it is matched against where the other end would be carried to
    bool oppositeMovesWithPoint { false };
    std::function<void (juce::int64)> setPosition;

    void moveToZeroCrossing (bool searchRight) const;
    void moveToOppositeLevel (bool searchRight) const;

    // the Left << / Right >> items for each
    juce::PopupMenu createZeroCrossingMenu (bool enabled) const;
    juce::PopupMenu createMatchOppositeMenu (bool enabled) const;
};
