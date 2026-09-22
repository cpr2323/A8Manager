#pragma once

#include <JuceHeader.h>
#include "A8ColourIds.h"
#include "A8Fonts.h"
#include "A8LookAndFeel.h"

/*
    Small pieces the app repeats often enough that they should only exist once.
    All of them resolve their colours at paint time, so they follow the palette
    without needing to be told a change happened.

    Anything that can be clicked answers the pointer the same way: its border
    takes the deep accent, as the sample file chip always has.
*/

namespace A8Paint
{
    // A section of the editor, or a whole pane: a filled, outlined panel lifted
    // off the background.
    inline void card (juce::Graphics& g, const juce::Component& colourSource, juce::Rectangle<int> bounds,
                      int fillColourId = A8Colours::listBackground, float cornerSize = 3.0f)
    {
        const auto area { bounds.toFloat ().reduced (0.5f) };
        g.setColour (colourSource.findColour (fillColourId));
        g.fillRoundedRectangle (area, cornerSize);
        g.setColour (colourSource.findColour (A8Colours::outline));
        g.drawRoundedRectangle (area, cornerSize, 1.0f);
    }

    // The plate behind a transient message, such as what a file drop will do. It is
    // coloured exactly as a tooltip is, from the tooltip's own colour IDs, so the
    // two kinds of passing message look alike and cannot drift apart.
    inline void messagePlate (juce::Graphics& g, const juce::Component& colourSource, juce::Rectangle<float> bounds, float cornerSize)
    {
        g.setColour (colourSource.findColour (juce::TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle (bounds, cornerSize);
        g.setColour (colourSource.findColour (juce::TooltipWindow::outlineColourId));
        g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);
    }

    // The ink for a message on a messagePlate; a file that cannot be used is said so in red.
    inline juce::Colour messageInk (const juce::Component& colourSource, bool isProblem = false)
    {
        return colourSource.findColour (isProblem ? static_cast<int> (A8Colours::danger)
                                                  : static_cast<int> (juce::TooltipWindow::textColourId));
    }

    // The width a line of text takes in a font, tracking included.
    inline int textWidth (const juce::Font& font, const juce::String& text)
    {
        return juce::roundToInt (std::ceil (juce::GlyphArrangement::getStringWidth (font, text)));
    }

    // A small solid triangle pointing down, centred on a point. Drawn rather than
    // typed, as the embedded faces do not carry the arrow glyphs.
    inline void caretDown (juce::Graphics& g, juce::Point<float> centre, float width)
    {
        const auto height { width * 0.55f };
        juce::Path caret;
        caret.addTriangle (centre.x - (width * 0.5f), centre.y - (height * 0.5f),
                           centre.x + (width * 0.5f), centre.y - (height * 0.5f),
                           centre.x, centre.y + (height * 0.5f));
        g.fillPath (caret);
    }

    // A small solid triangle pointing right, as a folder row's marker.
    inline void caretRight (juce::Graphics& g, juce::Point<float> centre, float height)
    {
        const auto width { height * 0.62f };
        juce::Path caret;
        caret.addTriangle (centre.x - (width * 0.5f), centre.y - (height * 0.5f),
                           centre.x - (width * 0.5f), centre.y + (height * 0.5f),
                           centre.x + (width * 0.5f), centre.y);
        g.fillPath (caret);
    }

    // A gear, for a menu of settings: a toothed ring with a hole, drawn as one path
    // so the hole is cut out rather than painted over.
    inline void gear (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        static constexpr auto kTeeth { 8 };
        const auto centre { bounds.getCentre () };
        const auto outerRadius { std::min (bounds.getWidth (), bounds.getHeight ()) * 0.5f };
        const auto rootRadius { outerRadius * 0.74f };
        const auto holeRadius { outerRadius * 0.34f };
        const auto toothHalfAngle { juce::MathConstants<float>::pi / static_cast<float> (kTeeth) * 0.5f };

        juce::Path shape;
        auto started { false };
        for (auto tooth { 0 }; tooth < kTeeth; ++tooth)
        {
            const auto angle { juce::MathConstants<float>::twoPi * static_cast<float> (tooth) / static_cast<float> (kTeeth) };
            const auto points = { centre.getPointOnCircumference (rootRadius, angle - (toothHalfAngle * 2.0f)),
                                  centre.getPointOnCircumference (outerRadius, angle - toothHalfAngle),
                                  centre.getPointOnCircumference (outerRadius, angle + toothHalfAngle),
                                  centre.getPointOnCircumference (rootRadius, angle + (toothHalfAngle * 2.0f)) };
            for (const auto& point : points)
            {
                if (! started)
                    shape.startNewSubPath (point);
                else
                    shape.lineTo (point);
                started = true;
            }
        }
        shape.closeSubPath ();
        shape.addEllipse (centre.x - holeRadius, centre.y - holeRadius, holeRadius * 2.0f, holeRadius * 2.0f);
        shape.setUsingNonZeroWinding (false);
        g.fillPath (shape);
    }

    // An audio file: a short run of bars of differing heights, like a waveform.
    inline void audioFile (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        static constexpr std::array<float, 5> kBarHeights { 0.45f, 0.9f, 0.6f, 1.0f, 0.4f };
        const auto barPitch { bounds.getWidth () / static_cast<float> (kBarHeights.size ()) };
        const auto barWidth { std::max (1.0f, barPitch - 0.6f) };
        for (size_t barIndex { 0 }; barIndex < kBarHeights.size (); ++barIndex)
        {
            const auto barHeight { bounds.getHeight () * kBarHeights [barIndex] };
            g.fillRoundedRectangle (bounds.getX () + (barPitch * static_cast<float> (barIndex)), bounds.getCentreY () - (barHeight * 0.5f),
                                    barWidth, barHeight, barWidth * 0.5f);
        }
    }

    // A folder: a tab on the left of a slightly rounded body.
    inline void folder (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        const auto tabHeight { bounds.getHeight () * 0.2f };
        juce::Path shape;
        shape.addRoundedRectangle (bounds.getX (), bounds.getY () + tabHeight, bounds.getWidth (), bounds.getHeight () - tabHeight, 1.0f);
        shape.addRoundedRectangle (bounds.getX (), bounds.getY (), bounds.getWidth () * 0.45f, tabHeight * 2.0f, 1.0f);
        g.fillPath (shape);
    }
}

/*
    The module reports itself with LEDs, so the app does too: a hollow ring for
    "nothing here", a lit dot for "has content". Two states only - anything more
    would be inventing information the app does not have. On a dark background a lit
    led carries a soft halo; the ledGlow token fades that out as the background lightens.
*/
class StatusLed : public juce::Component
{
public:
    explicit StatusLed (int theLitColourId = A8Colours::markerStart)
        : litColourId (theLitColourId)
    {
        setInterceptsMouseClicks (false, false);
    }

