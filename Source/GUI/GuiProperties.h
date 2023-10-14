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

    static inline const juce::Identifier GuiTypeId { "GUI" };
    static inline const juce::Identifier PositionPropertyId  { "position" };
    static inline const juce::Identifier SizePropertyId      { "size" };
    static inline const juce::Identifier PaneSizesPropertyId { "paneSizes" };

    void initValueTree ();
    void processValueTree ();

private:
};
