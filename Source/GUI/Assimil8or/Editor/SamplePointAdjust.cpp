#include "SamplePointAdjust.h"
#include "../../../Assimil8or/Audio/AudioManager.h"

namespace
{
    // a side the audio does not have (the right side of a mono file) is searched on its only channel
    int getSearchChannel (const juce::AudioBuffer<float>& audioBuffer, int channel)
    {
        return channel < audioBuffer.getNumChannels () ? channel : 0;
    }

    juce::PopupMenu createLeftRightMenu (bool enabled, std::function<void (bool searchRight)> move)
    {
        juce::PopupMenu menu;
        menu.addItem ("Left  <<", enabled, false, [move] () { move (false); });
        menu.addItem ("Right >>", enabled, false, [move] () { move (true); });
        return menu;
    }
}

void SamplePointAdjust::moveToZeroCrossing (bool searchRight) const
{
    auto audioBuffer { getAudioBuffer () };
    if (audioManager == nullptr || audioBuffer == nullptr)
        return;

    const auto channel { getSearchChannel (*audioBuffer, getChannel ()) };
    const auto zeroCrossing { searchRight ? audioManager->findNextZeroCrossing (getPosition (), getMaxPosition (), *audioBuffer, channel)
                                          : audioManager->findPreviousZeroCrossing (getPosition (), getMinPosition (), *audioBuffer, channel) };
    if (zeroCrossing != -1)
        setPosition (zeroCrossing);
}

void SamplePointAdjust::moveToOppositeLevel (bool searchRight) const
{
    auto audioBuffer { getAudioBuffer () };
    if (audioManager == nullptr || audioBuffer == nullptr)
        return;

    const auto channel { getSearchChannel (*audioBuffer, getChannel ()) };
    const auto match { searchRight ? audioManager->findNextMatchingLevel (getPosition (), getMaxPosition (), getOppositePosition (), oppositeMovesWithPoint, *audioBuffer, channel)
                                   : audioManager->findPreviousMatchingLevel (getPosition (), getMinPosition (), getOppositePosition (), oppositeMovesWithPoint, *audioBuffer, channel) };
    if (match != -1)
        setPosition (match);
}

juce::PopupMenu SamplePointAdjust::createZeroCrossingMenu (bool enabled) const
{
    return createLeftRightMenu (enabled, [adjust = *this] (bool searchRight) { adjust.moveToZeroCrossing (searchRight); });
}

juce::PopupMenu SamplePointAdjust::createMatchOppositeMenu (bool enabled) const
{
    return createLeftRightMenu (enabled, [adjust = *this] (bool searchRight) { adjust.moveToOppositeLevel (searchRight); });
}
