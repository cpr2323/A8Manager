#include "CurrentFolderComponent.h"
#include "Theme/A8ColourIds.h"
#include "../SystemServices.h"
#include "oolib/Properties/PersistentRootProperties.h"
#include "oolib/Properties/RuntimeRootProperties.h"

CurrentFolderComponent::CurrentFolderComponent ()
{
    setOpaque (true);
    outputButton.setShowsCaret (true);
    outputButton.setTooltip ("The audio output used to audition samples");
    outputButton.onClick = [this] () { showOutputMenu (); };
    addAndMakeVisible (outputButton);

    settingsButton.setTooltip ("Audio and Appearance settings");
    settingsButton.onClick = [this] () { guiProperties.showSettingsDialog (false); };
    addAndMakeVisible (settingsButton);
}

void CurrentFolderComponent::init (juce::ValueTree rootPropertiesVT)
{
    PersistentRootProperties persistentRootProperties (rootPropertiesVT, PersistentRootProperties::WrapperType::client, PersistentRootProperties::EnableCallbacks::no);
    appProperties.wrap (persistentRootProperties.getValueTree (), AppProperties::WrapperType::client, AppProperties::EnableCallbacks::yes);
    guiProperties.wrap (persistentRootProperties.getValueTree (), GuiProperties::WrapperType::client, GuiProperties::EnableCallbacks::no);
    // the folder being viewed and the preset last opened change independently
    appProperties.onMostRecentFolderChange = [this] (juce::String) { refreshPath (); };
    appProperties.onMostRecentFileChange = [this] (juce::String) { refreshPath (); };

    RuntimeRootProperties runtimeRootProperties (rootPropertiesVT, RuntimeRootProperties::WrapperType::client, RuntimeRootProperties::EnableCallbacks::no);
    SystemServices systemServices (runtimeRootProperties.getValueTree (), SystemServices::WrapperType::client, SystemServices::EnableCallbacks::no);
    audioDeviceManager = systemServices.getAudioDeviceManager ();

    directoryDataProperties.wrap (runtimeRootProperties.getValueTree (), DirectoryDataProperties::WrapperType::client, DirectoryDataProperties::EnableCallbacks::yes);
    directoryDataProperties.onProgressChange = [this] (juce::String progressString)
    {
        progressText = progressString;
        repaint ();
    };
    progressText = directoryDataProperties.getProgress ();

    refreshPath ();
    refreshOutputName ();
}

void CurrentFolderComponent::refreshPath ()
{
    const auto viewedFolder { appProperties.getMostRecentFolder () };
    pathSegments.clear ();
    pathSegments.addTokens (viewedFolder, juce::File::getSeparatorString (), {});
    pathSegments.removeEmptyStrings ();

    // The preset last opened is only still open if it is in the folder being viewed. After a folder
    // change the crumbs stop at the folder until the preset list has loaded one of its presets.
    const auto lastPresetOpenedName { appProperties.getRecentlyUsedFile (0) };
    const auto lastPresetOpened { juce::File (lastPresetOpenedName) };
    pathEndsInFile = viewedFolder.isNotEmpty () && lastPresetOpenedName.isNotEmpty ()
                     && lastPresetOpened.getParentDirectory () == juce::File (viewedFolder);
    if (pathEndsInFile)
        pathSegments.add (lastPresetOpened.getFileName ());

    repaint ();
}

void CurrentFolderComponent::refreshOutputName ()
{
    auto deviceName { juce::String ("none") };
    if (audioDeviceManager != nullptr)
        if (auto* device { audioDeviceManager->getCurrentAudioDevice () })
            deviceName = device->getName ();

    outputButton.setValueText (deviceName);
    resized ();
    repaint ();
}

void CurrentFolderComponent::showOutputMenu ()
{
    if (audioDeviceManager == nullptr)
        return;

    auto* deviceType { audioDeviceManager->getCurrentDeviceTypeObject () };
    if (deviceType == nullptr)
        return;

    deviceType->scanForDevices ();
    const auto deviceNames { deviceType->getDeviceNames (false) };
    const auto currentName { audioDeviceManager->getCurrentAudioDevice () != nullptr
                             ? audioDeviceManager->getCurrentAudioDevice ()->getName ()
                             : juce::String () };

    juce::PopupMenu menu;
    menu.addSectionHeader (deviceType->getTypeName ());
    menu.addSeparator ();
    for (auto deviceIndex { 0 }; deviceIndex < deviceNames.size (); ++deviceIndex)
    {
        const auto name { deviceNames [deviceIndex] };
        menu.addItem (name, true, name == currentName, [this, name] ()
        {
            auto setup { audioDeviceManager->getAudioDeviceSetup () };
            setup.outputDeviceName = name;
            // an empty result means it opened; anything else is the reason it did not
            const auto result { audioDeviceManager->setAudioDeviceSetup (setup, true) };
            if (result.isNotEmpty ())
                juce::Logger::writeToLog ("could not open audio output '" + name + "': " + result);
            refreshOutputName ();
        });
    }
    menu.showMenuAsync (juce::PopupMenu::Options ().withTargetComponent (&outputButton));
}

void CurrentFolderComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (A8Colours::listBackground));
    g.setColour (findColour (A8Colours::outline));
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));

    const auto separator { juce::String (juce::CharPointer_UTF8 ("\xe2\x80\xba")) };
    const auto ellipsis { juce::String (juce::CharPointer_UTF8 ("\xe2\x80\xa6")) };
    constexpr auto kGap { 5 };
    const auto contextFont { A8Type::body () };
    const auto currentFont { A8Type::bodyStrong () };
    const auto textArea { getLocalBounds ().withTrimmedBottom (1) };
    const auto separatorWidth { A8Paint::textWidth (contextFont, separator) };
    const auto ellipsisWidth { A8Paint::textWidth (contextFont, ellipsis) };
    const auto progress { progressText.isNotEmpty () ? "(" + progressText + ")" : juce::String () };
    const auto progressWidth { progress.isNotEmpty () ? A8Paint::textWidth (contextFont, progress) + kGap : 0 };

    auto isFileSegment = [this] (int segmentIndex) { return pathEndsInFile && segmentIndex == pathSegments.size () - 1; };
    auto segmentWidth = [&] (int segmentIndex)
    {
        return A8Paint::textWidth (isFileSegment (segmentIndex) ? currentFont : contextFont, pathSegments [segmentIndex]);
    };
    auto widthFrom = [&] (int firstSegment)
    {
        auto width { firstSegment > 0 ? ellipsisWidth + kGap + separatorWidth + kGap : 0 };
        for (auto segmentIndex { firstSegment }; segmentIndex < pathSegments.size (); ++segmentIndex)
            width += segmentWidth (segmentIndex) + (segmentIndex < pathSegments.size () - 1 ? kGap + separatorWidth + kGap : 0);
        return width;
    };

    // When the whole path does not fit, the folders nearest the root are the ones
    // dropped, so the folder being viewed and the open file stay in sight.
    const auto rightEdge { outputButton.getX () - 12 - progressWidth };
    auto firstSegment { 0 };
    while (firstSegment < pathSegments.size () - 1 && kPadding + widthFrom (firstSegment) > rightEdge)
        ++firstSegment;

    auto x { kPadding };
    auto drawContext = [&] (const juce::String& text, int width, int colourId)
    {
        g.setFont (contextFont);
        g.setColour (findColour (colourId));
        g.drawText (text, textArea.withX (x).withWidth (width + 1), juce::Justification::centredLeft, false);
        x += width + kGap;
    };

    if (firstSegment > 0)
    {
        drawContext (ellipsis, ellipsisWidth, A8Colours::textDim);
        drawContext (separator, separatorWidth, A8Colours::menuHeaderText);
    }

    for (auto segmentIndex { firstSegment }; segmentIndex < pathSegments.size (); ++segmentIndex)
    {
        const auto isFile { isFileSegment (segmentIndex) };
        const auto width { segmentWidth (segmentIndex) };
        // a single crumb too long for the bar is squeezed rather than overdrawing the chips
        const auto drawWidth { std::max (0, std::min (width + 1, rightEdge - x)) };
        g.setFont (isFile ? currentFont : contextFont);
        g.setColour (findColour (isFile ? A8Colours::text : A8Colours::textDim));
        g.drawFittedText (pathSegments [segmentIndex], textArea.withX (x).withWidth (drawWidth), juce::Justification::centredLeft, 1, 0.8f);
        x += std::min (width, drawWidth) + kGap;

        if (segmentIndex < pathSegments.size () - 1)
            drawContext (separator, separatorWidth, A8Colours::menuHeaderText);
    }

    if (progress.isNotEmpty ())
        drawContext (progress, progressWidth - kGap, A8Colours::textGhost);
}

void CurrentFolderComponent::resized ()
{
    auto localBounds { getLocalBounds ().withTrimmedBottom (1).reduced (kPadding, 0) };
    const auto settingsWidth { settingsButton.getIdealWidth () };
    settingsButton.setBounds (localBounds.removeFromRight (settingsWidth).withSizeKeepingCentre (settingsWidth, ChromeButton::kChipHeight));
    localBounds.removeFromRight (8);

    // a long device name is cut short rather than pushing the path out of view
    const auto outputWidth { std::min (outputButton.getIdealWidth (), 320) };
    outputButton.setBounds (localBounds.removeFromRight (outputWidth).withSizeKeepingCentre (outputWidth, ChromeButton::kChipHeight));
}
