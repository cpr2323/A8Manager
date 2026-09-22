#include "A8LookAndFeel.h"
#include "A8Fonts.h"
#include "UiComponents.h"

namespace
{
    constexpr auto kFieldCornerSize { 2.0f };
    constexpr auto kFieldPadding { 7 };
    constexpr auto kComboCaretWidth { 7.0f };

    // A field shows the pointer the same way every other control does.
    bool isHovered (const juce::Component& component)
    {
        return component.isEnabled () && component.isMouseOver (true);
    }

    constexpr auto kTooltipMaxWidth { 400.0f };
    constexpr auto kTooltipPaddingX { 8 };
    constexpr auto kTooltipPaddingY { 5 };

    juce::TextLayout layoutTooltip (const juce::String& text, juce::Colour colour)
    {
        juce::AttributedString attributedText;
        attributedText.setJustification (juce::Justification::centredLeft);
        attributedText.append (text, A8Type::body (), colour);

        juce::TextLayout layout;
        layout.createLayoutWithBalancedLineLengths (attributedText, kTooltipMaxWidth);
        return layout;
    }
}

A8LookAndFeel::A8LookAndFeel ()
{
    applyPalette ();
    // anything that does not ask for a font of its own is set in Plex Sans
    setDefaultSansSerifTypeface (A8Fonts::getDefaultTypeface ());
}

void A8LookAndFeel::setBackground (float newBackground)
{
    palette.setBackground (newBackground);
    applyPalette ();
}

void A8LookAndFeel::applyPalette ()
{
    for (const auto& [colourId, colour] : palette.getColours ())
        setColour (colourId, colour);
}

//==============================================================================
void A8LookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    // an unusable field sinks into the panel rather than staying recessed
    const auto background { textEditor.isEnabled () ? textEditor.findColour (juce::TextEditor::backgroundColourId)
                                                    : textEditor.findColour (A8Colours::listBackground) };
    g.setColour (background);
    g.fillRoundedRectangle (juce::Rectangle<int> { 0, 0, width, height }.toFloat (), kFieldCornerSize);
}

void A8LookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    const auto colourId { ! textEditor.isEnabled () ? A8Colours::outlineDim
                                                    : (textEditor.hasKeyboardFocus (true) || isHovered (textEditor)
                                                          ? A8Colours::accentDeep
                                                          : A8Colours::outline) };
    g.setColour (textEditor.findColour (colourId));
    g.drawRoundedRectangle (juce::Rectangle<int> { 0, 0, width, height }.toFloat ().reduced (0.5f), kFieldCornerSize, 1.0f);
}

void A8LookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto area { juce::Rectangle<int> { 0, 0, width, height }.toFloat () };
    const auto enabled { box.isEnabled () };
    const auto hovered { isHovered (box) };

    g.setColour (box.findColour (enabled ? juce::ComboBox::backgroundColourId : A8Colours::listBackground));
    g.fillRoundedRectangle (area, kFieldCornerSize);

    const auto outlineId { ! enabled ? A8Colours::outlineDim
                                     : (hovered || box.isPopupActive () ? A8Colours::accentDeep
                                                                        : A8Colours::outline) };
    g.setColour (box.findColour (outlineId));
    g.drawRoundedRectangle (area.reduced (0.5f), kFieldCornerSize, 1.0f);

    // a unit, such as kHz, sits just after the value
    if (const auto unit { box.getProperties () [A8LnFProperties::valueUnit].toString () }; unit.isNotEmpty ())
    {
        const auto valueWidth { A8Paint::textWidth (getComboBoxFont (box), box.getText ()) };
        g.setFont (A8Type::unit ());
        g.setColour (box.findColour (A8Colours::textGhost));
        g.drawText (unit, juce::Rectangle<int> { kFieldPadding + valueWidth + 3, 0, width, height },
                    juce::Justification::centredLeft, false);
    }

    if (box.getProperties ().contains (A8LnFProperties::noCaret))
        return;

    g.setColour (box.findColour (hovered ? A8Colours::accent : A8Colours::menuHeaderText));
    A8Paint::caretDown (g, { static_cast<float> (width) - 5.0f - (kComboCaretWidth * 0.5f), (static_cast<float> (height) * 0.5f) + 0.5f },
                           kComboCaretWidth);
}

