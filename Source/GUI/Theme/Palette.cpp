#include "Palette.h"
#include "A8ColourIds.h"
#include "oolib/GUI/ColourResolver.h"

namespace
{
    // ---- the two designed ends -------------------------------------------------
    // Shared with SquidManager, so the two apps read as a family: near-black is
    // violet biased rather than neutral grey, the accent is a cyan, and the status
    // colours are the greens, golds and reds of a module's panel LEDs.
    const juce::Colour kInk950Dark  { 0xff07060a }, kInk950Light  { 0xffffffff };
    const juce::Colour kInk900Dark  { 0xff0b0a0f }, kInk900Light  { 0xfff1f1f5 };
    const juce::Colour kInk850Dark  { 0xff100e15 }, kInk850Light  { 0xfff8f8fb };
    const juce::Colour kInk800Dark  { 0xff15131c }, kInk800Light  { 0xffebebf1 };
    const juce::Colour kInk750Dark  { 0xff1a1723 }, kInk750Light  { 0xffe8e8ef };
    const juce::Colour kInk700Dark  { 0xff211d2b }, kInk700Light  { 0xffdedee7 };
    const juce::Colour kInk650Dark  { 0xff2a2438 }, kInk650Light  { 0xffc2c2d0 };
    const juce::Colour kLineDark    { 0xff241f30 }, kLineLight    { 0xffdcdce5 };
    const juce::Colour kLineSoftDark { 0xff1c1826 }, kLineSoftLight { 0xffe9e9f0 };

    const juce::Colour kTextDark    { 0xffe4e0ec }, kTextLight    { 0xff191822 };
    const juce::Colour kDimDark     { 0xff9a93ab }, kDimLight     { 0xff56535f };
    const juce::Colour kMuteDark    { 0xff6a6380 }, kMuteLight    { 0xff7c7887 };
    const juce::Colour kGhostDark   { 0xff443e56 }, kGhostLight   { 0xffada9b8 };

    const juce::Colour kAccentDark     { 0xff2fb3e3 }, kAccentLight     { 0xff0e86b8 };
    const juce::Colour kAccentTextDark { 0xff2a9ac4 }, kAccentTextLight { 0xff0c79a6 };
    const juce::Colour kAccentDeepDark { 0xff17607c }, kAccentDeepLight { 0xff2e90bc };
    const juce::Colour kAccentInkDark  { 0xff04222e }, kAccentInkLight  { 0xffffffff };
    const juce::Colour kSelectDark     { 0xff111c22 }, kSelectLight     { 0xffdceff8 };

    const juce::Colour kGreenDark { 0xff35a97b }, kGreenLight { 0xff159163 };
    const juce::Colour kOliveDark { 0xffb9be3c }, kOliveLight { 0xff7e8317 };

    // waveform lane: the darkest surface at one end, lighter than the panels at
    // the other, which is why it flips at a different background level than they do
    const juce::Colour kLaneDark   { 0xff0a0910 }, kLaneLight   { 0xffededf3 };
    const juce::Colour kTraceDark  { 0xff82daf8 }, kTraceLight  { 0xff083e56 };
    const juce::Colour kCentreDark { 0x21e4e0ec }, kCentreLight { 0x2e1e1c28 };

    // markers. Start green and End red is the pairing that reads without being
    // learned; Loop is gold, and is the one value identical at both ends - it has
    // to hold on a near-black lane and a near-white one, so it must be a mid tone.
    // Loop End is violet: the one hue the other three, and the cyan trace, leave
    // free, and it sits with the violet bias of the inks rather than against it.
    const juce::Colour kMarkerStartDark   { 0xff35a97b }, kMarkerStartLight   { 0xff0b7a50 };
    const juce::Colour kMarkerLoop        { 0xffc58309 };
    const juce::Colour kMarkerEndDark     { 0xffd9483e }, kMarkerEndLight     { 0xffb0271d };
    const juce::Colour kMarkerLoopEndDark { 0xffa574e6 }, kMarkerLoopEndLight { 0xff7236b8 };


