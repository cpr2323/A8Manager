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

    void setFilterInfo (bool shouldFilter, bool includeSelfCallback);
    void setFilterWarning (bool shouldFilter, bool includeSelfCallback);
    void setFilterError (bool shouldFilter, bool includeSelfCallback);

    bool getFilterInfo ();
    bool getFilterWarning ();
    bool getFilterError ();

    std::function<void (bool shouldFilter)> onFilterInfoChange;
    std::function<void (bool shouldFilter)> onFilterWarningChange;
    std::function<void (bool shouldFilter)> onFilterErrorChange;

    static inline const juce::Identifier ValidatorComponentTypeId { "ValidatorComponent" };
    static inline const juce::Identifier FilterInfoPropertyId    { "filterInfo" };
    static inline const juce::Identifier FilterWarningPropertyId { "filterWarning" };
    static inline const juce::Identifier FilterErrorPropertyId   { "filterError" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
