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
    void setPaneSizes (double pane1Size, double pane2Size, double pane3Size, bool includeSelfCallback);

    std::tuple<int,int> getPosition ();
    std::tuple<int, int> getSize ();
    std::tuple<double, double, double> getPaneSizes ();

    std::function<void (int x, int y)> onPositionChange;
    std::function<void (int width, int height)> onSizeChange;

    static inline const juce::Identifier GuiTypeId { "GUI" };
    static inline const juce::Identifier PositionPropertyId { "position" };
    static inline const juce::Identifier SizePropertyId     { "size" };
    static inline const juce::Identifier PaneSizesPropertyId { "paneSizes" };

    void initValueTree ();
    void processValueTree ();

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
