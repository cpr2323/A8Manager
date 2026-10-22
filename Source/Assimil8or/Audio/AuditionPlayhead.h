#pragma once

#include <atomic>

/*
    Where the audition voice is reading, for the waveform displays to draw. The audio
    thread writes it once a block, and the GUI polls it, so it is plain atomics and
    never a ValueTree. The position is in frames of the source sample, so a display
    can map it as it does the markers; it is negative while no voice is sounding.
*/
struct AuditionPlayhead
{
    std::atomic<int> channelIndex { -1 };
    std::atomic<int> zoneIndex { -1 };
    std::atomic<double> position { -1.0 };
};
