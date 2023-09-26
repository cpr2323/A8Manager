#include "ValueTreeMonitor.h"

void ValueTreeMonitor::assign (juce::ValueTree& vtToListenTo)
{
    vtBeingListened = vtToListenTo;
    vtBeingListened.addListener (this);
}

void ValueTreeMonitor::valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property)
{
    juce::Logger::outputDebugString ("valueTreePropertyChanged(" + vtBeingListened.getType ().toString () + ")");
    if (vt == vtBeingListened)
    {
        if (vt.hasProperty (property))
        {
            // change in value
            juce::Logger::outputDebugString ("  (property value changed) - property: " +
                property.toString () + ", value: " + vt.getProperty (property).toString ());
        }
        else
        {
            // property removed
            juce::Logger::outputDebugString ("  (property removed) - property: " + property.toString ());
        }
    }
    else
    {
        if (vt.hasProperty (property))
        {
            // change in value
            juce::Logger::outputDebugString ("  (value changed) - vt: " + vt.getType ().toString () +
                ", property: " + property.toString () + ", value: " + vt.getProperty (property).toString ());
        }
        else
        {
            // property removed
            juce::Logger::outputDebugString ("  (property removed) - vt: " + vt.getType ().toString () + ", property: " + property.toString ());
        }
    }
}

void ValueTreeMonitor::valueTreeChildAdded (juce::ValueTree& vt, juce::ValueTree& child)
{
    juce::Logger::outputDebugString ("valueTreeChildAdded(" + vtBeingListened.getType ().toString () + ")");

    if (vt == vtBeingListened)
        juce::Logger::outputDebugString ("  child: " + child.getType ().toString ());
    else
        juce::Logger::outputDebugString ("  vt: " + vt.getType ().toString () + ", child: " + child.getType ().toString ());

}

void ValueTreeMonitor::valueTreeChildRemoved (juce::ValueTree& vt, juce::ValueTree& child, int index)
{
    juce::Logger::outputDebugString ("valueTreeChildRemoved(" + vtBeingListened.getType ().toString () + ")");
    if (vt == vtBeingListened)
        juce::Logger::outputDebugString ("  child: " + child.getType ().toString () + ", index: " + juce::String (index));
    else
        juce::Logger::outputDebugString ("  vt: " + vt.getType ().toString () +
            ", child: " + child.getType ().toString () + ", index: " + juce::String (index));
}

void ValueTreeMonitor::valueTreeChildOrderChanged (juce::ValueTree& vt, int oldIndex, int newIndex)
{
    juce::Logger::outputDebugString ("valueTreeChildOrderChanged(" + vtBeingListened.getType ().toString () + ")");
    if (vt == vtBeingListened)
        juce::Logger::outputDebugString ("  old index: " + juce::String (oldIndex) + ", new index: " + juce::String (newIndex));
    else
        juce::Logger::outputDebugString ("  vt: " + vt.getType ().toString () +
            ", old index: " + juce::String (oldIndex) + ", new index: " + juce::String (newIndex));
}

void ValueTreeMonitor::valueTreeParentChanged (juce::ValueTree& vt)
{
    juce::Logger::outputDebugString ("valueTreeParentChanged(" + vtBeingListened.getType ().toString () + ")");
    if (vt == vtBeingListened)
    {
        if (vt.getParent ().isValid ())
            juce::Logger::outputDebugString ("  new parent: " + vt.getParent ().getType ().toString ());
        else
            juce::Logger::outputDebugString ("  (removed)");
    }
    else
    {
        if (vt.getParent ().isValid ())
            juce::Logger::outputDebugString ("  vt: " + vt.getType ().toString () +
                ", new parent: " + vt.getParent ().getType ().toString ());
        else
            juce::Logger::outputDebugString ("  (removed) vt: " + vt.getType ().toString ());
    }
}

void ValueTreeMonitor::valueTreeRedirected (juce::ValueTree& vt)
{
    juce::Logger::outputDebugString ("valueTreeRedirected(" + vtBeingListened.getType ().toString () + "): " + vt.getType ().toString ());
    vtBeingListened = vt;
    //if (vt == vtBeingListened)
}
