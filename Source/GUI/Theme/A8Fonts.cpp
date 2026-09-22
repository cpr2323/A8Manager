#include "A8Fonts.h"
#include <BinaryData.h>

namespace
{
    juce::Typeface::Ptr loadTypeface (const char* data, int size)
    {
        return juce::Typeface::createSystemTypefaceFor (data, static_cast<size_t> (size));
    }
}

A8Fonts::A8Fonts ()
    : sansRegular (loadTypeface (BinaryData::IBMPlexSansRegular_ttf, BinaryData::IBMPlexSansRegular_ttfSize)),
      condensedRegular (loadTypeface (BinaryData::IBMPlexSansCondensedRegular_ttf, BinaryData::IBMPlexSansCondensedRegular_ttfSize)),
      condensedSemiBold (loadTypeface (BinaryData::IBMPlexSansCondensedSemiBold_ttf, BinaryData::IBMPlexSansCondensedSemiBold_ttfSize)),
      monoRegular (loadTypeface (BinaryData::IBMPlexMonoRegular_ttf, BinaryData::IBMPlexMonoRegular_ttfSize))
{
}

A8Fonts::~A8Fonts ()
{
    clearSingletonInstance ();
}

juce::Typeface::Ptr A8Fonts::getTypeface (Face face) const
{
    switch (face)
    {
        // A few roles ask for the 500 weight of Sans and Mono. Those
        // cuts are not embedded, so they fall back to the regular weight; the
        // colour those values are drawn in already sets them apart.
        case Face::sans:
        case Face::sansMedium:        return sansRegular;
        case Face::condensed:         return condensedRegular;
        case Face::condensedSemiBold: return condensedSemiBold;
        case Face::mono:
        case Face::monoMedium:        return monoRegular;
    }
    jassertfalse;
    return sansRegular;
}

juce::Typeface::Ptr A8Fonts::getDefaultTypeface ()
{
    return getInstance ()->sansRegular;
}

juce::Font A8Fonts::make (Face face, float emSize, float letterSpacingEm)
{
    juce::Font font { juce::FontOptions (getInstance ()->getTypeface (face)).withPointHeight (emSize) };
    if (juce::approximatelyEqual (letterSpacingEm, 0.0f))
        return font;

    // tracking is applied as a fraction of the JUCE height, so scale the em
    // spacing by how tall this face is per em
    const auto heightPerEm { font.getHeight () / emSize };
    font.setExtraKerningFactor (letterSpacingEm / heightPerEm);
    return font;
}

namespace A8Type
{
    using Face = A8Fonts::Face;

    juce::Font paneTitle ()         { return A8Fonts::make (Face::condensedSemiBold, 10.5f, 0.13f); }
    juce::Font sectionHeader ()     { return A8Fonts::make (Face::condensedSemiBold, 9.5f, 0.14f); }
    juce::Font parameterLabel ()    { return A8Fonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font cuePointLabel ()     { return A8Fonts::make (Face::condensedSemiBold, 9.5f, 0.10f); }
    juce::Font button ()            { return A8Fonts::make (Face::condensedSemiBold, 10.5f, 0.10f); }
    juce::Font buttonSmall ()       { return A8Fonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font channelTab ()        { return A8Fonts::make (Face::condensedSemiBold, 11.5f, 0.09f); }
    juce::Font cvTab ()             { return A8Fonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font transport ()         { return A8Fonts::make (Face::condensedSemiBold, 10.0f, 0.11f); }
    juce::Font cvFieldLabel ()      { return A8Fonts::make (Face::condensedSemiBold, 10.0f, 0.10f); }
    juce::Font zonesTitle ()        { return A8Fonts::make (Face::condensedSemiBold, 14.0f, 0.12f); }

    juce::Font chip ()              { return A8Fonts::make (Face::condensed, 10.5f, 0.08f); }
    juce::Font mini ()              { return A8Fonts::make (Face::condensed, 10.0f, 0.07f); }
    juce::Font statusTag ()         { return A8Fonts::make (Face::condensed, 10.0f, 0.09f); }
    juce::Font caption ()           { return A8Fonts::make (Face::condensed, 8.5f, 0.11f); }
    juce::Font markerLabel ()       { return A8Fonts::make (Face::condensed, 9.0f, 0.10f); }
    juce::Font menuSectionHeader () { return A8Fonts::make (Face::condensed, 9.5f, 0.13f); }

    juce::Font value ()             { return A8Fonts::make (Face::mono, 11.5f); }
    juce::Font nameField ()         { return A8Fonts::make (Face::mono, 12.5f, 0.02f); }
    juce::Font fileName ()          { return A8Fonts::make (Face::mono, 12.5f); }
    juce::Font meta ()              { return A8Fonts::make (Face::mono, 11.0f); }
    juce::Font cvValue ()           { return A8Fonts::make (Face::mono, 11.0f); }
    juce::Font ruler ()             { return A8Fonts::make (Face::mono, 9.0f); }
    juce::Font presetNumber ()        { return A8Fonts::make (Face::mono, 10.5f); }
    juce::Font count ()             { return A8Fonts::make (Face::mono, 10.0f); }
    juce::Font chipValue ()         { return A8Fonts::make (Face::monoMedium, 10.5f); }
    juce::Font unit ()              { return A8Fonts::make (Face::mono, 10.0f); }
    juce::Font glyph ()             { return A8Fonts::make (Face::mono, 13.0f); }

    juce::Font body ()              { return A8Fonts::make (Face::sans, 12.0f); }
    juce::Font bodyStrong ()        { return A8Fonts::make (Face::sansMedium, 12.0f); }
    juce::Font statusMessage ()     { return A8Fonts::make (Face::sans, 11.5f); }
}
