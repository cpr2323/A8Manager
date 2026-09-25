#pragma once

/*
    Colour IDs for the roles this app paints itself, as opposed to the ones JUCE
    already defines for its own widgets (juce::Label::textColourId and friends).

    These are looked up the normal JUCE way, with Component::findColour, which
    falls through to whichever LookAndFeel is in scope. A8LookAndFeel registers
    every one of them from a Palette, so changing the Palette and repainting the
    top level windows re-colours the whole app.

    The base value is arbitrary but has to be unlikely to collide with JUCE's own
    IDs (which sit around 0x1000000-0x1010000) or oolib's (0x6f6f0000).
*/
namespace A8Colours
{
    enum ColourIds
    {
        // surfaces
        windowBackground = 0x41380001,  // the background every editor pane fills with
        fieldBackground,                // text editors, combo boxes
        listBackground,                 // list boxes and panes
        panelHeader,                    // strips that sit above a pane's content

        // ink
        text,                           // default label / value text
        textDim,                        // list rows at rest
        textSelected,                   // the selected preset row
        textSupported,                  // a file the module can use
        unsavedEdits,                   // olive, as a panel LED: the preset has edits that are not saved
        selectedRow,                    // tint behind a selected row or an active toggle

        outline,                        // hairlines and label outlines
        outlineDim,                     // the quieter divider between tabs
        disabledOverlay,                // wash over a control that cannot be used

        accent,                         // the cyan: selection and action
        accentText,                     // the same cyan, dark enough to read as text
        accentDeep,                     // borders and rails in the accent hue
        accentInk,                      // text drawn on top of an accent fill

        buttonBackground,               // buttons at rest

        // waveform region (fed into the oolib waveform / timeline / marker views)
        waveformBackground,
        waveformForeground,
        waveformCentreLine,
        markerStart,                    // green: where playback starts
        markerLoop,                     // gold - the one value shared by both ends; where the loop starts
        markerEnd,                      // red: where playback ends
        markerLoopEnd,                  // violet: where the loop ends

        // drag and drop overlay
        dropOverlay,                    // wash drawn over the drop target; the message on it uses the tooltip colours

        // chrome
        tabBackground,
        zoneBackground,                 // the zone pages, set off from the channel page around them
        dialogBackground,
        menuHeaderText,                 // the muted ink: inactive tabs, section headers in menus

        textGhost,                      // the quietest ink: separators, unlit values, preset numbers
        outlineStrong,                  // a border that has to be seen: scroll thumbs, pressed chips
        hoverBackground,                // behind a tool button under the pointer
        accentEdge,                     // the lighter rim of an accent fill
        accentGlow,                     // halo around a lit accent control; none on a light background
        switchOnBackground,             // the track of a slide switch that is on
        ledGlow,                        // halo around a lit led; none on a light background
        danger,                         // a destructive tool under the pointer, an error
        warning,                        // a problem that is not an error
        dangerBackground,

        waveformGrid,                   // the vertical divisions behind the trace
        waveformShade,                  // wash over the audio outside start .. end

        tunerBackground,
        tunerDash,                      // the loop tuner's zero line
        tunerDivider,                   // where the loop end meets the loop start
        tunerCaption
    };
}
