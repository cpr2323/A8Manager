#include "Assimil8orSdCardComponent.h"
#include "../../Utility/RuntimeRootProperties.h"

Assimil8orSdCardComponent::Assimil8orSdCardComponent ()
{
    setOpaque (true);

    scanStatusListBox.setClickingTogglesRowSelection (false);
    scanStatusListBox.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
    scanStatusListBox.setOutlineThickness (1);
    scanStatusListBox.getHeader ().addColumn ("Status", 1, 60, 10, 60, juce::TableHeaderComponent::visible);
    scanStatusListBox.getHeader ().addColumn ("Message (0 items)", 2, 100, 10, 3000, juce::TableHeaderComponent::visible);
    addAndMakeVisible (scanStatusListBox);

    auto setupFilterButton = [this] (juce::TextButton& button, juce::String text)
    {
        button.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
        button.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green.darker(0.5f));
        button.setClickingTogglesState (true);
        button.setToggleable (true);
        button.setButtonText (text);
        button.setToggleState (true, juce::NotificationType::dontSendNotification);
        button.onClick = [this] ()
        {
            setupFilterList ();
            scanStatusQuickLookupList.clear ();
            buildQuickLookupList ();
            repaint ();
        };
        addAndMakeVisible (button);
    };
    setupFilterButton (idleFilterButton, "I");
    setupFilterButton (warningFilterButton, "W");
    setupFilterButton (errorFilterButton, "E");

    setupFilterList ();
    scanStatusQuickLookupList.clear ();
    buildQuickLookupList ();
    repaint ();
}

void Assimil8orSdCardComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    validatorProperties.wrap (runtimeRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::yes);
    validatorProperties.onScanStatusChanged = [this] (juce::String scanStatus)
    {
        scanStatusQuickLookupList.clear ();
        if (scanStatus == "idle")
            buildQuickLookupList ();

        scanStatusListBox.getHeader ().setColumnName (2, "Message (" + juce::String (scanStatusQuickLookupList.size ()) + " items)");
        scanStatusListBox.repaint ();
    };
}

void Assimil8orSdCardComponent::setupFilterList ()
{
    filterList.clearQuick ();
    if (idleFilterButton.getToggleState ())
        filterList.add ("info");
    if (warningFilterButton.getToggleState ())
        filterList.add ("warning");
    if (errorFilterButton.getToggleState ())
        filterList.add ("error");
}
void Assimil8orSdCardComponent::buildQuickLookupList ()
{
    // iterate over the state message list, adding each one to the quick list
    ValueTreeHelpers::forEachChildOfType (validatorProperties.getValidationStatusVT (), "Status", [this] (juce::ValueTree child)
    {
        if (filterList.contains (child.getProperty ("type").toString ()))
            scanStatusQuickLookupList.emplace_back (child);
        return true;
    });
}

void Assimil8orSdCardComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orSdCardComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    scanStatusListBox.setBounds (localBounds);
    scanStatusListBox.getHeader ().setColumnWidth (2, scanStatusListBox.getWidth () - 2 - scanStatusListBox.getHeader ().getColumnWidth (1));
    auto filterButtonBounds { getLocalBounds().removeFromBottom(45).withTrimmedBottom(15).withTrimmedRight(15) };
    errorFilterButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    warningFilterButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
    filterButtonBounds.removeFromRight (5);
    idleFilterButton.setBounds (filterButtonBounds.removeFromRight (filterButtonBounds.getHeight ()));
}

int Assimil8orSdCardComponent::getNumRows ()
{
    return (int) scanStatusQuickLookupList.size ();
}

void Assimil8orSdCardComponent::paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    if (rowNumber >= scanStatusQuickLookupList.size ())
        return;

    if (rowIsSelected)
    {
        g.fillAll (juce::Colours::lightblue);
    }
    else
    {
        auto unSelectedBackgroundColour { juce::Colours::lightgrey };
        if (rowNumber % 2)
            unSelectedBackgroundColour = unSelectedBackgroundColour.interpolatedWith (juce::Colours::black, 0.1f);
        g.fillAll (unSelectedBackgroundColour);
    }
}

void Assimil8orSdCardComponent::paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (rowNumber < scanStatusQuickLookupList.size ())
    {
        g.setColour (juce::Colours::lightsteelblue);
        g.fillRect (width - 1, 0, 1, height);
        //if (! rowIsSelected)
        {
            // draw rowNumber entry
            auto textColor { juce::Colours::black };
            auto statusType { scanStatusQuickLookupList [rowNumber].getProperty ("type").toString () };
            if (statusType == "warning")
                textColor = juce::Colours::orange.darker (0.3f);
            else if (statusType == "error")
                textColor = juce::Colours::red.darker (0.3f);
            g.setColour (textColor);
            juce::String data {"  "};
            switch (columnId)
            {
                case 1:
                {
                    data += scanStatusQuickLookupList [rowNumber].getProperty ("type").toString ();
                }
                break;
                case 2:
                {
                    data += scanStatusQuickLookupList [rowNumber].getProperty ("text").toString ();
                }
                break;
            }
            g.drawText (data, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        }
    }
}

juce::Component* Assimil8orSdCardComponent::refreshComponentForCell (int rowNumber, [[maybe_unused]] int columnId, bool rowIsSelected,
                                                                     juce::Component* existingComponentToUpdate)
{
    if (rowIsSelected)
    {
        jassert (rowNumber < scanStatusQuickLookupList.size ());
    }
    else if (rowNumber < scanStatusQuickLookupList.size ())
    {
        if (existingComponentToUpdate != nullptr)
            delete existingComponentToUpdate;
        return nullptr;
    }

    jassert (existingComponentToUpdate == nullptr);
    return nullptr;
}

void Assimil8orSdCardComponent::cellDoubleClicked (int rowNumber, int columnId, const juce::MouseEvent& mouseEvent)
{
    juce::Logger::outputDebugString ("rowNum: " + juce::String (rowNumber) + ", colId: " + juce::String (columnId));
}