    // the switch track, and the halos that only do any work on a dark background
    const juce::Colour kSwitchOnDark { 0xff0c1f28 }, kSwitchOnLight { 0xffcfe9f5 };
    const juce::Colour kAccentEdgeDark { 0xff5ccbf2 }, kAccentEdgeLight { 0xff0a6d96 };
    const juce::Colour kRedDark { 0xffd9483e }, kRedLight { 0xffc0392b };
    const juce::Colour kAmberDark { 0xffd89a2b }, kAmberLight { 0xff9a6205 };
    const juce::Colour kDangerBackgroundDark { 0xff2a1414 }, kDangerBackgroundLight { 0xfffbe1de };

    const juce::Colour kGridDark { 0xff171422 }, kGridLight { 0xffdcdce6 };
    // lifted above the channel page when dark, and sunk below it when light, so the white
    // value fields stand out on it at the light end
    const juce::Colour kZoneDark { 0xff1c1926 }, kZoneLight { 0xffe0e0e9 };
    const juce::Colour kShadeDark { 0xa807060a }, kShadeLight { 0x47605e70 };

    const juce::Colour kTunerDashDark { 0x739a93ab }, kTunerDashLight { 0x661e1b28 };
    const juce::Colour kTunerDividerDark { 0x8ce4e0ec }, kTunerDividerLight { 0x8c181520 };
    const juce::Colour kCaptionDark { 0xff6a6380 }, kCaptionLight { 0xff5c5869 };
}

float Palette::relativeLuminance (juce::Colour colour) noexcept
{
    auto linear = [] (float channel)
    {
        return channel <= 0.03928f ? channel / 12.92f
                                   : std::pow ((channel + 0.055f) / 1.055f, 2.4f);
    };
    return 0.2126f * linear (colour.getFloatRed ())
         + 0.7152f * linear (colour.getFloatGreen ())
         + 0.0722f * linear (colour.getFloatBlue ());
}