    void setLit (bool shouldBeLit)
    {
        if (lit == shouldBeLit)
            return;
        lit = shouldBeLit;
        repaint ();
    }
    bool isLit () const noexcept { return lit; }

    void paint (juce::Graphics& g) override
    {
        draw (g, getLocalBounds ().toFloat ().withSizeKeepingCentre (kDiameter, kDiameter), lit, *this, litColourId);
    }

    // for painting a led inline, where a child component would be awkward
    static void draw (juce::Graphics& g, juce::Rectangle<float> bounds, bool lit,
                      const juce::Component& colourSource, int litColourId = A8Colours::markerStart)
    {
        if (lit)
        {
            const auto litColour { colourSource.findColour (litColourId) };
            const auto glowAmount { colourSource.findColour (A8Colours::ledGlow).getFloatAlpha () };
            if (glowAmount > 0.0f)
            {
                juce::Path dot;
                dot.addEllipse (bounds);
                juce::DropShadow (litColour.withAlpha (glowAmount), 6, {}).drawForPath (g, dot);
            }
            g.setColour (litColour);
            g.fillEllipse (bounds);
        }
        else
        {
            g.setColour (colourSource.findColour (A8Colours::textGhost));
            g.drawEllipse (bounds.reduced (0.5f), 1.0f);
        }
    }

    static constexpr float kDiameter { 6.0f };

private:
    int litColourId;
    bool lit { false };
};

/*
    Repaints a control whenever the pointer enters or leaves it, or any of its
    children.

    A ComboBox or TextEditor is almost entirely covered by child components, so
    the pointer enters and leaves those rather than the control itself, and the
    control's own repaint-on-hover rarely fires: its highlight would either not
    appear, or not go away. Listening to the children as well fixes both.
*/
class HoverHighlight : private juce::MouseListener
{
public:
    static void attach (juce::Component& control)
    {
        control.getProperties ().set (kHoverTargetId, true);
        control.addMouseListener (&getInstance (), true);
    }

private:
    static inline const juce::Identifier kHoverTargetId { "hoverHighlightTarget" };

