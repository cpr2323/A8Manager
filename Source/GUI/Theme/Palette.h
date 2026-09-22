#pragma once

#include <JuceHeader.h>
#include <vector>

/*
    The single place every colour in the app is decided.

    A Palette resolves every (colourId, colour) pair from one scalar: the background
    level, 0.0 being the near-black scheme and 1.0 a white one. Nothing else in
    the app picks a colour literal.

    Tokens come in two kinds, and the difference is not cosmetic:

      ramp  - interpolated between the two ends. Every surface, line and fill
              does this happily.

      flip  - cannot be interpolated. If a background travels from dark to light
              and its ink travels with it, the two must cross somewhere, and at
              that point the ink is invisible. So ink switches ends rather than
              sliding, at the luminance where white ink and black ink are equally
              legible (0.179, both at 4.58:1), and is pushed toward pure white or
              black as it nears that point so contrast peaks where it is scarcest.

    The switch is per-surface, not global: the waveform lane starts as the
    darkest thing in the window and ends lighter than the panels, so it crosses
    at a different background level than the panels do. Each flip token names the
    surface it actually sits on.
*/
class Palette
{
public:
    Palette ();

    void  setBackground (float newBackground);
    float getBackground () const noexcept { return background; }

    juce::Colour get (int colourId) const;

    // Every pair this palette defines, for A8LookAndFeel to install. Includes
    // the A8Colours IDs, the JUCE widget IDs the app relies on, and oolib's.
    const std::vector<std::pair<int, juce::Colour>>& getColours () const noexcept { return colours; }

    // Background luminance at which ink has to change ends.
    static constexpr float kCrossover { 0.179f };
    static float relativeLuminance (juce::Colour colour) noexcept;

private:
    // Which resolved surface a flip token sits on.
    enum class Surface { none, background, lane, accentFill };

    struct TokenSpec
    {
        int          colourId;
        juce::Colour dark;      // value to use while the surface under it is dark
        juce::Colour light;     // value to use while that surface is light
        Surface      surface { Surface::none };  // none == plain interpolation
        float        push { 0.0f };              // how hard to drive toward pure white/black at the crossover
    };

    void resolve ();
    juce::Colour resolveToken (const TokenSpec& spec, float backgroundLevel,
                               const std::vector<float>& surfaceLuminance,
                               const std::vector<float>& surfaceLuminanceAtWhite) const;

    float background { 0.0f };
    std::vector<TokenSpec> specs;
    std::vector<std::pair<int, juce::Colour>> colours;
};