Palette::Palette ()
{
    using S = Surface;
    const auto transparent { juce::Colours::transparentBlack };

    // push: 1.0 drives all the way to pure white / black at the crossover, which
    // is what plain text wants. Semantic colours use less, so they keep their hue
    // rather than washing out to white at the midpoint.
    specs =
    {
        // ---- surfaces: plain interpolation ----
        { A8Colours::windowBackground, kInk900Dark,  kInk900Light },
        { A8Colours::fieldBackground,  kInk950Dark,  kInk950Light },
        { A8Colours::listBackground,   kInk850Dark,  kInk850Light },
        { A8Colours::panelHeader,      kInk800Dark,  kInk800Light },
        { A8Colours::buttonBackground, kInk750Dark,  kInk750Light },
        { A8Colours::tabBackground,    kInk850Dark,  kInk850Light },
        { A8Colours::zoneBackground,   kZoneDark,    kZoneLight },
        { A8Colours::dialogBackground, kInk900Dark,  kInk900Light },
        { A8Colours::outline,          kLineDark,    kLineLight },
        { A8Colours::outlineDim,       kLineSoftDark, kLineSoftLight },
        { A8Colours::accent,           kAccentDark,  kAccentLight },
        { A8Colours::accentDeep,       kAccentDeepDark, kAccentDeepLight },
        { A8Colours::waveformBackground, kLaneDark,  kLaneLight },
        { A8Colours::markerLoop,       kMarkerLoop,  kMarkerLoop },
        { A8Colours::disabledOverlay,  kInk900Dark.withAlpha (0.62f), kInk900Light.withAlpha (0.62f) },
        { A8Colours::dropOverlay,      kInk950Dark.withAlpha (0.66f), juce::Colour (0xff605e70).withAlpha (0.42f) },
        { A8Colours::outlineStrong,    kInk650Dark,  kInk650Light },
        { A8Colours::hoverBackground,  kInk700Dark,  kInk700Light },
        { A8Colours::accentEdge,       kAccentEdgeDark, kAccentEdgeLight },
        { A8Colours::switchOnBackground, kSwitchOnDark, kSwitchOnLight },
        { A8Colours::dangerBackground, kDangerBackgroundDark, kDangerBackgroundLight },
        // a halo is a smudge on a light background, so these fade out rather than flip
        { A8Colours::accentGlow,       kAccentDark.withAlpha (0.5f), transparent },
        { A8Colours::ledGlow,          kGreenDark.withAlpha (0.8f),  transparent },
        { A8Colours::waveformGrid,     kGridDark,       kGridLight },
        { A8Colours::waveformShade,    kShadeDark,      kShadeLight },
        { A8Colours::tunerBackground,  kInk950Dark,     kLaneLight },
        { A8Colours::tunerDash,        kTunerDashDark,  kTunerDashLight },
        { A8Colours::tunerDivider,     kTunerDividerDark, kTunerDividerLight },

        // ---- ink on the panels ----
        { A8Colours::text,          kTextDark,       kTextLight,       S::background, 1.00f },
        { A8Colours::textDim,       kDimDark,        kDimLight,        S::background, 0.80f },
        { A8Colours::menuHeaderText, kMuteDark,      kMuteLight,       S::background, 0.80f },
        { A8Colours::textSelected,  kAccentTextDark, kAccentTextLight, S::background, 0.60f },
        { A8Colours::accentText,    kAccentTextDark, kAccentTextLight, S::background, 0.60f },
        { A8Colours::textSupported, kGreenDark,      kGreenLight,      S::background, 0.35f },
        { A8Colours::unsavedEdits,  kOliveDark,      kOliveLight,      S::background, 0.35f },
        { A8Colours::selectedRow,   kSelectDark,     kSelectLight },
        { A8Colours::textGhost,     kGhostDark,      kGhostLight,      S::background, 0.40f },
        { A8Colours::danger,        kRedDark,        kRedLight,        S::background, 0.35f },
        { A8Colours::warning,       kAmberDark,      kAmberLight,      S::background, 0.35f },

        // ---- ink on the waveform lane ----
        { A8Colours::waveformForeground, kTraceDark,  kTraceLight,       S::lane, 0.50f },
        { A8Colours::waveformCentreLine, kCentreDark, kCentreLight,      S::lane, 0.40f },
        { A8Colours::markerStart,        kMarkerStartDark, kMarkerStartLight, S::lane, 0.35f },
        { A8Colours::markerEnd,          kMarkerEndDark,   kMarkerEndLight,   S::lane, 0.35f },
        { A8Colours::markerLoopEnd,      kMarkerLoopEndDark, kMarkerLoopEndLight, S::lane, 0.35f },
        { A8Colours::tunerCaption,       kCaptionDark,     kCaptionLight,     S::lane, 0.60f },

        // ---- ink on an accent fill ----
        { A8Colours::accentInk, kAccentInkDark, kAccentInkLight, S::accentFill, 1.00f },

        // ---- JUCE widget roles ----
        { juce::Label::textColourId,       kTextDark, kTextLight, S::background, 1.00f },
        { juce::Label::backgroundColourId, transparent, transparent },

        { juce::TextEditor::backgroundColourId,      kInk950Dark, kInk950Light },
        { juce::TextEditor::textColourId,            kTextDark, kTextLight, S::background, 1.00f },
        { juce::TextEditor::outlineColourId,         kLineDark, kLineLight },
        { juce::TextEditor::focusedOutlineColourId,  kAccentDeepDark, kAccentDeepLight },
        { juce::TextEditor::highlightColourId,       kAccentDark.withAlpha (0.4f), kAccentLight.withAlpha (0.3f) },
        { juce::TextEditor::highlightedTextColourId, kTextDark, kTextLight, S::background, 1.00f },
        { juce::CaretComponent::caretColourId,       kAccentDark, kAccentLight },

        { juce::ComboBox::backgroundColourId,   kInk950Dark, kInk950Light },
        { juce::ComboBox::textColourId,         kTextDark, kTextLight, S::background, 1.00f },
        { juce::ComboBox::outlineColourId,      kLineDark, kLineLight },
        { juce::ComboBox::buttonColourId,       kInk950Dark, kInk950Light },
        { juce::ComboBox::arrowColourId,        kMuteDark, kMuteLight, S::background, 0.60f },
        { juce::ComboBox::focusedOutlineColourId, kAccentDeepDark, kAccentDeepLight },

        { juce::ListBox::backgroundColourId, kInk850Dark, kInk850Light },
        { juce::ListBox::textColourId,       kTextDark, kTextLight, S::background, 1.00f },
        { juce::ListBox::outlineColourId,    kLineDark, kLineLight },

        { juce::TextButton::buttonColourId,   kInk750Dark, kInk750Light },
        { juce::TextButton::buttonOnColourId, kAccentDark, kAccentLight },
        { juce::TextButton::textColourOffId,  kDimDark, kDimLight, S::background, 0.80f },
        { juce::TextButton::textColourOnId,   kAccentInkDark, kAccentInkLight, S::accentFill, 1.00f },

        { juce::ToggleButton::textColourId,       kTextDark, kTextLight, S::background, 1.00f },
        { juce::ToggleButton::tickColourId,       kAccentDark, kAccentLight },
        { juce::ToggleButton::tickDisabledColourId, kGhostDark, kGhostLight, S::background, 0.40f },

        { juce::PopupMenu::backgroundColourId,            kInk800Dark, kInk800Light },
        { juce::PopupMenu::textColourId,                  kTextDark, kTextLight, S::background, 1.00f },
        { juce::PopupMenu::headerTextColourId,            kMuteDark, kMuteLight, S::background, 0.80f },
        { juce::PopupMenu::highlightedBackgroundColourId, kSelectDark, kSelectLight },
        { juce::PopupMenu::highlightedTextColourId,       kTextDark, kTextLight, S::background, 1.00f },

        { juce::ScrollBar::thumbColourId,      kInk650Dark, kInk650Light },
        { juce::ScrollBar::trackColourId,      transparent, transparent },
        { juce::ScrollBar::backgroundColourId, transparent, transparent },

        { juce::TabbedComponent::backgroundColourId,   kInk900Dark, kInk900Light },
        { juce::TabbedComponent::outlineColourId,      kLineDark, kLineLight },
        { juce::TabbedButtonBar::tabOutlineColourId,   kLineDark, kLineLight },
        { juce::TabbedButtonBar::frontOutlineColourId, kAccentDark, kAccentLight },
        { juce::TabbedButtonBar::tabTextColourId,      kMuteDark, kMuteLight, S::background, 0.80f },
        { juce::TabbedButtonBar::frontTextColourId,    kTextDark, kTextLight, S::background, 1.00f },

        { juce::ResizableWindow::backgroundColourId, kInk900Dark, kInk900Light },
        { juce::DocumentWindow::textColourId,        kTextDark, kTextLight, S::background, 1.00f },

        // tooltips sit on whatever is under them, so the rim is ink rather than a
        // hairline: it has to stand off every surface at every background level
        { juce::TooltipWindow::backgroundColourId, kInk800Dark, kInk800Light },
        { juce::TooltipWindow::textColourId,       kTextDark, kTextLight, S::background, 1.00f },
        { juce::TooltipWindow::outlineColourId,    kDimDark, kDimLight, S::background, 0.80f },

        { juce::AlertWindow::backgroundColourId, kInk800Dark, kInk800Light },
        { juce::AlertWindow::textColourId,       kTextDark, kTextLight, S::background, 1.00f },
        { juce::AlertWindow::outlineColourId,    kLineDark, kLineLight },

        { juce::Slider::backgroundColourId,      kInk950Dark, kInk950Light },
        { juce::Slider::trackColourId,           kAccentDeepDark, kAccentDeepLight },
        { juce::Slider::thumbColourId,           kAccentDark, kAccentLight },
        { juce::Slider::textBoxTextColourId,     kTextDark, kTextLight, S::background, 1.00f },
        { juce::Slider::textBoxBackgroundColourId, kInk950Dark, kInk950Light },
        { juce::Slider::textBoxOutlineColourId,  kLineDark, kLineLight },

        // ---- oolib roles ----
        { oolib::ColourIds::splitterBackground,    kInk900Dark, kInk900Light },
        { oolib::ColourIds::splitterHandle,        kInk700Dark, kInk700Light },
        { oolib::ColourIds::splitterHandleOutline, kAccentDeepDark, kAccentDeepLight },
        { oolib::ColourIds::splitterDivider,       kLineSoftDark, kLineSoftLight },
        { oolib::ColourIds::slideSwitchTrackOff,   kInk950Dark, kInk950Light },
        { oolib::ColourIds::slideSwitchTrackOn,    kSwitchOnDark, kSwitchOnLight },
        { oolib::ColourIds::slideSwitchTrackOutlineOff, kLineDark, kLineLight },
        { oolib::ColourIds::slideSwitchTrackOutlineOn,  kAccentDeepDark, kAccentDeepLight },
        { oolib::ColourIds::slideSwitchThumbOff,   kGhostDark, kGhostLight, S::background, 0.40f },
        { oolib::ColourIds::slideSwitchThumbOn,    kAccentDark, kAccentLight },
        { oolib::ColourIds::slideSwitchThumbGlow,  kAccentDark.withAlpha (0.85f), transparent },
        { oolib::ColourIds::customTextEditorText,  kTextDark, kTextLight, S::background, 1.00f },
    };

    resolve ();
}