    static HoverHighlight& getInstance ()
    {
        static HoverHighlight instance;
        return instance;
    }

    // the event arrives for whichever child is under the pointer; the control to
    // repaint is the nearest one that asked for this
    static void repaintTarget (const juce::MouseEvent& mouseEvent)
    {
        for (auto* component { mouseEvent.eventComponent }; component != nullptr; component = component->getParentComponent ())
        {
            if (component->getProperties ().contains (kHoverTargetId))
            {
                component->repaint ();
                return;
            }
        }
    }

    void mouseEnter (const juce::MouseEvent& mouseEvent) override { repaintTarget (mouseEvent); }
    void mouseExit (const juce::MouseEvent& mouseEvent) override  { repaintTarget (mouseEvent); }
    void mouseUp (const juce::MouseEvent& mouseEvent) override    { repaintTarget (mouseEvent); }
};

/*
    Tracks which row of a ListBox is under the pointer, so the model can outline
    it when painting. ListBoxModel is never told about hover itself.
*/
class ListRowHover : private juce::MouseListener
{
public:
    explicit ListRowHover (juce::ListBox& theListBox)
        : listBox (theListBox)
    {
        listBox.addMouseListener (this, true);
    }

    ~ListRowHover () override
    {
        listBox.removeMouseListener (this);
    }

    // -1 unless the pointer is actually over the list. A modal dialog, such as the
    // unsaved preset warning, takes the pointer without the list ever seeing it leave,
    // so the last row it saw would otherwise stay outlined after the list changed.
    int getRow () const { return listBox.isMouseOver (true) ? row : -1; }

    // outline drawn by a row painter for the hovered row
    static void paintOutline (juce::Graphics& g, const juce::Component& colourSource, int width, int height)
    {
        g.setColour (colourSource.findColour (A8Colours::accentDeep));
        g.drawRect (0, 0, width, height, 1);
    }

private:
    juce::ListBox& listBox;
    int row { -1 };

    void setRow (int newRow)
    {
        if (newRow == row)
            return;
        const auto oldRow { row };
        row = newRow;
        if (oldRow >= 0)
            listBox.repaintRow (oldRow);
        if (row >= 0)
            listBox.repaintRow (row);
    }

    void track (const juce::MouseEvent& mouseEvent)
    {
        const auto position { mouseEvent.getEventRelativeTo (&listBox).getPosition () };
        setRow (listBox.getLocalBounds ().contains (position) ? listBox.getRowContainingPosition (position.x, position.y) : -1);
    }

    void mouseMove (const juce::MouseEvent& mouseEvent) override  { track (mouseEvent); }
    void mouseEnter (const juce::MouseEvent& mouseEvent) override { track (mouseEvent); }
    void mouseDrag (const juce::MouseEvent& mouseEvent) override  { track (mouseEvent); }
    void mouseExit (const juce::MouseEvent& mouseEvent) override  { track (mouseEvent); }
};

/*
    The strip that names a pane - a title in the accent colour, an optional count
    on the right, and room for a few small buttons in between.
*/
class PaneHeader : public juce::Component
{
public:
    explicit PaneHeader (juce::String headerTitle)
        : title (std::move (headerTitle))
    {
    }

    static constexpr int kHeight { 28 };

    void setCountText (juce::String newCountText)
    {
        if (countText == newCountText)
            return;
        countText = std::move (newCountText);
        repaint ();
    }

    // the width this header needs to show its title, its count, and tools of the
    // given total width between them without squeezing any of them
    int getRequiredWidth (int toolsWidth) const
    {
        return titleRight () + toolsWidth + countWidth () + kPadding;
    }

    // where buttons may be placed, between the title and the count, in this
    // component's coordinates
    juce::Rectangle<int> getFreeBounds () const
    {
        const auto titleRight { this->titleRight () };
        const auto countWidth { this->countWidth () };
        return getLocalBounds ().withTrimmedLeft (titleRight)
                                .withTrimmedRight (kPadding + countWidth)
                                .withSizeKeepingCentre (getWidth () - titleRight - kPadding - countWidth, 19);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (findColour (A8Colours::panelHeader));
        g.setColour (findColour (A8Colours::outline));
        g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));

