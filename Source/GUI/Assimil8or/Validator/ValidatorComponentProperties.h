#pragma once

#include <JuceHeader.h>
#include "../../../Utility/ValueTreeWrapper.h"

class ValidatorComponentProperties : public ValueTreeWrapper<ValidatorComponentProperties>
{
public:
    ValidatorComponentProperties () noexcept : ValueTreeWrapper<ValidatorComponentProperties> (ValidatorComponentTypeId)
    {
    }
    ValidatorComponentProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<ValidatorComponentProperties> (ValidatorComponentTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setViewInfo (bool shouldView, bool includeSelfCallback);
    void setViewWarning (bool shouldView, bool includeSelfCallback);
    void setViewError (bool shouldView, bool includeSelfCallback);
    void enableRenameAll (bool enabled, bool includeSelfCallback);
    void triggerRenameAll (bool includeSelfCallback);

    bool getViewInfo ();
    bool getViewWarning ();
    bool getViewError ();
    bool getEnabledRenameAll ();

    std::function<void (bool shouldFilter)> onViewInfoChange;
    std::function<void (bool shouldFilter)> onViewWarningChange;
    std::function<void (bool shouldFilter)> onViewErrorChange;
    std::function<void (bool enabled)> onEnableRenameAllChange;
    std::function<void ()> onRenameAllTrigger;

    static inline const juce::Identifier ValidatorComponentTypeId { "ValidatorComponent" };
    static inline const juce::Identifier ViewInfoPropertyId        { "viewInfo" };
    static inline const juce::Identifier ViewWarningPropertyId     { "viewWarning" };
    static inline const juce::Identifier ViewErrorPropertyId       { "viewError" };
    static inline const juce::Identifier EnableRenameAllPropertyId { "enableRenameAll" };
    static inline const juce::Identifier RenameAllPropertyId       { "renameAll" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