void Palette::setBackground (float newBackground)
{
    const auto clamped { std::clamp (newBackground, 0.0f, 1.0f) };
    if (juce::approximatelyEqual (clamped, background))
        return;
    background = clamped;
    resolve ();
}

juce::Colour Palette::get (int colourId) const
{
    for (const auto& [id, colour] : colours)
        if (id == colourId)
            return colour;

    // Every ID the app asks for should be defined here.
    jassertfalse;
    return juce::Colours::magenta;
}

juce::Colour Palette::resolveToken (const TokenSpec& spec, float backgroundLevel,
                                    const std::vector<float>& surfaceLuminance,
                                    const std::vector<float>& surfaceLuminanceAtWhite) const
{
    if (spec.surface == Surface::none)
        return spec.dark.interpolatedWith (spec.light, backgroundLevel);

    const auto surfaceIndex { static_cast<size_t> (spec.surface) };
    const auto backgroundLuminance { surfaceLuminance [surfaceIndex] };
    const auto onDark { backgroundLuminance < kCrossover };
    const auto base { onDark ? spec.dark : spec.light };

    // 0 at either end of the travel, 1 at the crossover. Squared, so the push is
    // concentrated where contrast is actually scarce and both ends land exactly
    // on the designed values.
    const auto maxLuminance { surfaceLuminanceAtWhite [surfaceIndex] };
    const auto span { onDark ? kCrossover
                             : std::max (1.0e-6f, maxLuminance - kCrossover) };
    const auto distance { onDark ? backgroundLuminance
                                 : maxLuminance - backgroundLuminance };
    const auto linearAmount { std::clamp (distance / span, 0.0f, 1.0f) };
    const auto amount { linearAmount * linearAmount * spec.push };

    const auto target { onDark ? juce::Colours::white : juce::Colours::black };
    // interpolatedWith would drag the alpha toward the target's, so put it back
    return base.interpolatedWith (target, amount).withAlpha (base.getFloatAlpha ());
}