        auto bounds { getLocalBounds ().reduced (kPadding, 0).withTrimmedBottom (1) };
        g.setFont (A8Type::paneTitle ());
        g.setColour (findColour (A8Colours::accentText));
        g.drawText (title, bounds, juce::Justification::centredLeft, false);

        if (countText.isNotEmpty ())
        {
            g.setFont (A8Type::count ());
            g.setColour (findColour (A8Colours::textGhost));
            g.drawText (countText, bounds, juce::Justification::centredRight, false);
        }
    }

private:
    static constexpr int kPadding { 8 };
    juce::String title;
    juce::String countText;

    int titleRight () const { return kPadding + A8Paint::textWidth (A8Type::paneTitle (), title) + 8; }
    // sized for the widest count the presets pane shows, so the header does not
    // change its mind about fitting as presets are found
    int countWidth () const { return countText.isEmpty () ? 0 : A8Paint::textWidth (A8Type::count (), "199/199") + 8; }
};

/*
    The small outlined buttons of the chrome. Two sizes of the same idea: a mini
    for the pane tools (OPTIONS, ALL), and a chip for the path bar (OUT,
    SETTINGS). A chip can carry a value after its label and a caret after that,
    which is how the output device reads as "OUT  Speakers (HD Audio)  v".

    A toggled on button takes the selection tint and the accent, so an on state
    is visible without hovering over it.
*/
class ChromeButton : public juce::TextButton
{
public:
    enum class Size { mini, chip };

    explicit ChromeButton (juce::String text, Size theSize = Size::mini)
        : size (theSize)
    {
        setButtonText (text);
    }

    void setValueText (juce::String newValueText)
    {
        valueText = std::move (newValueText);
        repaint ();
    }

    void setShowsCaret (bool shouldShowCaret)
    {
        showsCaret = shouldShowCaret;
        repaint ();
    }

    static constexpr int kMiniHeight { 19 };
    static constexpr int kChipHeight { 22 };

    int getIdealWidth () const
    {
        const auto padding { size == Size::mini ? 6 : 8 };
        auto width { padding * 2 + A8Paint::textWidth (labelFont (), getButtonText ()) };
        if (valueText.isNotEmpty ())
            width += kGap + A8Paint::textWidth (A8Type::chipValue (), valueText);
        if (showsCaret)
            width += kGap + kChipCaretWidth;
        return width + 2;
    }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        const auto area { getLocalBounds ().toFloat ().reduced (0.5f) };
        const auto enabled { isEnabled () };
        const auto on { getToggleState () };
        const auto hovered { enabled && (isMouseOver || isMouseDown) };

        g.setColour (findColour (on ? A8Colours::selectedRow
                                    : (size == Size::mini ? A8Colours::buttonBackground : A8Colours::panelHeader)));
        g.fillRoundedRectangle (area, 2.0f);
        g.setColour (findColour (on || hovered ? A8Colours::accentDeep : A8Colours::outline));
        g.drawRoundedRectangle (area, 2.0f, 1.0f);

        const auto labelColour { findColour (! enabled ? A8Colours::textGhost
                                                       : (on ? A8Colours::accentText
                                                             : (hovered ? A8Colours::text : A8Colours::textDim))) };

        if (valueText.isEmpty () && ! showsCaret)
        {
            g.setFont (labelFont ());
            g.setColour (labelColour);
            g.drawText (getButtonText (), getLocalBounds (), juce::Justification::centred, false);
            return;
        }

        auto content { getLocalBounds ().reduced (size == Size::mini ? 6 : 8, 0) };
        g.setFont (labelFont ());
        g.setColour (labelColour);
        const auto labelWidth { A8Paint::textWidth (labelFont (), getButtonText ()) };
        g.drawText (getButtonText (), content.removeFromLeft (labelWidth), juce::Justification::centredLeft, false);

        if (showsCaret)
        {
            const auto caretArea { content.removeFromRight (kChipCaretWidth) };
            g.setColour (findColour (A8Colours::menuHeaderText));
            A8Paint::caretDown (g, caretArea.toFloat ().getCentre (), static_cast<float> (kChipCaretWidth));
            content.removeFromRight (kGap);
        }

