#include "CustomTextButton.h"

void CustomTextButton::mouseDown (const juce::MouseEvent& mouseEvent)
{
    if (! mouseEvent.mods.isPopupMenu ())
    {
        juce::TextButton::mouseDown (mouseEvent);
        return;
    }
    if (onPopupMenuCallback != nullptr)
        onPopupMenuCallback ();
}
