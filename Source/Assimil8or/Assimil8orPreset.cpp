#include "Assimil8orPreset.h"
#include "FileTypeHelpers.h"
#include "Preset/ParameterNames.h"
#include "Preset/ParameterPresetsSingleton.h"
#include "oolib/Debug/DebugLog.h"

#define LOG_PARSING 0
#if LOG_PARSING
#define LogParsing(text) DebugLog ("Assimil8orPreset", text);
#else
#define LogParsing(text) ;
#endif

Assimil8orPreset::Assimil8orPreset ()
{
    initParser ();

    minPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MinParameterPresetType),
                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    maxPresetProperties.wrap (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::MaxParameterPresetType),
                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
}

void Assimil8orPreset::write (juce::File presetFile, juce::ValueTree presetPropertiesVT)
{
    jassert (presetPropertiesVT.isValid ());

    PresetProperties presetPropertiesToWrite (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    if (FileTypeHelpers::isPresetFile (presetFile))
    {
        if (const auto presetNumber { FileTypeHelpers::getPresetNumberFromName (presetFile) }; presetNumber != FileTypeHelpers::kBadPresetNumber)
            presetPropertiesToWrite.setId (presetNumber, false);
    }

    PresetProperties defaultPresetProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                              PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    ChannelProperties defaultChannelProperties (defaultPresetProperties.getChannelVT (0), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
    ZoneProperties defaultZoneProperties (defaultChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);

    auto indentAmount { 0 };
    juce::StringArray lines;
    auto addLine = [&indentAmount, &lines] (bool doAdd, juce::String lineToAdd)
    {
        if (doAdd)
            lines.add (juce::String (lineToAdd).paddedLeft (' ', lineToAdd.length () + indentAmount * 2));
    };

    addLine (true, Section::PresetId + " " + juce::String (presetPropertiesToWrite.getId ()) + " :");
    ++indentAmount;
    addLine (true, Parameter::Preset::NameId + " : " + presetPropertiesToWrite.getName ());
    addLine (presetPropertiesToWrite.getMidiSetup () != defaultPresetProperties.getMidiSetup (), Parameter::Preset::MidiSetupId + " : " + juce::String (presetPropertiesToWrite.getMidiSetup ()));
    addLine (presetPropertiesToWrite.getData2AsCV () != defaultPresetProperties.getData2AsCV (), Parameter::Preset::Data2asCVId + " : " + presetPropertiesToWrite.getData2AsCV ());
    addLine (presetPropertiesToWrite.getXfadeACV () != defaultPresetProperties.getXfadeACV (), Parameter::Preset::XfadeACVId + " : " + presetPropertiesToWrite.getXfadeACV ());
    addLine (! juce::exactlyEqual (presetPropertiesToWrite.getXfadeAWidth (), defaultPresetProperties.getXfadeAWidth ()), Parameter::Preset::XfadeAWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeAWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeBCV () != defaultPresetProperties.getXfadeBCV (), Parameter::Preset::XfadeBCVId + " : " + presetPropertiesToWrite.getXfadeBCV ());
    addLine (! juce::exactlyEqual (presetPropertiesToWrite.getXfadeBWidth (), defaultPresetProperties.getXfadeBWidth ()), Parameter::Preset::XfadeBWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeBWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeCCV () != defaultPresetProperties.getXfadeCCV (), Parameter::Preset::XfadeCCVId + " : " + presetPropertiesToWrite.getXfadeCCV ());
    addLine (! juce::exactlyEqual (presetPropertiesToWrite.getXfadeCWidth (), defaultPresetProperties.getXfadeCWidth ()), Parameter::Preset::XfadeCWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeCWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeDCV () != defaultPresetProperties.getXfadeDCV (), Parameter::Preset::XfadeDCVId + " : " + presetPropertiesToWrite.getXfadeDCV ());
    addLine (! juce::exactlyEqual (presetPropertiesToWrite.getXfadeDWidth (), defaultPresetProperties.getXfadeDWidth ()), Parameter::Preset::XfadeDWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeDWidth (), 2));

    presetPropertiesToWrite.forEachChannel ([&addLine, &indentAmount, &defaultZoneProperties, &defaultChannelProperties] (juce::ValueTree channelVT, int)
    {
        ChannelProperties curChannelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        ZoneProperties firstZoneProperties (curChannelProperties.getZoneVT (0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        if (! firstZoneProperties.getSample ().isEmpty ())
        {
            addLine (true, Section::ChannelId + " " + juce::String (curChannelProperties.getId ()) + " :");
            ++indentAmount;
            addLine (curChannelProperties.getAliasing () != defaultChannelProperties.getAliasing (), Parameter::Channel::AliasingId + " : " + juce::String (curChannelProperties.getAliasing ()));
            addLine (curChannelProperties.getAliasingMod () != defaultChannelProperties.getAliasingMod (), Parameter::Channel::AliasingModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getAliasingMod (), 4));
            addLine (! juce::exactlyEqual (curChannelProperties.getAttack (), defaultChannelProperties.getAttack ()), Parameter::Channel::AttackId + " : " + juce::String (curChannelProperties.getAttack ()));
            addLine (curChannelProperties.getAttackFromCurrent () != defaultChannelProperties.getAttackFromCurrent (), Parameter::Channel::AttackFromCurrentId + " : " + (curChannelProperties.getAttackFromCurrent () ? "1" : "0"));
            addLine (curChannelProperties.getAttackMod () != defaultChannelProperties.getAttackMod (), Parameter::Channel::AttackModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getAttackMod (), 4));
            addLine (curChannelProperties.getAutoTrigger () != defaultChannelProperties.getAutoTrigger (), Parameter::Channel::AutoTriggerId + " : " + (curChannelProperties.getAutoTrigger () ? "1" : "0"));
            addLine (! juce::exactlyEqual (curChannelProperties.getBits (), defaultChannelProperties.getBits ()), Parameter::Channel::BitsId + " : " + juce::String (curChannelProperties.getBits (), 1));
            addLine (curChannelProperties.getBitsMod () != defaultChannelProperties.getBitsMod (), Parameter::Channel::BitsModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getBitsMod (), 4));
            addLine (curChannelProperties.getChannelMode () != defaultChannelProperties.getChannelMode (), Parameter::Channel::ChannelModeId + " : " + juce::String (curChannelProperties.getChannelMode ()));
            addLine (curChannelProperties.getExpAM () != defaultChannelProperties.getExpAM (), Parameter::Channel::ExpAMId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getExpAM (), 4));
            addLine (curChannelProperties.getExpFM () != defaultChannelProperties.getExpFM (), Parameter::Channel::ExpFMId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getExpFM (), 4));
            addLine (! juce::exactlyEqual (curChannelProperties.getLevel (), defaultChannelProperties.getLevel ()), Parameter::Channel::LevelId + " : " + juce::String (curChannelProperties.getLevel (), 1));
            addLine (curChannelProperties.getLinAM () != defaultChannelProperties.getLinAM (), Parameter::Channel::LinAMId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getLinAM (), 4));
            addLine (curChannelProperties.getLinAMisExtEnv () != defaultChannelProperties.getLinAMisExtEnv (), Parameter::Channel::LinAMisExtEnvId + " : " + (curChannelProperties.getLinAMisExtEnv () ? "1" : "0"));
            addLine (curChannelProperties.getLinFM () != defaultChannelProperties.getLinFM (), Parameter::Channel::LinFMId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getLinFM (), 4));
            addLine (curChannelProperties.getLoopLengthIsEnd () != defaultChannelProperties.getLoopLengthIsEnd (), Parameter::Channel::LoopLengthIsEndId + " : " + (curChannelProperties.getLoopLengthIsEnd () ? "1" : "0"));
            addLine (curChannelProperties.getLoopLengthMod () != defaultChannelProperties.getLoopLengthMod (), Parameter::Channel::LoopLengthModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getLoopLengthMod (), 4));
            addLine (curChannelProperties.getLoopMode () != defaultChannelProperties.getLoopMode (), Parameter::Channel::LoopModeId + " : " + juce::String (curChannelProperties.getLoopMode ()));
            addLine (curChannelProperties.getLoopStartMod () != defaultChannelProperties.getLoopStartMod (), Parameter::Channel::LoopStartModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getLoopStartMod (), 4));
            addLine (! juce::exactlyEqual (curChannelProperties.getMixLevel (), defaultChannelProperties.getMixLevel ()), Parameter::Channel::MixLevelId + " : " + juce::String (curChannelProperties.getMixLevel ()));
            addLine (curChannelProperties.getMixMod () != defaultChannelProperties.getMixMod (), Parameter::Channel::MixModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getMixMod (), 4));
            addLine (curChannelProperties.getMixModIsFader () != defaultChannelProperties.getMixModIsFader (), Parameter::Channel::MixModIsFaderId + " : " + (curChannelProperties.getMixModIsFader () ? "1" : "0"));
            addLine (! juce::exactlyEqual (curChannelProperties.getPan (), defaultChannelProperties.getPan ()), Parameter::Channel::PanId + " : " + juce::String (curChannelProperties.getPan ()));
            addLine (curChannelProperties.getPanMod () != defaultChannelProperties.getPanMod (), Parameter::Channel::PanModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getPanMod (), 4));
            addLine (curChannelProperties.getPhaseCV () != defaultChannelProperties.getPhaseCV (), Parameter::Channel::PhaseCVId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getPhaseCV (), 4));
            addLine (! juce::exactlyEqual (curChannelProperties.getPitch (), defaultChannelProperties.getPitch ()), Parameter::Channel::PitchId + " : " + juce::String (curChannelProperties.getPitch ()));
            addLine (curChannelProperties.getPitchCV () != defaultChannelProperties.getPitchCV (), Parameter::Channel::PitchCVId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getPitchCV (), 4));
            addLine (curChannelProperties.getPlayMode () != defaultChannelProperties.getPlayMode (), Parameter::Channel::PlayModeId + " : " + juce::String (curChannelProperties.getPlayMode ()));
            addLine (! juce::exactlyEqual (curChannelProperties.getPMIndex (), defaultChannelProperties.getPMIndex ()), Parameter::Channel::PMIndexId + " : " + juce::String (curChannelProperties.getPMIndex ()));
            addLine (curChannelProperties.getPMIndexMod () != defaultChannelProperties.getPMIndexMod (), Parameter::Channel::PMIndexModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getPMIndexMod (), 4));
            addLine (curChannelProperties.getPMSource () != defaultChannelProperties.getPMSource (), Parameter::Channel::PMSourceId + " : " + juce::String (curChannelProperties.getPMSource ()));
            addLine (! juce::exactlyEqual (curChannelProperties.getRelease (), defaultChannelProperties.getRelease ()), Parameter::Channel::ReleaseId + " : " + juce::String (curChannelProperties.getRelease ()));
            addLine (curChannelProperties.getReleaseMod () != defaultChannelProperties.getReleaseMod (), Parameter::Channel::ReleaseModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getReleaseMod (), 4));
            addLine (curChannelProperties.getReverse () != defaultChannelProperties.getReverse (), Parameter::Channel::ReverseId + " : " + (curChannelProperties.getReverse () ? "1" : "0"));
            addLine (curChannelProperties.getSampleStartMod () != defaultChannelProperties.getSampleStartMod (), Parameter::Channel::SampleStartModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getSampleStartMod (), 4));
            addLine (curChannelProperties.getSampleEndMod () != defaultChannelProperties.getSampleEndMod (), Parameter::Channel::SampleEndModId + " : " + ChannelProperties::getCvInputAndValueString (curChannelProperties.getSampleEndMod (), 4));
            addLine (curChannelProperties.getSpliceSmoothing () != defaultChannelProperties.getSpliceSmoothing (), Parameter::Channel::SpliceSmoothingId + " : " + (curChannelProperties.getSpliceSmoothing () ? "1" : "0"));
            addLine (curChannelProperties.getXfadeGroup () != defaultChannelProperties.getXfadeGroup (), Parameter::Channel::XfadeGroupId + " : " + curChannelProperties.getXfadeGroup ());
            addLine (curChannelProperties.getZonesCV () != defaultChannelProperties.getZonesCV (), Parameter::Channel::ZonesCVId + " : " + curChannelProperties.getZonesCV ());
            addLine (curChannelProperties.getZonesRT () != defaultChannelProperties.getZonesRT (), Parameter::Channel::ZonesRTId + " : " + juce::String (curChannelProperties.getZonesRT ()));

            curChannelProperties.forEachZone ([&indentAmount, &addLine, &defaultZoneProperties] (juce::ValueTree zoneVT, int)
            {
                ZoneProperties curZoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                if (! curZoneProperties.getSample ().isEmpty ())
                {
                    addLine (true, Section::ZoneId + " " + juce::String (curZoneProperties.getId ()) + " :");
                    ++indentAmount;
                    addLine (! juce::exactlyEqual (curZoneProperties.getLevelOffset (), defaultZoneProperties.getLevelOffset ()), Parameter::Zone::LevelOffsetId + " : " + juce::String (curZoneProperties.getLevelOffset ()));
                    addLine (curZoneProperties.getLoopLength ().has_value () && curZoneProperties.getLoopLength () != defaultZoneProperties.getLoopLength (),
                             Parameter::Zone::LoopLengthId + " : " + juce::String (curZoneProperties.getLoopLength ().value_or (0)));
                    addLine (curZoneProperties.getLoopStart ().has_value () && curZoneProperties.getLoopStart () != defaultZoneProperties.getLoopStart (),
                             Parameter::Zone::LoopStartId + " : " + juce::String (curZoneProperties.getLoopStart ().value_or (0)));
                    addLine (! juce::exactlyEqual (curZoneProperties.getMinVoltage (), defaultZoneProperties.getMinVoltage ()), Parameter::Zone::MinVoltageId + " : " + juce::String (curZoneProperties.getMinVoltage ()));
                    addLine (! juce::exactlyEqual (curZoneProperties.getPitchOffset (), defaultZoneProperties.getPitchOffset ()), Parameter::Zone::PitchOffsetId + " : " + juce::String (curZoneProperties.getPitchOffset ()));
                    addLine (curZoneProperties.getSample () != defaultZoneProperties.getSample (), Parameter::Zone::SampleId + " : " + curZoneProperties.getSample ());
                    addLine (curZoneProperties.getSampleStart ().has_value () && curZoneProperties.getSampleStart () != defaultZoneProperties.getSampleStart (),
                             Parameter::Zone::SampleStartId + " : " + juce::String (curZoneProperties.getSampleStart ().value_or (0)));
                    addLine (curZoneProperties.getSampleEnd ().has_value () && curZoneProperties.getSampleEnd () != defaultZoneProperties.getSampleEnd (),
                             Parameter::Zone::SampleEndId + " : " + juce::String (curZoneProperties.getSampleEnd ().value_or (0)));
                    addLine (curZoneProperties.getSide () != defaultZoneProperties.getSide (), Parameter::Zone::SideId + " : " + juce::String (curZoneProperties.getSide ()));
                    --indentAmount;
                }
                return true;
            });
            --indentAmount;
        }
        return true;
    });

    // write data out to preset file
    const auto stringToWrite { lines.joinIntoString ("\r\n") };
    presetFile.replaceWithText (stringToWrite);
}

