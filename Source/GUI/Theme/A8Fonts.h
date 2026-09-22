#pragma once

#include <JuceHeader.h>

/*
    The app's type: IBM Plex, in the three families SquidManager also uses. Plex was
    drawn for technical interfaces, and its condensed cut sets the small caps
    labels at a width the parameter grid can afford.

    Every size here is a CSS style font-size, which is the em size, so it
    goes through withPointHeight. withHeight would read it as ascent + descent,
    which for Plex is 1.3 em, and draw everything about a quarter too small.

    Letter spacing is also given in em, as CSS gives it. JUCE's tracking is a
    fraction of the JUCE height rather than of the em, so it is converted using
    the typeface's own metrics rather than passed straight through.
*/
class A8Fonts : private juce::DeletedAtShutdown
{
public:
    enum class Face
    {
        sans,
        sansMedium,
        condensed,
        condensedSemiBold,
        mono,
        monoMedium
    };

    static juce::Font make (Face face, float emSize, float letterSpacingEm = 0.0f);

    // the regular sans face, which the LookAndFeel installs as the default for
    // anything that does not ask for a font of its own
    static juce::Typeface::Ptr getDefaultTypeface ();

    ~A8Fonts () override;

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (A8Fonts)

private:
    A8Fonts ();
    juce::Typeface::Ptr getTypeface (Face face) const;

    // Held here rather than in function statics, so they are released with the
    // rest of JUCE at shutdown instead of after it.
    juce::Typeface::Ptr sansRegular;
    juce::Typeface::Ptr condensedRegular;
    juce::Typeface::Ptr condensedSemiBold;
    juce::Typeface::Ptr monoRegular;
};

/*
    One function per role, so a size or spacing is decided in one place. The
    names say where a style is used, not what it looks like.
*/
namespace A8Type
{
    // condensed semi bold, upper case
    juce::Font paneTitle ();        // FOLDERS, PRESETS
    juce::Font sectionHeader ();    // PRESET, PITCH, LEVEL, ZONES ...
    juce::Font parameterLabel ();   // SEMI, SRC, ATTACK ...
    juce::Font cuePointLabel ();    // SMPL START, LOOP LENGTH ... beside their fields
    juce::Font button ();           // TOOLS, SAVE
    juce::Font buttonSmall ();      // channel and zone TOOLS
    juce::Font channelTab ();       // CH 1 ... CH 8, and the zone tabs
    juce::Font cvTab ();            // the MIDI setup tabs
    juce::Font transport ();        // ONCE, LOOP
    juce::Font cvFieldLabel ();     // CV, Width
    juce::Font zonesTitle ();       // ZONES, which heads the zone column rather than a group of parameters

    // condensed regular
    juce::Font chip ();             // OUT, SETTINGS
    juce::Font mini ();             // OPTIONS, ALL
    juce::Font statusTag ();        // UNSAVED EDITS
    juce::Font caption ();          // the loop tuner's END / LOOP
    juce::Font markerLabel ();      // the waveform marker labels
    juce::Font menuSectionHeader ();

    // mono
    juce::Font value ();            // parameter fields and combo boxes
    juce::Font nameField ();        // the preset name
    juce::Font fileName ();         // the zone's sample file
    juce::Font meta ();             // the zone min voltage
    juce::Font cvValue ();          // the zone tab voltages
    juce::Font ruler ();            // the timeline over the waveform
    juce::Font presetNumber ();
    juce::Font count ();            // 31/199 in the presets header
    juce::Font chipValue ();        // the device name in the OUT chip
    juce::Font unit ();             // dB, SEMI after a value
    juce::Font glyph ();            // + and - glyphs

    // sans
    juce::Font body ();             // list rows, breadcrumbs, popup menus
    juce::Font bodyStrong ();       // the open preset file in the breadcrumbs
    juce::Font statusMessage ();    // the bottom status bar
}
