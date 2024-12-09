#include "MidiSetup.h"

MidiSetupFile::MidiSetupFile ()
{
    initParser ();
}

void MidiSetupFile::write (juce::File midiSetupFile, juce::ValueTree midiSetupPropertiesVT)
{
    MidiSetupProperties midiSetupPropertiesToWrite (midiSetupPropertiesVT, MidiSetupProperties::WrapperType::owner, MidiSetupProperties::EnableCallbacks::no);

    juce::StringArray lines;
    auto addLine = [this, &lines] (juce::String key, int value)
    {
        juce::String lineToAdd { key + " : " + juce::String (value) };
        lines.add (juce::String (lineToAdd));
    };

    lines.add ("# MIDI Setup");
    addLine (MidiSetup::ModeId, midiSetupPropertiesToWrite.getMode ());
    addLine (MidiSetup::AssignId, midiSetupPropertiesToWrite.getAssign ());
    addLine (MidiSetup::BasicChannelId, midiSetupPropertiesToWrite.getBasicChannel ());
    addLine (MidiSetup::RcvProgramChangeId, midiSetupPropertiesToWrite.getRcvProgramChange ());
    addLine (MidiSetup::XmtProgramChangeId, midiSetupPropertiesToWrite.getXmtProgramChange ());
    addLine (MidiSetup::ColACCId, midiSetupPropertiesToWrite.getColACC ());
    addLine (MidiSetup::ColBCCId, midiSetupPropertiesToWrite.getColBCC ());
    addLine (MidiSetup::ColCCCId, midiSetupPropertiesToWrite.getColCCC ());
    addLine (MidiSetup::PitchWheelSemiId, midiSetupPropertiesToWrite.getPitchWheelSemi ());
    addLine (MidiSetup::VelocityDepthId, midiSetupPropertiesToWrite.getVelocityDepth ());
    addLine (MidiSetup::NotificationsId, midiSetupPropertiesToWrite.getNotifications ());

    const auto stringToWrite { lines.joinIntoString ("\r\n") };
    midiSetupFile.replaceWithText (stringToWrite);
}

juce::ValueTree MidiSetupFile::parse (juce::StringArray presetLines)
{
    for (const auto& presetLine : presetLines)
    {
        // skip empty lines and comment lines
        if (presetLine.initialSectionNotContaining (" ").startsWith ("#") || presetLine.trim ().isEmpty ())
            continue;

        key = presetLine.upToFirstOccurrenceOf (":", false, false).trim ();
        value = presetLine.fromFirstOccurrenceOf (":", false, false).trim ();
        const auto paramName { key.upToFirstOccurrenceOf (" ", false, false) };
        if (const auto action { globalActions.find (paramName) }; action != globalActions.end ())
        {
            action->second ();
        }
        else
        {
            jassertfalse;
        }
    }
    return midiSetupProperties.getValueTree ();
}

void MidiSetupFile::initParser ()
{
    globalActions.insert ({
        {MidiSetup::ModeId, [this] ()
        {
            midiSetupProperties.setMode (value.getIntValue (), false);
        }},
        {MidiSetup::AssignId, [this] ()
        {
            midiSetupProperties.setAssign (value.getIntValue (), false);
        }},
        {MidiSetup::BasicChannelId, [this] ()
        {
            midiSetupProperties.setBasicChannel (value.getIntValue (), false);
        }},
        {MidiSetup::RcvProgramChangeId, [this] ()
        {
            midiSetupProperties.setRcvProgramChange (value.getIntValue (), false);
        }},
        {MidiSetup::XmtProgramChangeId, [this] ()
        {
            midiSetupProperties.setXmtProgramChange(value.getIntValue (), false);
        }},
        {MidiSetup::ColACCId, [this] ()
        {
            midiSetupProperties.setColACC (value.getIntValue (), false);
        }},
        {MidiSetup::ColBCCId, [this] ()
        {
            midiSetupProperties.setColBCC (value.getIntValue (), false);
        }},
        {MidiSetup::ColCCCId, [this] ()
        {
            midiSetupProperties.setColCCC (value.getIntValue (), false);
        }},
        {MidiSetup::PitchWheelSemiId, [this] ()
        {
            midiSetupProperties.setPitchWheelSemi (value.getIntValue (), false);
        }},
        {MidiSetup::VelocityDepthId, [this] ()
        {
            midiSetupProperties.setVelocityDepth (value.getIntValue (), false);
        }},
        {MidiSetup::NotificationsId, [this] ()
        {
            midiSetupProperties.setNotifications (value.getIntValue (), false);
        }},
    });

}

void MidiSetupProperties::setMode (int mode, bool includeSelfCallback)
{
    setValue (mode, ModePropertyId, false);
}

void MidiSetupProperties::setAssign (int assign, bool includeSelfCallback)
{
    setValue (assign, AssignPropertyId, false);
}

void MidiSetupProperties::setBasicChannel (int basicChannel, bool includeSelfCallback)
{
    setValue (basicChannel, BasicChannelPropertyId, false);
}

