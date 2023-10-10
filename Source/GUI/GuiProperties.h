#pragma once

#include <JuceHeader.h>
#include "../Utility/ValueTreeWrapper.h"

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

    std::tuple<int,int> getPosition ();
    std::tuple<int, int> getSize ();
    std::tuple<int, int, int> getPaneSizes ();

    std::function<void (int x, int y)> onPositionChange;
    std::function<void (int width, int height)> onSizeChange;
    std::function<void (int, int, int)> onPaneSizeChange;

    static inline const juce::Identifier GuiTypeId { "GUI" };
    static inline const juce::Identifier PositionPropertyId  { "position" };
    static inline const juce::Identifier SizePropertyId      { "size" };
    static inline const juce::Identifier PaneSizesPropertyId { "paneSizes" };

    void initValueTree ();
    void processValueTree ();

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