void A8LookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBorderSize ({ 0, 0, 0, 0 });
    if (box.getProperties ().contains (A8LnFProperties::noCaret))
    {
        label.setBounds (box.getLocalBounds ().reduced (2, 0));
        label.setJustificationType (box.getJustificationType ());
        label.setFont (getComboBoxFont (box));
        return;
    }

    label.setBounds (kFieldPadding, 0, box.getWidth () - kFieldPadding - 5 - static_cast<int> (kComboCaretWidth) - 2, box.getHeight ());
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (getComboBoxFont (box));
}

juce::Font A8LookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return A8Type::value ();
}

//==============================================================================
juce::Font A8LookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return A8Type::button ();
}

void A8LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const auto area { button.getLocalBounds ().toFloat ().reduced (0.5f) };
    const auto hovered { button.isEnabled () && (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown) };

    g.setColour (backgroundColour);
    g.fillRoundedRectangle (area, kFieldCornerSize);
    g.setColour (button.findColour (hovered ? A8Colours::accentDeep : A8Colours::outline));
    g.drawRoundedRectangle (area, kFieldCornerSize, 1.0f);
}

//==============================================================================
int A8LookAndFeel::getTabButtonBestWidth (juce::TabBarButton& button, int tabDepth)
{
    auto& buttonBar { button.getTabbedButtonBar () };
    if (! buttonBar.getProperties ().contains (A8LnFProperties::tabsHaveLeds))
        return juce::LookAndFeel_V4::getTabButtonBestWidth (button, tabDepth);

    // a vertical strip is shared out evenly, so the tabs fill it top to bottom
    if (buttonBar.isVertical ())
        return buttonBar.getHeight () / std::max (1, buttonBar.getNumTabs ());

    // padding, led, gap, name, padding
    return 13 + static_cast<int> (StatusLed::kDiameter) + 7 + A8Paint::textWidth (A8Type::channelTab (), button.getButtonText ()) + 13;
}

int A8LookAndFeel::getTabButtonOverlap (int)
{
    // tabs sit edge to edge, divided by their own hairline
    return 0;
}

void A8LookAndFeel::drawTabbedButtonBarBackground (juce::TabbedButtonBar& buttonBar, juce::Graphics& g)
{
    g.fillAll (buttonBar.findColour (A8Colours::tabBackground));
    g.setColour (buttonBar.findColour (A8Colours::outline));
    g.drawHorizontalLine (buttonBar.getHeight () - 1, 0.0f, static_cast<float> (buttonBar.getWidth ()));
}

void A8LookAndFeel::drawTabAreaBehindFrontButton (juce::TabbedButtonBar&, juce::Graphics&, int, int)
{
    // the front tab marks itself with its own underline, so there is no shadow behind it
}

//==============================================================================
void A8LookAndFeel::drawTableHeaderBackground (juce::Graphics& g, juce::TableHeaderComponent& header)
{
    const auto area { header.getLocalBounds () };
    g.setColour (header.findColour (A8Colours::panelHeader));
    g.fillRect (area);
    g.setColour (header.findColour (A8Colours::outline));
    g.fillRect (area.withTop (area.getBottom () - 1));

    // a hairline after every column, as the cells below have
    g.setColour (header.findColour (A8Colours::outlineDim));
    for (auto columnIndex { header.getNumColumns (true) }; --columnIndex >= 0;)
        g.fillRect (header.getColumnPosition (columnIndex).removeFromRight (1));
}