        if (valueText.isNotEmpty ())
        {
            content.removeFromLeft (kGap);
            g.setFont (A8Type::chipValue ());
            g.setColour (findColour (enabled ? A8Colours::text : A8Colours::textGhost));
            g.drawFittedText (valueText, content, juce::Justification::centredLeft, 1, 1.0f);
        }
    }

private:
    static constexpr int kGap { 6 };
    static constexpr int kChipCaretWidth { 6 };

    Size size;
    juce::String valueText;
    bool showsCaret { false };

    juce::Font labelFont () const { return size == Size::mini ? A8Type::mini () : A8Type::chip (); }
};

/*
    The editor's buttons: TOOLS, SAVE. A primary one
    takes the accent fill - that is kept for the single action most worth
    finding, and only while it has something to do.
*/
class ActionButton : public juce::TextButton
{
public:
    enum class Size { normal, small };

    explicit ActionButton (juce::String text, Size theSize = Size::normal)
        : size (theSize)
    {
        setButtonText (text);
    }

    void setPrimary (bool shouldBePrimary)
    {
        if (primary == shouldBePrimary)
            return;
        primary = shouldBePrimary;
        repaint ();
    }

    void setShowsCaret (bool shouldShowCaret)
    {
        showsCaret = shouldShowCaret;
        repaint ();
    }

    static constexpr int kNormalHeight { 25 };
    static constexpr int kSmallHeight { 23 };

    int getIdealWidth () const
    {
        auto width { (horizontalPadding () * 2) + A8Paint::textWidth (font (), getButtonText ()) };
        if (showsCaret)
            width += kCaretGap + kCaretWidth;
        return width + 2;
    }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        const auto area { getLocalBounds ().toFloat ().reduced (0.5f) };
        const auto enabled { isEnabled () };
        const auto hovered { enabled && (isMouseOver || isMouseDown) };
        const auto asPrimary { primary && enabled };

        if (asPrimary)
        {
            const auto glow { findColour (A8Colours::accentGlow) };
            if (! glow.isTransparent ())
            {
                juce::Path body;
                body.addRoundedRectangle (area, 2.0f);
                juce::DropShadow (glow, 12, {}).drawForPath (g, body);
            }
        }

        g.setColour (findColour (asPrimary ? A8Colours::accent : A8Colours::buttonBackground));
        g.fillRoundedRectangle (area, 2.0f);
        g.setColour (findColour (asPrimary ? (hovered ? A8Colours::accentInk : A8Colours::accentEdge)
                                           : (hovered ? A8Colours::accentDeep : A8Colours::outline)));
        g.drawRoundedRectangle (area, 2.0f, 1.0f);

        const auto textColour { findColour (asPrimary ? A8Colours::accentInk
                                                      : (! enabled ? A8Colours::textGhost
                                                                   : (hovered ? A8Colours::text : A8Colours::textDim))) };

        // label and caret are centred as one unit
        const auto labelWidth { A8Paint::textWidth (font (), getButtonText ()) };
        const auto contentWidth { labelWidth + (showsCaret ? kCaretGap + kCaretWidth : 0) };
        auto content { getLocalBounds ().withSizeKeepingCentre (contentWidth, getHeight ()) };

        g.setFont (font ());
        g.setColour (textColour);
        g.drawText (getButtonText (), content.removeFromLeft (labelWidth), juce::Justification::centredLeft, false);

        if (showsCaret)
        {
            content.removeFromLeft (kCaretGap);
            g.setColour (textColour.withMultipliedAlpha (0.7f));
            A8Paint::caretDown (g, content.toFloat ().getCentre ().translated (0.0f, 0.5f), static_cast<float> (kCaretWidth));
        }
    }

private:
    static constexpr int kCaretGap { 5 };
    static constexpr int kCaretWidth { 6 };

    Size size;
    bool primary { false };
    bool showsCaret { false };

    juce::Font font () const { return size == Size::normal ? A8Type::button () : A8Type::buttonSmall (); }
    int horizontalPadding () const { return size == Size::normal ? 13 : 10; }
};