void MidiSetupProperties::setRcvProgramChange (int receiveChange, bool includeSelfCallback)
{
    setValue (receiveChange, RcvProgramChangePropertyId, false);
}

void MidiSetupProperties::setXmtProgramChange (int transmitChange, bool includeSelfCallback)
{
    setValue (transmitChange, XmtProgramChangePropertyId, false);
}

void MidiSetupProperties::setColACC (int cc, bool includeSelfCallback)
{
    setValue (cc, ColACCPropertyId, false);
}

void MidiSetupProperties::setColBCC (int cc, bool includeSelfCallback)
{
    setValue (cc, ColBCCPropertyId, false);
}

void MidiSetupProperties::setColCCC (int cc, bool includeSelfCallback)
{
    setValue (cc, ColCCCPropertyId, false);
}

void MidiSetupProperties::setPitchWheelSemi (int semiTones, bool includeSelfCallback)
{
    setValue (semiTones, PitchWheelSemiPropertyId, false);
}

void MidiSetupProperties::setVelocityDepth (int velocityDepth, bool includeSelfCallback)
{
    setValue (velocityDepth, VelocityDepthPropertyId, false);
}

void MidiSetupProperties::setNotifications (int notifications, bool includeSelfCallback)
{
    setValue (notifications, NotificationsPropertyId, false);
}

int MidiSetupProperties::getMode ()
{
    return getValue<int> (ModePropertyId);
}

int MidiSetupProperties::getAssign ()
{
    return getValue<int> (AssignPropertyId);
}

int MidiSetupProperties::getBasicChannel ()
{
    return getValue<int> (BasicChannelPropertyId);
}

int MidiSetupProperties::getRcvProgramChange ()
{
    return getValue<int> (RcvProgramChangePropertyId);
}

int MidiSetupProperties::getXmtProgramChange ()
{
    return getValue<int> (XmtProgramChangePropertyId);
}

int MidiSetupProperties::getColACC ()
{
    return getValue<int> (ColACCPropertyId);
}

int MidiSetupProperties::getColBCC ()
{
    return getValue<int> (ColBCCPropertyId);
}

int MidiSetupProperties::getColCCC ()
{
    return getValue<int> (ColCCCPropertyId);
}

int MidiSetupProperties::getPitchWheelSemi ()
{
    return getValue<int> (PitchWheelSemiPropertyId);
}

int MidiSetupProperties::getVelocityDepth ()
{
    return getValue<int> (VelocityDepthPropertyId);
}

int MidiSetupProperties::getNotifications ()
{
    return getValue<int> (NotificationsPropertyId);
}

void MidiSetupProperties::initValueTree ()
{
    // initialize to defaults
    setMode (1, false);
    setAssign (0, false);
    setBasicChannel (0, false);
    setRcvProgramChange (1, false);
    setXmtProgramChange (1, false);
    setColACC (-1, false);
    setColBCC (-1, false);
    setColCCC (-1, false);
    setPitchWheelSemi (12, false);
    setVelocityDepth (32, false);
    setNotifications (1, false);
}

void MidiSetupProperties::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    if (vt != data)
        return;

    if (property == ModePropertyId)
    {
        if (onModeChange != nullptr)
            onModeChange (getMode ());
    }
    else if (property == AssignPropertyId)
    {
        if (onAssignChange != nullptr)
            onAssignChange (getAssign ());
    }
    else if (property == BasicChannelPropertyId)
    {
        if (onBasicChannelChange != nullptr)
            onBasicChannelChange (getBasicChannel ());
    }
    else if (property == RcvProgramChangePropertyId)
    {
        if (onRcvProgramChangeChange != nullptr)
            onRcvProgramChangeChange (getRcvProgramChange ());
    }
    else if (property == XmtProgramChangePropertyId)
    {
        if (onXmtProgramChangeChange != nullptr)
            onXmtProgramChangeChange (getXmtProgramChange ());
    }
    else if (property == ColACCPropertyId)
    {
        if (onCollACCChange != nullptr)
            onCollACCChange (getColACC ());
    }
    else if (property == ColBCCPropertyId)
    {
        if (onCollBCCChange != nullptr)
            onCollBCCChange (getColBCC ());
    }
    else if (property == ColCCCPropertyId)
    {
        if (onCollCCCChange != nullptr)
            onCollCCCChange (getColCCC ());
    }
    else if (property == PitchWheelSemiPropertyId)
    {
        if (onPitchWheelSemiChange != nullptr)
            onPitchWheelSemiChange (getPitchWheelSemi ());
    }
    else if (property == VelocityDepthPropertyId)
    {
        if (onVelocityDepthChange != nullptr)
            onVelocityDepthChange (getVelocityDepth ());
    }
    else if (property == NotificationsPropertyId)
    {
        if (onNotificationsChange != nullptr)
            onNotificationsChange (getNotifications ());
    }
}