void A8LookAndFeel::drawTableHeaderColumn (juce::Graphics& g, juce::TableHeaderComponent& header, const juce::String& columnName,
                                           int /*columnId*/, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags)
{
    if (isMouseDown || isMouseOver)
    {
        g.setColour (header.findColour (A8Colours::hoverBackground));
        g.fillRect (0, 0, width - 1, height - 1);
    }

    auto area { juce::Rectangle<int> (width, height).reduced (8, 0) };
    if ((columnFlags & (juce::TableHeaderComponent::sortedForwards | juce::TableHeaderComponent::sortedBackwards)) != 0)
    {
        juce::Path sortArrow;
        sortArrow.addTriangle (0.0f, 0.0f, 0.5f, (columnFlags & juce::TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f, 1.0f, 0.0f);
        g.setColour (header.findColour (A8Colours::menuHeaderText));
        g.fillPath (sortArrow, sortArrow.getTransformToScaleToFit (area.removeFromRight (height / 2).reduced (2).toFloat (), true));
    }

    g.setColour (header.findColour (A8Colours::menuHeaderText));
    // set as given: the message column carries its item counts in its name
    g.setFont (A8Type::chip ());
    g.drawFittedText (columnName, area, juce::Justification::centredLeft, 1);
}

//==============================================================================
juce::Font A8LookAndFeel::getPopupMenuFont ()
{
    return A8Type::body ();
}

void A8LookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                                       bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                                       const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour)
{
    // everything but the tick is V4's, so let it draw the item with the tick left off
    juce::LookAndFeel_V4::drawPopupMenuItem (g, area, isSeparator, isActive, isHighlighted, false, hasSubMenu,
                                             text, shortcutKeyText, icon, textColour);
    if (isSeparator || ! isTicked || icon != nullptr)
        return;

    // V4 keeps a square the height of the text at the left of the item for the tick; the led sits in it
    const auto itemArea { area.reduced (1).reduced (std::min (5, area.getWidth () / 20), 0) };
    const auto iconSize { juce::roundToInt (static_cast<float> (itemArea.getHeight ()) / 1.3f) };
    const auto ledBounds { itemArea.withWidth (iconSize).toFloat ().withSizeKeepingCentre (StatusLed::kDiameter, StatusLed::kDiameter) };
    const auto litColour { findColour (A8Colours::markerStart).withMultipliedAlpha (isActive ? 1.0f : 0.5f) };
    if (const auto glowAmount { findColour (A8Colours::ledGlow).getFloatAlpha () }; glowAmount > 0.0f)
    {
        juce::Path dot;
        dot.addEllipse (ledBounds);
        juce::DropShadow (litColour.withAlpha (glowAmount), 6, {}).drawForPath (g, dot);
    }
    g.setColour (litColour);
    g.fillEllipse (ledBounds);
}

void A8LookAndFeel::drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName)
{
    g.setFont (A8Type::menuSectionHeader ());
    g.setColour (findColour (juce::PopupMenu::headerTextColourId));
    // sits slightly low in its row, so it reads as belonging to the items below it
    g.drawText (sectionName.toUpperCase (), area.reduced (11, 0).withTrimmedTop (3), juce::Justification::centredLeft, true);
}

void A8LookAndFeel::getIdealPopupMenuSectionHeaderSizeWithOptions (const juce::String& text, int,
                                                                      int& idealWidth, int& idealHeight,
                                                                      const juce::PopupMenu::Options&)
{
    // the default is one and a half item heights, which leaves a header floating
    // in empty space; the header text plus a little room is enough
    const auto font { A8Type::menuSectionHeader () };
    idealHeight = juce::roundToInt (font.getHeight ()) + 9;
    idealWidth = A8Paint::textWidth (font, text.toUpperCase ()) + 22;
}

//==============================================================================
juce::Rectangle<int> A8LookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const auto layout { layoutTooltip (tipText, juce::Colours::black) };
    const auto width { juce::roundToInt (std::ceil (layout.getWidth ())) + (kTooltipPaddingX * 2) };
    const auto height { juce::roundToInt (std::ceil (layout.getHeight ())) + (kTooltipPaddingY * 2) };

    // placed away from the pointer, on whichever side of it there is more room
    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX () ? screenPos.x - (width + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY () ? screenPos.y - (height + 6) : screenPos.y + 6,
                                 width, height)
               .constrainedWithin (parentArea);
}

void A8LookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    const juce::Rectangle<int> bounds { 0, 0, width, height };
    g.fillAll (findColour (juce::TooltipWindow::backgroundColourId));
    g.setColour (findColour (juce::TooltipWindow::outlineColourId));
    g.drawRect (bounds, 1);

    layoutTooltip (text, findColour (juce::TooltipWindow::textColourId))
        .draw (g, bounds.reduced (kTooltipPaddingX, kTooltipPaddingY).toFloat ());
}

//==============================================================================
int A8LookAndFeel::getDefaultScrollbarWidth ()
{
    return 8;
}

void A8LookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height,
                                      bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                      bool isMouseOver, bool isMouseDown)
{
    juce::Rectangle<int> thumbBounds { x, y, width, height };
    thumbBounds = isScrollbarVertical ? thumbBounds.withY (thumbStartPosition).withHeight (thumbSize)
                                      : thumbBounds.withX (thumbStartPosition).withWidth (thumbSize);
    const auto thumb { thumbBounds.toFloat ().reduced (1.0f) };

    g.setColour (scrollbar.findColour (isMouseOver || isMouseDown ? A8Colours::accentDeep : A8Colours::outlineStrong));
    g.fillRoundedRectangle (thumb, std::min (thumb.getWidth (), thumb.getHeight ()) * 0.5f);
}
