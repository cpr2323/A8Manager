#pragma once

#include <JuceHeader.h>
#include "ValueTreeWrapper.h"

// A ValueTreeWrapper for properties that are not saved to the properties file
class RuntimeRootProperties : public ValueTreeWrapper<RuntimeRootProperties>
{
public:
    enum class QuitState
    {
        idle,
        now,
        cancel
    };

    RuntimeRootProperties () noexcept : ValueTreeWrapper<RuntimeRootProperties> (RuntimeRootPropertiesId) {}

    juce::ValueTree addSection (juce::Identifier sectionType);
    bool removeSection (juce::Identifier sectionType);
    juce::ValueTree getSection (juce::Identifier sectionType);

    void setAppDataPath (juce::String appDataPath, bool includeSelfCallback = false);
    void setAppVersion (juce::String appVersion, bool includeSelfCallback = false);
    void setLayout (juce::String layout, bool includeSelfCallback = false);
    void setPreferredQuitState (QuitState preferredQuitState, bool includeSelfCallback = false);
    void setQuitState (QuitState newQuitState, bool includeSelfCallback = false);
    void triggerWindowMoved (bool includeSelfCallback = false);
    void triggerMinimiseButtonPressed (bool includeSelfCallback = false);
    void triggerMaximiseButtonPressed (bool includeSelfCallback = false);
    void triggerAppSuspended (bool includeSelfCallback = false);
    void triggerAppResumed (bool includeSelfCallback = false);
    void triggerSystemRequestedQuit (bool includeSelfCallback = false);

    juce::String getAppDataPath () noexcept;
    juce::String getAppVersion () noexcept;
    juce::String getLayout () noexcept;
    const QuitState getQuitState ();
    const QuitState getPreferredQuitState ();

    std::function<void ()> onAppSuspended;
    std::function<void ()> onAppResumed;
    std::function<void (juce::String layout)> onLayoutChange;
    std::function<void ()> onMinimiseButtonPressed;
    std::function<void ()> onMaximiseButtonPressed;
    std::function<void (QuitState quitState)> onQuitStateChanged;
    std::function<void (bool enabled)> onOpenGlDisable;
    std::function<void ()> onSystemRequestedQuit;
    std::function<void ()> onWindowMoved;
    // there is no onPreferredQuitStateChanged call because it doesn't cause anything to happen, just a change in state

    static inline const juce::Identifier RuntimeRootPropertiesId  { "RuntimeRoot" };
    static inline const juce::Identifier AppDataPathPropertyId    { "appDataPath" };
    static inline const juce::Identifier AppResumedId             { "appResumed" };
    static inline const juce::Identifier AppSuspendedId           { "appSuspended" };
    static inline const juce::Identifier AppVersionPropertyId     { "appVersion" };
    static inline const juce::Identifier LayoutPropertyId         { "layout" };
    static inline const juce::Identifier MinimiseButtonPressedId  { "minimisedButtonPressed" };
    static inline const juce::Identifier MaximiseButtonPressedId  { "maximiseButtonPressed" };
    static inline const juce::Identifier PreferredQuitStatePropertyId { "preferredQuitState" };
    static inline const juce::Identifier QuitStatePropertyId      { "quitState" };
    static inline const juce::Identifier SystemRequestedQuitId    { "systemRequestedQuit" };
    static inline const juce::Identifier WindowMovedId            { "windowMoved" };

    static inline const juce::String LayoutUnknown    { "unknown" };
    static inline const juce::String LayoutFullscreen { "fullscreen" };
    static inline const juce::String LayoutPortrait   { "portrait" };
    static inline const juce::String LayoutLandscape  { "landscape" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
};
