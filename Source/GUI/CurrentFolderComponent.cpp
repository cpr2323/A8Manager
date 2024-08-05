#include "CurrentFolderComponent.h"
#include "../Utility/PersistentRootProperties.h"
#include "../Utility/RuntimeRootProperties.h"

CurrentFolderComponent::CurrentFolderComponent ()
{
    addAndMakeVisible (currentFolderAndProgressLabel);
}

void CurrentFolderComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    appProperties.onMostRecentFolderChange = [this] (juce::String folderName)
    {
        currentFolderAndProgressLabel.setText (getFolderAndProgressString (folderName, directoryDataProperties.getProgress ()), juce::NotificationType::dontSendNotification);
    };
    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onProgressChange = [this] (juce::String progressString)
    {
        currentFolderAndProgressLabel.setText (getFolderAndProgressString (appProperties.getMostRecentFolder (), progressString), juce::NotificationType::dontSendNotification);
    };

    currentFolderAndProgressLabel.setText (getFolderAndProgressString (appProperties.getMostRecentFolder (), directoryDataProperties.getProgress ()), juce::NotificationType::dontSendNotification);
}

juce::String CurrentFolderComponent::getFolderAndProgressString (juce::String folder, juce::String progress)
{
    if (! progress.isEmpty ())
        return folder + " (" + progress + ")";
    else
        return folder;
}

void CurrentFolderComponent::resized ()
{
    currentFolderAndProgressLabel.setBounds (getLocalBounds ());
}
