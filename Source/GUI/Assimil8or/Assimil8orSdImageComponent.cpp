#include "Assimil8orSdImageComponent.h"
#include "../../Utility/RuntimeRootProperties.h"

Assimil8orSdImageComponent::Assimil8orSdImageComponent ()
{
    setOpaque (true);

//    sdImageListBox.setLookAndFeel (&tableListBoxHeaderCenteredLookAndFeel);
    sdImageListBox.setClickingTogglesRowSelection (true);
    sdImageListBox.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
    sdImageListBox.setOutlineThickness (1);
    sdImageListBox.getHeader ().addColumn ("Status Message (0 items)", 1, 100, 10, 3000, juce::TableHeaderComponent::visible);
    addAndMakeVisible (sdImageListBox);
}

void Assimil8orSdImageComponent::init (juce::ValueTree rootPropertiesVT)
{
    RuntimeRootProperties runtimeRootProperties;
    runtimeRootProperties.wrap (rootPropertiesVT, ValueTreeWrapper::WrapperType::client, ValueTreeWrapper::EnableCallbacks::no);
    sdCardImage = runtimeRootProperties.getValueTree ().getChildWithName ("SDCardImage");
    jassert (sdCardImage.isValid ());
    sdCardImage.addListener (this);
    validationStatusProperties = sdCardImage.getChildWithName ("ValidationStatus");
}

void Assimil8orSdImageComponent::paint ([[maybe_unused]] juce::Graphics& g)
{
    g.fillAll (juce::Colours::navajowhite);
}

void Assimil8orSdImageComponent::resized ()
{
    auto localBounds { getLocalBounds () };
    sdImageListBox.setBounds (localBounds);
    sdImageListBox.getHeader ().setColumnWidth (1, sdImageListBox.getWidth () - 2);
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

void Assimil8orSdImageComponent::paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < quickLookupList.size ())
    {
        if (! rowIsSelected)
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
                    data += quickLookupList [rowNumber].getProperty ("text").toString ();
                }
                break;
            }
            g.drawText (data, juce::Rectangle<float>{ 0.0f, 0.0f, (float) width, (float) height }, juce::Justification::centredLeft, true);
        }
    }
    g.setColour (juce::Colours::lightsteelblue);
    g.fillRect (width - 1, 0, 1, height);
}

juce::Component* Assimil8orSdImageComponent::refreshComponentForCell (int rowNumber, int columnId, bool rowIsSelected,
                                                                      juce::Component* existingComponentToUpdate)
{
    if (rowIsSelected)
    {
        jassert (rowNumber < quickLookupList.size ());

#if 0
        switch (columnId)
        {
        case kGestureColumn:
        {
            String gesture;
            jassert (gmtList [rowNumber]->getGesture () != nullptr);
            if (gmtList [rowNumber]->getGesture () != nullptr)
                gesture = gmtList [rowNumber]->getGesture ()->getName ();
            auto* comboBox = static_cast<GestureEditorComponent*> (existingComponentToUpdate);
            if (comboBox == nullptr)
                comboBox = new GestureEditorComponent;
            comboBox->config (&gestureList);
            comboBox->setSelection (gesture);
            comboBox->setDeleteAction ([this, rowNumber] ()
                {
                    gmtList.erase (gmtList.begin () + rowNumber);
                    updateChainPrioValues ();
                    updatePresetEntry ();
                    gmtTable.updateContent ();
                });
            comboBox->setItem (gmtList [rowNumber]->getGesture ());
            comboBox->setRow (gmtList [rowNumber].get ());
            comboBox->setChangeAction ([this, rowNumber, comboBox] (GmtItem* /*item*/)
                {
                    const auto gestureUuid { getUuidFromName (gestureList, comboBox->getSelection ()) };
                    jassert (gestureUuid.isNotEmpty ());
                    auto newItem { createGesture (gestureUuid) };
                    gmtList [rowNumber].get ()->setGesture (newItem, true);
                    comboBox->setItem (newItem.get ());
                    gmtTable.updateContent ();
                    gmtTable.repaint ();
                });
            return comboBox;
        }
        break;
        case kModifierColumn:
        {
            String modifier;
            jassert (gmtList [rowNumber]->getModifier () != nullptr);
            if (gmtList [rowNumber]->getModifier () != nullptr)
                modifier = gmtList [rowNumber]->getModifier ()->getName ();
            auto* comboBox = static_cast<ModifierEditorComponent*> (existingComponentToUpdate);
            if (comboBox == nullptr)
                comboBox = new ModifierEditorComponent;
            comboBox->config (&modifierList);
            comboBox->setSelection (modifier);
            comboBox->setItem (gmtList [rowNumber]->getModifier ());
            comboBox->setRow (gmtList [rowNumber].get ());
            comboBox->setChangeAction ([this, rowNumber, comboBox] (GmtItem* /*item*/)
                {
                    const auto modifierUuid { getUuidFromName (modifierList, comboBox->getSelection ()) };
                    jassert (modifierUuid.isNotEmpty ());
                    auto newItem { createModifier (modifierUuid) };
                    gmtList [rowNumber].get ()->setModifier (newItem, true);
                    comboBox->setItem (newItem.get ());
                    gmtTable.updateContent ();
                    gmtTable.repaint ();
                });
            return comboBox;
        }
        break;
        case kTargetColumn:
        {
            String target;
            jassert (gmtList [rowNumber]->getTarget () != nullptr);
            if (gmtList [rowNumber]->getTarget () != nullptr)
                target = gmtList [rowNumber]->getTarget ()->getName ();
            auto* comboBox = static_cast<TargetEditorComponent*> (existingComponentToUpdate);
            if (comboBox == nullptr)
                comboBox = new TargetEditorComponent;
            comboBox->config (&targetList);
            comboBox->setSelection (target);
            comboBox->setItem (gmtList [rowNumber]->getTarget ());
            comboBox->setRow (gmtList [rowNumber].get ());
            comboBox->setChangeAction ([this, rowNumber, comboBox] (GmtItem* /*item*/)
                {
                    const auto targetUuid { getUuidFromName (targetList, comboBox->getSelection ()) };
                    jassert (targetUuid.isNotEmpty ());
                    auto newItem { createTarget (targetUuid) };
                    gmtList [rowNumber].get ()->setTarget (newItem, true);
                    comboBox->setItem (newItem.get ());
                    gmtTable.updateContent ();
                    gmtTable.repaint ();
                });
            return comboBox;
        }
        break;
        }
#endif
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

void Assimil8orSdImageComponent::valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == sdCardImage)
    {
        if (property.toString () == "scanStatus")
        {
            juce::Logger::outputDebugString ("tool window received scanning update - " + sdCardImage.getProperty ("scanStatus").toString ());
            if (sdCardImage.getProperty ("scanStatus").toString () == "idle")
            {
                // iterate over the state message list, adding each one to the quick list
                quickLookupList.clear ();
                ValueTreeHelpers::forEachChild (validationStatusProperties, [this] (juce::ValueTree child)
                {
                    if (child.getType().toString() == "Status")
                    {
                        juce::Logger::outputDebugString (child.getProperty ("text").toString ());
                        quickLookupList.emplace_back (child);
                    }
                    return true;
                });
                sdImageListBox.getHeader ().setColumnName (1, "Status Message (" + juce::String (quickLookupList.size ()) + " items)");
                sdImageListBox.repaint ();
            }
        }
    }
}
