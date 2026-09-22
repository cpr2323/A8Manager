#pragma once

#include <JuceHeader.h>
#include "oolib/ValueTree/ValueTreeWrapper.h"

class GuiProperties : public ValueTreeWrapper<GuiProperties>
{
public:
    GuiProperties () noexcept : ValueTreeWrapper<GuiProperties> (GuiTypeId)
    {
    }
    GuiProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<GuiProperties> (GuiTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setPosition (int x, int y, bool includeSelfCallback);
    void setSize (int width, int height, bool includeSelfCallback);
    void setPaneSizes (int pane1Size, int pane2Size, int pane3Size, bool includeSelfCallback);
    // 0.0 is the near-black scheme, 1.0 the white one
    void setBackgroundLevel (float backgroundLevel, bool includeSelfCallback);
    // a request for the settings window; whoever shows it listens for this
    void showSettingsDialog (bool includeSelfCallback);
    // the settings page last looked at, so the window reopens where it was left
    void setSettingsTabName (juce::String tabName, bool includeSelfCallback);

    std::tuple<int,int> getPosition ();
    std::tuple<int, int> getSize ();
    std::tuple<int, int, int> getPaneSizes ();
    float getBackgroundLevel ();
    juce::String getSettingsTabName ();

    std::function<void (float backgroundLevel)> onBackgroundLevelChange;
    std::function<void ()> onShowSettingsDialog;

    static inline const juce::Identifier GuiTypeId { "GUI" };
    static inline const juce::Identifier PositionPropertyId           { "position" };
    static inline const juce::Identifier SizePropertyId               { "size" };
    static inline const juce::Identifier PaneSizesPropertyId          { "paneSizes" };
    static inline const juce::Identifier BackgroundLevelPropertyId    { "backgroundLevel" };
    static inline const juce::Identifier ShowSettingsDialogPropertyId { "showSettingsDialog" };
    static inline const juce::Identifier SettingsTabNamePropertyId    { "settingsTabName" };

    void initValueTree ();
    void processValueTree ();

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