void Palette::resolve ()
{
    // The reference surfaces are plain ramps, so they can be resolved first and
    // then used to decide which way every flip token goes.
    auto surfaceAt = [this] (int colourId, float atBackground)
    {
        for (const auto& spec : specs)
            if (spec.colourId == colourId)
                return spec.dark.interpolatedWith (spec.light, atBackground);
        jassertfalse;
        return juce::Colours::magenta;
    };

    const auto backgroundColour { surfaceAt (A8Colours::windowBackground, background) };
    const auto laneColour   { surfaceAt (A8Colours::waveformBackground, background) };
    const auto accentColour { surfaceAt (A8Colours::accent, background) };

    std::vector<float> luminance (4, 0.0f), luminanceAtWhite (4, 0.0f);
    auto setSurface = [&] (Surface surface, juce::Colour now, juce::Colour atWhite)
    {
        luminance [static_cast<size_t> (surface)] = relativeLuminance (now);
        luminanceAtWhite [static_cast<size_t> (surface)] = relativeLuminance (atWhite);
    };
    setSurface (Surface::background, backgroundColour, surfaceAt (A8Colours::windowBackground, 1.0f));
    setSurface (Surface::lane,       laneColour,   surfaceAt (A8Colours::waveformBackground, 1.0f));
    setSurface (Surface::accentFill, accentColour, surfaceAt (A8Colours::accent, 1.0f));

    colours.clear ();
    colours.reserve (specs.size ());
    for (const auto& spec : specs)
        colours.emplace_back (spec.colourId, resolveToken (spec, background, luminance, luminanceAtWhite));
}
