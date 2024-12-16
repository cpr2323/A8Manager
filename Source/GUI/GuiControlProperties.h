#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

class GuiControlProperties : public ValueTreeWrapper<GuiControlProperties>
{
public:
    GuiControlProperties () noexcept : ValueTreeWrapper<GuiControlProperties> (GuiTypeId)
    {
    }
    GuiControlProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<GuiControlProperties> (GuiTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void showMidiConfigWindow (bool show);

    bool getShowMidiConfigWindow ();

    std::function<void (bool)> onShowMidiConfigWindowChange;

    static inline const juce::Identifier GuiTypeId { "GUIControl" };
    static inline const juce::Identifier ShowMidiConfigWindowPropertyId { "showMidiConfigWindow" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
};