/*
    A button that opens a menu, and says so with a caret after its label.
*/
class MenuButton : public ActionButton
{
public:
    explicit MenuButton (juce::String text, Size theSize = Size::normal)
        : ActionButton (std::move (text), theSize)
    {
        setShowsCaret (true);
    }
};

/*
    Audition buttons. At rest they show what they will do; while their sample is
    playing they take the accent fill - cyan means playing, here as on the
    playhead - and offer to stop.
*/
class TransportButton : public juce::TextButton
{
public:
    enum class Glyph { play, loop };

    TransportButton (juce::String text, Glyph theGlyph)
        : glyph (theGlyph)
    {
        setButtonText (text);
    }

    void setPlaying (bool nowPlaying)
    {
        if (playing == nowPlaying)
            return;
        playing = nowPlaying;
        repaint ();
    }
    bool isPlaying () const noexcept { return playing; }

    static constexpr int kHeight { 23 };

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        const auto enabled { isEnabled () };
        const auto hovered { enabled && (isMouseOver || isMouseDown) };
        const auto area { getLocalBounds ().toFloat ().reduced (0.5f) };

        // a control that cannot be used is dimmed as a whole
        g.beginTransparencyLayer (enabled ? 1.0f : 0.4f);

        if (playing)
        {
            const auto glow { findColour (A8Colours::accentGlow) };
            if (! glow.isTransparent ())
            {
                juce::Path body;
                body.addRoundedRectangle (area, 2.0f);
                juce::DropShadow (glow, 12, {}).drawForPath (g, body);
            }
        }

        g.setColour (findColour (playing ? A8Colours::accent : A8Colours::buttonBackground));
        g.fillRoundedRectangle (area, 2.0f);
        g.setColour (findColour (playing ? A8Colours::accentEdge
                                         : (hovered ? A8Colours::accentDeep : A8Colours::outline)));
        g.drawRoundedRectangle (area, 2.0f, 1.0f);

        const auto ink { findColour (playing ? A8Colours::accentInk
                                             : (hovered ? A8Colours::text : A8Colours::textDim)) };
        const auto label { playing ? juce::String ("STOP") : getButtonText () };
        const auto labelFont { A8Type::transport () };
        const auto labelWidth { A8Paint::textWidth (labelFont, label) };
        constexpr auto glyphSize { 8.0f };
        constexpr auto gap { 7 };
        auto content { getLocalBounds ().withSizeKeepingCentre (static_cast<int> (glyphSize) + gap + labelWidth, getHeight ()) };

        g.setColour (ink);
        const auto glyphBounds { content.removeFromLeft (static_cast<int> (glyphSize)).toFloat ().withSizeKeepingCentre (glyphSize, glyphSize) };
        paintGlyph (g, glyphBounds);
        content.removeFromLeft (gap);

        g.setFont (labelFont);
        g.drawText (label, content, juce::Justification::centredLeft, false);

        g.endTransparencyLayer ();
    }

private:
    Glyph glyph;
    bool playing { false };

    void paintGlyph (juce::Graphics& g, juce::Rectangle<float> bounds) const
    {
        if (playing)
        {
            g.fillRect (bounds.reduced (1.0f));
            return;
        }

        if (glyph == Glyph::play)
        {
            juce::Path triangle;
            triangle.addTriangle (bounds.getX () + 1.0f, bounds.getY (), bounds.getX () + 1.0f, bounds.getBottom (),
                                  bounds.getRight (), bounds.getCentreY ());
            g.fillPath (triangle);
            return;
        }

        // the clockwise open circle arrow, which Plex does carry
        g.setFont (A8Type::transport ().withPointHeight (11.0f));
        g.drawText (juce::String (juce::CharPointer_UTF8 ("\xe2\x86\xbb")), bounds.expanded (4.0f), juce::Justification::centred, false);
    }
};

/*
    Tabs that say at a glance which of them hold something: each carries a status
    led before its name, lit when its page has content (a channel or zone with a
    sample assigned).

    Along the top, the front tab takes the colour of the page under it, so the two
    read as one surface, and is underlined in the accent. Down the side, the tabs
    share the strip evenly and the front one is marked by an accent bar on the edge
    it shares with the page. A vertical tab may carry a second
    line - the tab name is split at a carriage return - set in the value face.

    TabbedComponent stores a background colour per tab rather than resolving one,
    so the page colour is pushed into every tab again whenever the palette changes.
*/
class LedTabbedComponent : public juce::TabbedComponent
{
public:
    explicit LedTabbedComponent (juce::TabbedButtonBar::Orientation orientation)
        : juce::TabbedComponent (orientation)
    {
        getTabbedButtonBar ().getProperties ().set (A8LnFProperties::tabsHaveLeds, true);
        setOutline (0);
        setIndent (0);
    }

