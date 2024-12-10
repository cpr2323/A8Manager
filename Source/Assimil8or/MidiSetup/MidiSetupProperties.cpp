#include "MidiSetupProperties.h"

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
