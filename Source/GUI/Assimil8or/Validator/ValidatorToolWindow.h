#pragma once

#include <JuceHeader.h>
#include "ValidatorComponentProperties.h"
#include "../../Theme/UiComponents.h"

class ValidatorToolWindow : public juce::Component
{
public:
    ValidatorToolWindow ();
    void init (juce::ValueTree rootPropertiesVT);

private:
    ValidatorComponentProperties validatorComponentProperties;

    PaneHeader paneHeader { "VALIDATOR" };
    ChromeButton convertAllButton { "CONVERT ALL" };
    ChromeButton locateAllButton { "LOCATE ALL" };
    ChromeButton renameAllButton { "RENAME ALL" };
    ChromeButton viewInfoButton { "INFO" };
    ChromeButton viewWarningButton { "WARNING" };
    ChromeButton viewErrorButton { "ERROR" };

    void paint (juce::Graphics& g) override;
    void resized () override;
};
