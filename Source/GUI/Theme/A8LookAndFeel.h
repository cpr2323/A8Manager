#pragma once

#include <JuceHeader.h>
#include "Palette.h"

/*
    Tags a component can set on itself to ask for a drawing variation, rather than
    being given a LookAndFeel of its own.

    A component with its own LookAndFeel resolves every colour through that object,
    so a second LookAndFeel is also a second palette - and one that nothing updates.
    Keeping the variations here means there is exactly one LookAndFeel in the app,
    and therefore exactly one set of colours.
*/
namespace A8LnFProperties
{
    // a unit drawn after a combo box's value, as the rate reads "44 kHz"
    static inline const juce::Identifier valueUnit { "valueUnit" };
    // set on a combo box too narrow for a caret, such as a CV input: the value
    // takes the whole field, placed by the box's own justification
    static inline const juce::Identifier noCaret { "noCaret" };
    // set on a TabbedButtonBar whose tabs carry a status led before their name
    static inline const juce::Identifier tabsHaveLeds { "tabsHaveLeds" };
}

/*
    Installs a Palette into JUCE's colour lookup, and the app's type into its font
    lookup.

    ThemeController sets this as the default LookAndFeel, so every component in
    the app - including popup menus and dialogs, which have no parent to inherit
    from - resolves its colours and default typeface through here.
*/
class A8LookAndFeel : public juce::LookAndFeel_V4
{
public:
    A8LookAndFeel ();

    void setBackground (float newBackground);
    float getBackground () const noexcept { return palette.getBackground (); }

    const Palette& getPalette () const noexcept { return palette; }

    // value fields
    void fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
    void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

    // buttons that do not paint themselves: dialogs, alert windows
    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    // tabs
    int getTabButtonBestWidth (juce::TabBarButton& button, int tabDepth) override;
    int getTabButtonOverlap (int tabDepth) override;
    void drawTabbedButtonBarBackground (juce::TabbedButtonBar& buttonBar, juce::Graphics& g) override;
    void drawTabAreaBehindFrontButton (juce::TabbedButtonBar& buttonBar, juce::Graphics& g, int width, int height) override;

    // table headers: a panel header strip, with the column names in the chrome face
    void drawTableHeaderBackground (juce::Graphics& g, juce::TableHeaderComponent& header) override;
    void drawTableHeaderColumn (juce::Graphics& g, juce::TableHeaderComponent& header, const juce::String& columnName, int columnId,
                                int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override;

    // menus
    juce::Font getPopupMenuFont () override;
    // as V4's, but a ticked item is marked by a lit green led, as the tabs and lists mark content
    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                            bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour) override;
    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName) override;
    void getIdealPopupMenuSectionHeaderSizeWithOptions (const juce::String& text, int standardMenuItemHeight,
                                                        int& idealWidth, int& idealHeight,
                                                        const juce::PopupMenu::Options& options) override;

    // tooltips: square, since the window is opaque, with a rim that stands off the background
    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    void drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height) override;

    // scrollbars: a thin rounded thumb and no track
    int getDefaultScrollbarWidth () override;
    void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;

private:
    void applyPalette ();

    Palette palette;
};