    void addLedTab (const juce::String& tabName, juce::Component* contentComponent)
    {
        addTab (tabName, findColour (pageColourId), contentComponent, false);
    }

    // lights the tab's led when its page holds something
    void setTabHasContent (int tabIndex, bool tabHasContent)
    {
        if (auto* tabButton { dynamic_cast<LedTabBarButton*> (getTabbedButtonBar ().getTabButton (tabIndex)) })
            tabButton->setHasContent (tabHasContent);
    }

    // the colour a lit led takes; green, for content, unless set otherwise
    void setLedColourId (int newLedColourId)
    {
        ledColourId = newLedColourId;
        repaint ();
    }

    // leaves out the divider between a tab and the one after it, so the two read as a pair
    void setTabJoinedToNext (int tabIndex, bool joinedToNext)
    {
        if (auto* tabButton { dynamic_cast<LedTabBarButton*> (getTabbedButtonBar ().getTabButton (tabIndex)) })
            tabButton->setJoinedToNext (joinedToNext);
    }

    // the colour of the pages, and of the front tab that joins them
    void setPageColourId (int newPageColourId)
    {
        pageColourId = newPageColourId;
        lookAndFeelChanged ();
    }

    std::function<void (int tabIndex)> onSelectedTabChanged;
    // the ink of a vertical tab's second line, given its text; textDim when unset
    std::function<int (const juce::String& detailText)> detailColourIdFor;

protected:
    juce::TabBarButton* createTabButton (const juce::String& tabName, int /*tabIndex*/) override
    {
        return new LedTabBarButton (tabName, getTabbedButtonBar ());
    }

    void currentTabChanged (int newTabIndex, const juce::String& /*tabName*/) override
    {
        // the partner of a paired tab changes with it, and only the two toggled buttons repaint themselves
        getTabbedButtonBar ().repaint ();
        if (onSelectedTabChanged != nullptr)
            onSelectedTabChanged (newTabIndex);
    }

    void lookAndFeelChanged () override
    {
        juce::TabbedComponent::lookAndFeelChanged ();
        for (auto tabIndex { 0 }; tabIndex < getNumTabs (); ++tabIndex)
            setTabBackgroundColour (tabIndex, findColour (pageColourId));
    }

private:
    int pageColourId { A8Colours::windowBackground };
    int ledColourId { A8Colours::markerStart };

    class LedTabBarButton : public juce::TabBarButton
    {
    public:
        LedTabBarButton (const juce::String& tabName, juce::TabbedButtonBar& tabbedButtonBar)
            : juce::TabBarButton (tabName, tabbedButtonBar)
        {
        }

        void setHasContent (bool tabHasContent)
        {
            if (hasContent == tabHasContent)
                return;
            hasContent = tabHasContent;
            repaint ();
        }

        void setJoinedToNext (bool shouldBeJoinedToNext)
        {
            if (joinedToNext == shouldBeJoinedToNext)
                return;
            joinedToNext = shouldBeJoinedToNext;
            repaint ();
        }

    private:
        bool hasContent { false };
        bool joinedToNext { false };

