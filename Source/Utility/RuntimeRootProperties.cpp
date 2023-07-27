#include "RuntimeRootProperties.h"

void RuntimeRootProperties::initValueTree ()
{
    setAppDataPath ("");
    setAppVersion ("0.0.0");
    setLayout (LayoutUnknown);
    setQuitState (QuitState::idle);
    triggerAppSuspended ();
    triggerAppResumed ();
    triggerMinimiseButtonPressed ();
    triggerMaximiseButtonPressed ();
    triggerSystemRequestedQuit ();
    triggerWindowMoved ();
}
juce::ValueTree RuntimeRootProperties::addSection (juce::Identifier sectionType)
{
    if (auto theSection { getSection (sectionType) }; theSection.isValid ())
        return theSection;

    auto newSection { juce::ValueTree (sectionType) };
    data.addChild (newSection, -1, nullptr);
    return newSection;
}

bool RuntimeRootProperties::removeSection (juce::Identifier sectionType)
{
    if (auto section { getSection (sectionType) }; section.isValid ())
    {
        data.removeChild (section, nullptr);
        return true;
    }
    return false; // child does not exist
}

juce::ValueTree RuntimeRootProperties::getSection (juce::Identifier sectionType)
{
    return data.getChildWithName (sectionType);
}

void RuntimeRootProperties::setAppDataPath (juce::String appDataPath, bool includeSelfCallback)
{
    setValue (appDataPath, AppDataPathPropertyId, includeSelfCallback);
}

void RuntimeRootProperties::setAppVersion (juce::String appVersion, bool includeSelfCallback)
{
    setValue (appVersion, AppVersionPropertyId, includeSelfCallback);
}

void RuntimeRootProperties::setLayout (juce::String layout, bool includeSelfCallback)
{
    setValue (layout, LayoutPropertyId, includeSelfCallback);
}

void RuntimeRootProperties::setPreferredQuitState (QuitState preferredQuitState, bool includeSelfCallback)
{
    setValue ((int) preferredQuitState, PreferredQuitStatePropertyId, includeSelfCallback);
}

void RuntimeRootProperties::setQuitState (QuitState newQuitState, bool includeSelfCallback)
{
    setValue ((int) newQuitState, QuitStatePropertyId, includeSelfCallback);
}

void RuntimeRootProperties::triggerMinimiseButtonPressed (bool includeSelfCallback)
{
     setValue (! getValue<bool> (MinimiseButtonPressedId), MinimiseButtonPressedId, includeSelfCallback);
}

void RuntimeRootProperties::triggerMaximiseButtonPressed (bool includeSelfCallback)
{
     setValue (! getValue<bool> (MaximiseButtonPressedId), MaximiseButtonPressedId, includeSelfCallback);
}

void RuntimeRootProperties::triggerWindowMoved (bool includeSelfCallback)
{
    setValue (! getValue<bool> (WindowMovedId), WindowMovedId, includeSelfCallback);
}

void RuntimeRootProperties::triggerAppSuspended (bool selfCallback)
{
    setValue (! getValue<bool> (AppSuspendedId), AppSuspendedId, selfCallback);
}

void RuntimeRootProperties::triggerAppResumed (bool selfCallback)
{
    setValue (! getValue<bool> (AppResumedId), AppResumedId, selfCallback);
}

void RuntimeRootProperties::triggerSystemRequestedQuit (bool selfCallback)
{
    setValue (! getValue<bool> (SystemRequestedQuitId), SystemRequestedQuitId, selfCallback);
}

juce::String RuntimeRootProperties::getAppDataPath () noexcept
{
    return getValue<juce::String> (AppDataPathPropertyId);
}

juce::String RuntimeRootProperties::getAppVersion () noexcept
{
    return getValue<juce::String> (AppVersionPropertyId);
}

juce::String RuntimeRootProperties::getLayout () noexcept
{
    return getValue<juce::String> (LayoutPropertyId);
}

const RuntimeRootProperties::QuitState RuntimeRootProperties::getPreferredQuitState ()
{
    return (RuntimeRootProperties::QuitState) getValue<int> (PreferredQuitStatePropertyId);
}

const RuntimeRootProperties::QuitState RuntimeRootProperties::getQuitState ()
{
    return (RuntimeRootProperties::QuitState) getValue<int> (QuitStatePropertyId);
}

void RuntimeRootProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == AppResumedId)
    {
        if (onAppResumed)
            onAppResumed ();
    }
    else if (property == AppSuspendedId)
    {
        if (onAppSuspended)
            onAppSuspended ();
    }
    else if (property == LayoutPropertyId)
    {
        if (onLayoutChange != nullptr)
            onLayoutChange (getLayout ());
    }
    else if (property == MinimiseButtonPressedId)
    {
        if (onMinimiseButtonPressed)
            onMinimiseButtonPressed ();
    }
    else if (property == MaximiseButtonPressedId)
    {
        if (onMaximiseButtonPressed)
            onMaximiseButtonPressed ();
    }
    else if (property == QuitStatePropertyId)
    {
        if (onQuitStateChanged)
            onQuitStateChanged (getQuitState ());
    }
    else if (property == SystemRequestedQuitId)
    {
        if (onSystemRequestedQuit)
            onSystemRequestedQuit ();
    }
    else if (property == WindowMovedId)
    {
       if (onWindowMoved)
           onWindowMoved ();
    }
}
