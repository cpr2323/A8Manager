#include "Assimil8orSdImageComponent.h"
#include "../../Utility/RuntimeRootProperties.h"

Assimil8orSdImageComponent::Assimil8orSdImageComponent ()
{
    setOpaque (true);

    sdImageListBox.setClickingTogglesRowSelection (false);
    sdImageListBox.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
    sdImageListBox.setOutlineThickness (1);
    sdImageListBox.getHeader ().addColumn ("Status", 1, 60, 10, 60, juce::TableHeaderComponent::visible);
    sdImageListBox.getHeader ().addColumn ("Message (0 items)", 2, 100, 10, 3000, juce::TableHeaderComponent::visible);
    addAndMakeVisible (sdImageListBox);
}

void Assimil8orSdImageComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    a8SDCardValidatorProperties.wrap (runtimeRootProperties.getValueTree (), ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::yes);
    a8SDCardValidatorProperties.onScanStatusChanged = [this] (juce::String scanStatus)
    {
        quickLookupList.clear ();
        if (scanStatus == "idle")
            buildQuickLookupList ();

        sdImageListBox.getHeader ().setColumnName (2, "Message (" + juce::String (quickLookupList.size ()) + " items)");
        sdImageListBox.repaint ();
    };
}

void Assimil8orSdImageComponent::buildQuickLookupList ()
{
    // iterate over the state message list, adding each one to the quick list
    ValueTreeHelpers::forEachChild (a8SDCardValidatorProperties.getValidationStatusVT (), [this] (juce::ValueTree child)
    {
        if (child.getType ().toString () == "Status")
            quickLookupList.emplace_back (child);
        return true;
    });
}

void Assimil8orSdImageComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orSdImageComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    sdImageListBox.setBounds (localBounds);
    sdImageListBox.getHeader ().setColumnWidth (2, sdImageListBox.getWidth () - 2 - sdImageListBox.getHeader ().getColumnWidth (1));
}

int Assimil8orSdImageComponent::getNumRows ()
{
    return (int) quickLookupList.size ();
}

void Assimil8orSdImageComponent::paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    if (rowNumber >= quickLookupList.size ())
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

void Assimil8orSdImageComponent::paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    if (rowNumber < quickLookupList.size ())
    {
        g.setColour (juce::Colours::lightsteelblue);
        g.fillRect (width - 1, 0, 1, height);
        //if (! rowIsSelected)
        {
            // draw rowNumber entry
            auto textColor { juce::Colours::black };
            auto statusType { quickLookupList [rowNumber].getProperty ("type").toString () };
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
                    data += quickLookupList [rowNumber].getProperty ("type").toString ();
                }
                break;
                case 2:
                {
                    data += quickLookupList [rowNumber].getProperty ("text").toString ();
                }
                break;
            }
            g.drawText (data, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        }
    }
}

juce::Component* Assimil8orSdImageComponent::refreshComponentForCell (int rowNumber, [[maybe_unused]] int columnId, bool rowIsSelected,
                                                                      juce::Component* existingComponentToUpdate)
{
    if (rowIsSelected)
    {
        jassert (rowNumber < quickLookupList.size ());
    }
    else if (rowNumber < quickLookupList.size ())
    {
        if (existingComponentToUpdate != nullptr)
            delete existingComponentToUpdate;
        return nullptr;
    }

    jassert (existingComponentToUpdate == nullptr);
    return nullptr;
}