        void paintButton (juce::Graphics& g, bool isMouseOver, bool /*isMouseDown*/) override
        {
            const auto bounds { getLocalBounds () };
            const auto selected { getToggleState () };
            const auto hovered { isMouseOver && ! selected && isEnabled () };
            const auto vertical { getTabbedButtonBar ().isVertical () };
            auto* tabbedComponent { findParentComponentOfClass<LedTabbedComponent> () };

            // the two tabs of a pair read as one: when either is at the front, both take the page colour
            auto& buttonBar { getTabbedButtonBar () };
            const auto* nextTab { dynamic_cast<LedTabBarButton*> (buttonBar.getTabButton (getIndex () + 1)) };
            const auto* previousTab { dynamic_cast<LedTabBarButton*> (buttonBar.getTabButton (getIndex () - 1)) };
            const auto pairAtFront { (joinedToNext && nextTab != nullptr && nextTab->getToggleState ())
                                     || (previousTab != nullptr && previousTab->joinedToNext && previousTab->getToggleState ()) };

            if (selected || pairAtFront)
                g.fillAll (findColour (tabbedComponent != nullptr ? tabbedComponent->pageColourId : static_cast<int> (A8Colours::windowBackground)));
            else if (hovered)
                g.fillAll (findColour (A8Colours::panelHeader));

            // a hairline between neighbouring tabs, as strong as the line along the strip, unless the two are a pair
            g.setColour (findColour (A8Colours::outline));
            if (vertical)
                g.fillRect (bounds.withTop (bounds.getBottom () - 1));
            else if (! joinedToNext)
                g.fillRect (bounds.withLeft (bounds.getRight () - 1));
            const auto litColourId { tabbedComponent != nullptr ? tabbedComponent->ledColourId : static_cast<int> (A8Colours::markerStart) };

            const auto nameColour { findColour (! isEnabled () ? A8Colours::textGhost
                                                               : (selected ? A8Colours::text
                                                                           : (hovered ? A8Colours::textDim : A8Colours::menuHeaderText))) };
            const auto tabText { getButtonText ().trim () };
            const auto name { tabText.upToFirstOccurrenceOf ("\r", false, false) };
            const auto detail { tabText.fromFirstOccurrenceOf ("\r", false, false) };
            const auto nameFont { A8Type::channelTab () };

            if (vertical)
            {
                // led and name centred as one unit in the upper part, the detail under them
                const auto hasDetail { detail.isNotEmpty () };
                auto content { bounds.withTrimmedBottom (1).withTrimmedRight (2) };
                auto nameRow { hasDetail ? content.removeFromTop (content.getHeight () / 2 + 2).withTrimmedTop (2) : content };
                const auto nameWidth { A8Paint::textWidth (nameFont, name) };
                const auto rowWidth { static_cast<int> (StatusLed::kDiameter) + 5 + nameWidth };
                auto unit { nameRow.withSizeKeepingCentre (rowWidth, nameRow.getHeight ()) };
                const auto ledBounds { unit.removeFromLeft (static_cast<int> (StatusLed::kDiameter)).toFloat ()
                                           .withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
                StatusLed::draw (g, ledBounds, hasContent, *this, litColourId);
                unit.removeFromLeft (5);
                g.setFont (nameFont);
                g.setColour (nameColour);
                g.drawText (name, unit, juce::Justification::centredLeft, false);

                if (hasDetail)
                {
                    const auto detailColourId { tabbedComponent != nullptr && tabbedComponent->detailColourIdFor != nullptr ? tabbedComponent->detailColourIdFor (detail)
                                                                                                          : static_cast<int> (A8Colours::textDim) };
                    g.setFont (A8Type::cvValue ().withPointHeight (9.5f));
                    g.setColour (findColour (isEnabled () ? detailColourId : static_cast<int> (A8Colours::textGhost)));
                    g.drawFittedText (detail, content.withTrimmedBottom (2), juce::Justification::centredTop, 1, 0.8f);
                }

                if (selected)
                {
                    g.setColour (findColour (A8Colours::accent));
                    g.fillRect (bounds.withLeft (bounds.getRight () - 2));
                }
                else if (hovered)
                {
                    g.setColour (findColour (A8Colours::accentDeep));
                    g.drawRect (bounds.withTrimmedBottom (1), 1);
                }
                return;
            }

            auto content { bounds.withTrimmedLeft (13) };
            const auto ledBounds { content.removeFromLeft (static_cast<int> (StatusLed::kDiameter)).toFloat ()
                                          .withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
            StatusLed::draw (g, ledBounds, hasContent, *this, litColourId);
            content.removeFromLeft (7);

            g.setFont (nameFont);
            g.setColour (nameColour);
            g.drawText (name, content, juce::Justification::centredLeft, false);

            if (selected)
            {
                g.setColour (findColour (A8Colours::accent));
                g.fillRect (bounds.withTop (bounds.getBottom () - 2));
            }
            else if (hovered)
            {
                g.setColour (findColour (A8Colours::accentDeep));
                g.drawRect (bounds.withTrimmedBottom (1), 1);
            }
        }
    };
};
