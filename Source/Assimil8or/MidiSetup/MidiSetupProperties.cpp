#include "MidiSetupProperties.h"

void MidiSetupProperties::initValueTree ()
{
    // initialize to defaults
    setAssign (0, false);
    setBasicChannel (0, false);
    setColACC (-1, false);
    setColBCC (-1, false);
    setColCCC (-1, false);
    setIndexBaseKey (120, false);
    setMode (1, false);
    setNotifications (1, false);
    setPitchWheelSemi (12, false);
    setRcvProgramChange (1, false);
    setVelocityDepth (32, false);
    setXmtProgramChange (1, false);
}

void MidiSetupProperties::setMode (int mode, bool includeSelfCallback)
{
    setValue (mode, ModePropertyId, includeSelfCallback);
}

void MidiSetupProperties::setAssign (int assign, bool includeSelfCallback)
{
    setValue (assign, AssignPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setIndexBaseKey (int baseKey, bool includeSelfCallback)
{
    setValue (baseKey, IndexBaseKeyPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setBasicChannel (int basicChannel, bool includeSelfCallback)
{
    setValue (basicChannel, BasicChannelPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setRcvProgramChange (int receiveChange, bool includeSelfCallback)
{
    setValue (receiveChange, RcvProgramChangePropertyId, includeSelfCallback);
}

void MidiSetupProperties::setXmtProgramChange (int transmitChange, bool includeSelfCallback)
{
    setValue (transmitChange, XmtProgramChangePropertyId, includeSelfCallback);
}

void MidiSetupProperties::setColACC (int cc, bool includeSelfCallback)
{
    setValue (cc, ColACCPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setColBCC (int cc, bool includeSelfCallback)
{
    setValue (cc, ColBCCPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setColCCC (int cc, bool includeSelfCallback)
{
    setValue (cc, ColCCCPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setPitchWheelSemi (int semiTones, bool includeSelfCallback)
{
    setValue (semiTones, PitchWheelSemiPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setVelocityDepth (int velocityDepth, bool includeSelfCallback)
{
    setValue (velocityDepth, VelocityDepthPropertyId, includeSelfCallback);
}

void MidiSetupProperties::setNotifications (int notifications, bool includeSelfCallback)
{
    setValue (notifications, NotificationsPropertyId, includeSelfCallback);
}

int MidiSetupProperties::getMode ()
{
    return getValue<int> (ModePropertyId);
}

int MidiSetupProperties::getAssign ()
{
    return getValue<int> (AssignPropertyId);
}

int MidiSetupProperties::getIndexBaseKey ()
{
    return getValue<int> (IndexBaseKeyPropertyId);
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

void MidiSetupProperties::copyFrom (juce::ValueTree srcMidiSetupPropertiesVT)
{
    MidiSetupProperties srcMidiSetupProperties { srcMidiSetupPropertiesVT, MidiSetupProperties::WrapperType::client, MidiSetupProperties::EnableCallbacks::no };
    setMode (srcMidiSetupProperties.getMode (), true);
    setAssign (srcMidiSetupProperties.getAssign (), true);
    setIndexBaseKey (srcMidiSetupProperties.getIndexBaseKey (), true);
    setBasicChannel (srcMidiSetupProperties.getBasicChannel (), true);
    setRcvProgramChange (srcMidiSetupProperties.getRcvProgramChange (), true);
    setXmtProgramChange (srcMidiSetupProperties.getXmtProgramChange (), true);
    setColACC (srcMidiSetupProperties.getColACC (), true);
    setColBCC (srcMidiSetupProperties.getColBCC (), true);
    setColCCC (srcMidiSetupProperties.getColCCC (), true);
    setPitchWheelSemi (srcMidiSetupProperties.getPitchWheelSemi (), true);
    setVelocityDepth (srcMidiSetupProperties.getVelocityDepth (), true);
    setNotifications (srcMidiSetupProperties.getNotifications (), true);
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
    else if (property == IndexBaseKeyPropertyId)
    {
        if (onIndexBaseKeyChange != nullptr)
            onIndexBaseKeyChange (getIndexBaseKey ());
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
        if (onColACCChange != nullptr)
            onColACCChange (getColACC ());
    }
    else if (property == ColBCCPropertyId)
    {
        if (onColBCCChange != nullptr)
            onColBCCChange (getColBCC ());
    }
    else if (property == ColCCCPropertyId)
    {
        if (onColCCCChange != nullptr)
            onColCCCChange (getColCCC ());
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