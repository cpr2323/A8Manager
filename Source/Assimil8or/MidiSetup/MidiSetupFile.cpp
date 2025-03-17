#include "MidiSetupFile.h"

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
    addLine (MidiSetup::IndexBaseKeyId, midiSetupPropertiesToWrite.getIndexBaseKey ());

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
        {MidiSetup::IndexBaseKeyId, [this] ()
        {
            midiSetupProperties.setIndexBaseKey (value.getIntValue (), false);
        }},
    });
}