void Assimil8orPreset::write (juce::File presetFile)
{
    write (presetFile, presetProperties.getValueTree ());
}

void Assimil8orPreset::parse (juce::StringArray presetLines)
{
    LogParsing ("Assimil8orPreset::parse - enter");
    jassert (presetProperties.isValid ());
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    parseErrorList.removeAllChildren (nullptr);
    parseErrorList.removeAllProperties (nullptr);

    while (! undoActionsStack.empty ())
        undoActionsStack.pop ();

    curActions = &globalActions;
    curPresetSection = {};
    channelProperties = {};
    curChannelSection = {};
    zoneProperties = {};
    curZoneSection = {};

    for (const auto& presetLine : presetLines)
    {
        LogParsing ("parsing - " + presetLine.trimStart ());
        if (presetLine.trim ().isEmpty ())
            continue;

        key = presetLine.upToFirstOccurrenceOf (":", false, false).trim ();
        value = presetLine.fromFirstOccurrenceOf (":", false, false).trim ();
        const auto paramName { key.upToFirstOccurrenceOf (" ", false, false) };

        auto findActionMapContaining = [this, &paramName] () -> ActionMap*
        {
            auto actionMapToCheck { curActions };
            while (actionMapToCheck != nullptr)
            {
                if (actionMapToCheck->find (paramName) != actionMapToCheck->end ())
                    return actionMapToCheck;

                if (actionMapToCheck == &zoneActions)
                    actionMapToCheck = &channelActions;
                else if (actionMapToCheck == &channelActions)
                    actionMapToCheck = &presetActions;
                else if (actionMapToCheck == &presetActions)
                    actionMapToCheck = &globalActions;
                else
                    actionMapToCheck = nullptr;
            }
            return nullptr;
        };

        if (auto actionMap { findActionMapContaining () }; actionMap != nullptr)
        {
            while (curActions != actionMap)
            {
                jassert (! undoActionsStack.empty ());
                if (undoActionsStack.empty ())
                    break;

                auto undoAction { undoActionsStack.top () };
                undoActionsStack.pop ();
                undoAction ();
            }

            if (curActions == actionMap)
                actionMap->find (paramName)->second ();
        }
        else
        {
            const auto unknownParameterError { "unknown parameter: " + key };
            LogParsing (unknownParameterError);
            juce::ValueTree newParseError ("ParseError");
            newParseError.setProperty ("type", "UnknownParameterError", nullptr);
            newParseError.setProperty ("description", unknownParameterError, nullptr);
            parseErrorList.addChild (newParseError, -1, nullptr);
        }
    }
    LogParsing ("Assimil8orPreset::parse - exit");
}

juce::String Assimil8orPreset::getSectionName ()
{
    if (curActions == &globalActions)
        return "Global";
    else if (curActions == &presetActions)
        return "Preset";
    else if (curActions == &channelActions)
        return "Channel";
    else if (curActions == &zoneActions)
        return "Zone";
    else
        // curActions is not pointing to an action map!
        jassertfalse;
        return "";
}

void Assimil8orPreset::checkCvInputAndAmountFormat (juce::String theKey, juce::String theValue)
{
    const auto delimiterLocation { theValue.indexOfChar (0, ' ') };
    if (delimiterLocation <= 0 || delimiterLocation >= theValue.length () - 1)
    {
        const auto parameterFormatError { juce::String ("value '") + theValue + "' for parameter '" + theKey + "' - incorrect format" };
        LogParsing (parameterFormatError);
        juce::ValueTree newParseError ("ParseError");
        newParseError.setProperty ("type", "ParameterFormatError", nullptr);
        newParseError.setProperty ("description", parameterFormatError, nullptr);
        parseErrorList.addChild (newParseError, -1, nullptr);
    }
}

void Assimil8orPreset::initParser ()
{
    auto getParameterId = [this] ()
    {
        const auto id { key.fromFirstOccurrenceOf (" ", false, false).getIntValue () };
        jassert (id > 0);
        return id;
    };

    auto setActions = [this] (ActionMap* newActions, Action undoAction)
    {
        curActions = newActions;
        undoActionsStack.push (undoAction);
    };

    auto undoAction = [this] (ActionMap* newActions, juce::ValueTree & sectionToRevert)
    {
        curActions = newActions;
        sectionToRevert = {};
    };

    // Global Action
    globalActions.insert ({
        {Section::PresetId, [this, getParameterId, undoAction, setActions] () {
            curPresetSection = presetProperties.getValueTree ();
            presetProperties.setId (getParameterId (), false);
            setActions (&presetActions, [this, undoAction]{
                undoAction (&globalActions, curPresetSection);
            });
        }}
    });

    // Preset Actions
    presetActions.insert ({
        {Section::ChannelId, [this, getParameterId, undoAction, setActions] () {
            jassert (getParameterId () > 0 && getParameterId () < 9);
            curChannelSection = presetProperties.getChannelVT (getParameterId () - 1);
            channelProperties.wrap (curChannelSection, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            setActions (&channelActions, [this, undoAction]{
                undoAction (&presetActions, curChannelSection);
            });
        }},
        {Parameter::Preset::NameId, [this] () {
            // validate max length
            // validate characters
            presetProperties.setName (value, false);
        }},
        {Parameter::Preset::MidiSetupId, [this] () {
            presetProperties.setMidiSetup (value.getIntValue (), false);
        }},
        {Parameter::Preset::Data2asCVId, [this] () {
            // validate 0/1
            presetProperties.setData2AsCV (value, false);
        }},
        {Parameter::Preset::XfadeACVId, [this] () {
            // validate global cv input
            presetProperties.setXfadeACV (value, false);
        }},
        {Parameter::Preset::XfadeAWidthId, [this] () {
            // validate -1 to +1
            presetProperties.setXfadeAWidth (value.getDoubleValue (), false);
        }},
        {Parameter::Preset::XfadeBCVId, [this] () {
            // validate global cv input
            presetProperties.setXfadeBCV (value, false);
        }},
        {Parameter::Preset::XfadeBWidthId, [this] () {
            presetProperties.setXfadeBWidth (value.getDoubleValue (), false);
        }},
        {Parameter::Preset::XfadeCCVId, [this] () {
            // validate global cv input
            presetProperties.setXfadeCCV (value, false);
        }},
        {Parameter::Preset::XfadeCWidthId, [this] () {
            presetProperties.setXfadeCWidth (value.getDoubleValue (), false);
        }},
        {Parameter::Preset::XfadeDCVId, [this] () {
            // validate global cv input
            presetProperties.setXfadeDCV (value, false);
        }},
        {Parameter::Preset::XfadeDWidthId, [this] () {
            presetProperties.setXfadeDWidth (value.getDoubleValue (), false);
        }}
    });

    // Channel Actions
    channelActions.insert ({
        {Section::ZoneId, [this, getParameterId, undoAction, setActions] () {
            jassert (getParameterId () > 0 && getParameterId () < 9);
            curZoneSection = channelProperties.getZoneVT (getParameterId () - 1);
            zoneProperties.wrap (curZoneSection, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
            setActions (&zoneActions, [this, undoAction]{
                undoAction (&channelActions, curZoneSection);
            });
        }},
        {Parameter::Channel::AttackId, [this] () {
            channelProperties.setAttack (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::AttackFromCurrentId, [this] () {
            channelProperties.setAttackFromCurrent (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::AttackModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, attackModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setAttackMod (cvInput, attackModAmount, false);
        }},
        {Parameter::Channel::AliasingId, [this] () {
            channelProperties.setAliasing (value.getIntValue (), false);
        }},
        {Parameter::Channel::AliasingModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, aliasingModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setAliasingMod (cvInput, aliasingModAmount, false);
        }},
        {Parameter::Channel::AutoTriggerId, [this] () {
            channelProperties.setAutoTrigger (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::BitsId, [this] () {
            channelProperties.setBits (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::BitsModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, bitsModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setBitsMod (cvInput, bitsModAmount, false);
        }},
        {Parameter::Channel::ChannelModeId, [this] () {
            channelProperties.setChannelMode (value.getIntValue (), false);
        }},
        {Parameter::Channel::ExpAMId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, expAMAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setExpAM (cvInput, expAMAmount, false);
        }},
        {Parameter::Channel::ExpFMId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, expFMAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setExpFM (cvInput, expFMAmount, false);
        }},
        {Parameter::Channel::LevelId, [this] () {
            channelProperties.setLevel (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::LinAMId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, linAMAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setLinAM (cvInput, linAMAmount, false);
        }},
        {Parameter::Channel::LinAMisExtEnvId, [this] () {
            channelProperties.setLinAMisExtEnv (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::LinFMId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, linFMAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setLinFM (cvInput, linFMAmount, false);
        }},
        {Parameter::Channel::LoopLengthIsEndId, [this] () {
            channelProperties.setLoopLengthIsEnd (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::LoopLengthModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, loopLengthModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setLoopLengthMod (cvInput, loopLengthModAmount, false);
        }},
        {Parameter::Channel::LoopModeId, [this] () {
            channelProperties.setLoopMode (value.getIntValue (), false);
        }},
        {Parameter::Channel::LoopStartModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, loopStartModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setLoopStartMod (cvInput, loopStartModAmount, false);
        }},
        {Parameter::Channel::MixLevelId, [this] () {
            channelProperties.setMixLevel (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::MixModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, mixModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setMixMod (cvInput, mixModAmount, false);
        }},
        {Parameter::Channel::MixModIsFaderId, [this] () {
            channelProperties.setMixModIsFader (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::PanId, [this] () {
            channelProperties.setPan (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::PanModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, panModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setPanMod (cvInput, panModAmount, false);
        }},
        {Parameter::Channel::PhaseCVId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, phaseCvAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setPhaseCV (cvInput, phaseCvAmount, false);
        }},
        {Parameter::Channel::PitchId, [this] () {
            channelProperties.setPitch (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::PitchCVId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, pitchCvAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setPitchCV (cvInput, pitchCvAmount, false);
        }},
        {Parameter::Channel::PlayModeId, [this] () {
            channelProperties.setPlayMode (value.getIntValue (), false);
        }},
        {Parameter::Channel::PMIndexId, [this] () {
            channelProperties.setPMIndex (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::PMIndexModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, pMIndexModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setPMIndexMod (cvInput, pMIndexModAmount, false);
        }},
        {Parameter::Channel::PMSourceId, [this] () {
            channelProperties.setPMSource (value.getIntValue (), false);
        }},
        {Parameter::Channel::ReleaseId, [this] () {
            channelProperties.setRelease (value.getDoubleValue (), false);
        }},
        { Parameter::Channel::ReleaseModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, releaseModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setReleaseMod (cvInput, releaseModAmount, false);
        }},
        {Parameter::Channel::ReverseId, [this] () {
            channelProperties.setReverse (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::SampleEndModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, sampleEndModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setSampleEndMod (cvInput, sampleEndModAmount, false);
        }},
        { Parameter::Channel::SampleStartModId, [this] () {
            checkCvInputAndAmountFormat (key, value);
            const auto [cvInput, sampleStartModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setSampleStartMod (cvInput, sampleStartModAmount, false);
        }},
        {Parameter::Channel::SpliceSmoothingId, [this] () {
            channelProperties.setSpliceSmoothing (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::XfadeGroupId, [this] () {
            channelProperties.setXfadeGroup (value, false);
        }},
        {Parameter::Channel::ZonesCVId, [this] () {
            channelProperties.setZonesCV (value, false);
        }},
        {Parameter::Channel::ZonesRTId, [this] () {
            channelProperties.setZonesRT (value.getIntValue (), false);
        }}
    });

    // Zone Actions
    zoneActions.insert ({
        {Parameter::Zone::LevelOffsetId, [this] () {
            zoneProperties.setLevelOffset (value.getDoubleValue (), false);
        }},
        {Parameter::Zone::LoopLengthId, [this] () {
            zoneProperties.setLoopLength (value.getDoubleValue (), false);
        }},
        {Parameter::Zone::LoopStartId, [this] () {
            zoneProperties.setLoopStart (value.getIntValue (), false);
        }},
        {Parameter::Zone::MinVoltageId, [this] () {
            zoneProperties.setMinVoltage (value.getDoubleValue (), false);
        }},
        {Parameter::Zone::PitchOffsetId, [this] () {
            zoneProperties.setPitchOffset (value.getDoubleValue (), false);
        }},
        {Parameter::Zone::SampleId, [this] () {
            zoneProperties.setSample (value, false);
        }},
        {Parameter::Zone::SampleStartId, [this] () {
            zoneProperties.setSampleStart (value.getIntValue (), false);
        }},
        {Parameter::Zone::SampleEndId, [this] () {
            zoneProperties.setSampleEnd (value.getIntValue (), false);
        }},
        {Parameter::Zone::SideId, [this] () {
            zoneProperties.setSide (value.getIntValue (), false);
        }}
    });
}
