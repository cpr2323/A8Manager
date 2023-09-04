#include "Assimil8orPreset.h"
#include "Assimil8orValidator.h"
#include "FileTypeHelpers.h"
#include "Preset/ParameterNames.h"
#include "Preset/ParameterPresetsSingleton.h"
// TODO - we should not include something from the UI here, so let's move the FormatHelpers elsewhere
#include "../GUI/Assimil8or/Editor/FormatHelpers.h"

#define LOG_PARSING 0
#if LOG_PARSING
#define LogParsing(text) juce::Logger::outputDebugString (text);
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
    auto addLine = [this, &indentAmount, &lines] (bool doAdd, juce::String lineToAdd)
    {
        if (doAdd)
            lines.add (juce::String (lineToAdd).paddedLeft (' ', lineToAdd.length () + indentAmount * 2));
    };

    addLine (true, Section::PresetId + " " + juce::String (presetPropertiesToWrite.getId ()) + " :");
    ++indentAmount;
    addLine (true, Parameter::Preset::NameId + " : " + presetPropertiesToWrite.getName ());
    addLine (presetPropertiesToWrite.getData2AsCV () != defaultPresetProperties.getData2AsCV (), Parameter::Preset::Data2asCVId + " : " + presetPropertiesToWrite.getData2AsCV ());
    addLine (presetPropertiesToWrite.getXfadeACV () != defaultPresetProperties.getXfadeACV (), Parameter::Preset::XfadeACVId + " : " + presetPropertiesToWrite.getXfadeACV ());
    addLine (presetPropertiesToWrite.getXfadeAWidth () != defaultPresetProperties.getXfadeAWidth (), Parameter::Preset::XfadeAWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeAWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeBCV () != defaultPresetProperties.getXfadeBCV (), Parameter::Preset::XfadeBCVId + " : " + presetPropertiesToWrite.getXfadeBCV ());
    addLine (presetPropertiesToWrite.getXfadeBWidth () != defaultPresetProperties.getXfadeBWidth (), Parameter::Preset::XfadeBWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeBWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeCCV () != defaultPresetProperties.getXfadeCCV (), Parameter::Preset::XfadeCCVId + " : " + presetPropertiesToWrite.getXfadeCCV ());
    addLine (presetPropertiesToWrite.getXfadeCWidth () != defaultPresetProperties.getXfadeCWidth (), Parameter::Preset::XfadeCWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeCWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeDCV () != defaultPresetProperties.getXfadeDCV (), Parameter::Preset::XfadeDCVId + " : " + presetPropertiesToWrite.getXfadeDCV ());
    addLine (presetPropertiesToWrite.getXfadeDWidth () != defaultPresetProperties.getXfadeDWidth (), Parameter::Preset::XfadeDWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeDWidth (), 2));

    presetPropertiesToWrite.forEachChannel ([this, &addLine, &indentAmount, &defaultZoneProperties, &defaultChannelProperties] (juce::ValueTree channelVT)
    {
        ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        ZoneProperties zoneProperties (channelProperties.getZoneVT(0), ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        if (! zoneProperties.getSample().isEmpty ())
        {
            addLine (true, Section::ChannelId + " " + juce::String (channelProperties.getId ()) + " :");
            ++indentAmount;
            addLine (channelProperties.getAliasing () != defaultChannelProperties.getAliasing (), Parameter::Channel::AliasingId + " : " + juce::String (channelProperties.getAliasing ()));
            addLine (channelProperties.getAliasingMod () != defaultChannelProperties.getAliasingMod (), Parameter::Channel::AliasingModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getAliasingMod (), 4));
            addLine (channelProperties.getAttack () != defaultChannelProperties.getAttack (), Parameter::Channel::AttackId + " : " + juce::String (channelProperties.getAttack ()));
            addLine (channelProperties.getAttackFromCurrent () != defaultChannelProperties.getAttackFromCurrent (), Parameter::Channel::AttackFromCurrentId + " : " + (channelProperties.getAttackFromCurrent () ? "1" : "0"));
            addLine (channelProperties.getAttackMod () != defaultChannelProperties.getAttackMod (), Parameter::Channel::AttackModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getAttackMod (), 4));
            addLine (channelProperties.getAutoTrigger () != defaultChannelProperties.getAutoTrigger (), Parameter::Channel::AutoTriggerId + " : " + (channelProperties.getAutoTrigger () ? "1" : "0"));
            addLine (channelProperties.getBits () != defaultChannelProperties.getBits (), Parameter::Channel::BitsId + " : " + juce::String (channelProperties.getBits (), 1));
            addLine (channelProperties.getBitsMod () != defaultChannelProperties.getBitsMod (), Parameter::Channel::BitsModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getBitsMod (), 4));
            addLine (channelProperties.getChannelMode () != defaultChannelProperties.getChannelMode (), Parameter::Channel::ChannelModeId + " : " + juce::String (channelProperties.getChannelMode ()));
            addLine (channelProperties.getExpAM () != defaultChannelProperties.getExpAM (), Parameter::Channel::ExpAMId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getExpAM (), 4));
            addLine (channelProperties.getExpFM () != defaultChannelProperties.getExpFM (), Parameter::Channel::ExpFMId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getExpFM (), 4));
            addLine (channelProperties.getLevel () != defaultChannelProperties.getLevel (), Parameter::Channel::LevelId + " : " + juce::String (channelProperties.getLevel ()));
            addLine (channelProperties.getLinAM () != defaultChannelProperties.getLinAM (), Parameter::Channel::LinAMId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLinAM (), 4));
            addLine (channelProperties.getLinAMisExtEnv () != defaultChannelProperties.getLinAMisExtEnv (), Parameter::Channel::LinAMisExtEnvId + " : " + (channelProperties.getLinAMisExtEnv () ? "1" : "0"));
            addLine (channelProperties.getLinFM () != defaultChannelProperties.getLinFM (), Parameter::Channel::LinFMId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLinFM (), 4));
            addLine (channelProperties.getLoopLengthIsEnd () != defaultChannelProperties.getLoopLengthIsEnd (), Parameter::Channel::LoopLengthIsEndId + " : " + (channelProperties.getLoopLengthIsEnd () ? "1" : "0"));
            addLine (channelProperties.getLoopLengthMod () != defaultChannelProperties.getLoopLengthMod (), Parameter::Channel::LoopLengthModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLoopLengthMod (), 4));
            addLine (channelProperties.getLoopMode () != defaultChannelProperties.getLoopMode (), Parameter::Channel::LoopModeId + " : " + juce::String (channelProperties.getLoopMode ()));
            addLine (channelProperties.getLoopStartMod () != defaultChannelProperties.getLoopStartMod (), Parameter::Channel::LoopStartModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLoopStartMod (), 4));
            addLine (channelProperties.getMixLevel () != defaultChannelProperties.getMixLevel (), Parameter::Channel::MixLevelId + " : " + juce::String (channelProperties.getMixLevel ()));
            addLine (channelProperties.getMixMod () != defaultChannelProperties.getMixMod (), Parameter::Channel::MixModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getMixMod (), 4));
            addLine (channelProperties.getMixModIsFader () != defaultChannelProperties.getMixModIsFader (), Parameter::Channel::MixModIsFaderId + " : " + (channelProperties.getMixModIsFader () ? "1" : "0"));
            addLine (channelProperties.getPan () != defaultChannelProperties.getPan (), Parameter::Channel::PanId + " : " + juce::String (channelProperties.getPan ()));
            addLine (channelProperties.getPanMod () != defaultChannelProperties.getPanMod (), Parameter::Channel::PanModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPanMod (), 4));
            addLine (channelProperties.getPhaseCV () != defaultChannelProperties.getPhaseCV (), Parameter::Channel::PhaseCVId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPhaseCV (), 4));
            addLine (channelProperties.getPitch () != defaultChannelProperties.getPitch (), Parameter::Channel::PitchId + " : " + juce::String (channelProperties.getPitch ()));
            addLine (channelProperties.getPitchCV () != defaultChannelProperties.getPitchCV (), Parameter::Channel::PitchCVId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPitchCV (), 4));
            addLine (channelProperties.getPlayMode () != defaultChannelProperties.getPlayMode (), Parameter::Channel::PlayModeId + " : " + juce::String (channelProperties.getPlayMode ()));
            addLine (channelProperties.getPMIndex () != defaultChannelProperties.getPMIndex (), Parameter::Channel::PMIndexId + " : " + juce::String (channelProperties.getPMIndex ()));
            addLine (channelProperties.getPMIndexMod () != defaultChannelProperties.getPMIndexMod (), Parameter::Channel::PMIndexModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPMIndexMod (), 4));
            addLine (channelProperties.getPMSource () != defaultChannelProperties.getPMSource (), Parameter::Channel::PMSourceId + " : " + juce::String (channelProperties.getPMSource ()));
            addLine (channelProperties.getRelease () != defaultChannelProperties.getRelease (), Parameter::Channel::ReleaseId + " : " + juce::String (channelProperties.getRelease ()));
            addLine (channelProperties.getReleaseMod () != defaultChannelProperties.getReleaseMod (), Parameter::Channel::ReleaseModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getReleaseMod (), 4));
            addLine (channelProperties.getReverse () != defaultChannelProperties.getReverse (), Parameter::Channel::ReverseId + " : " + (channelProperties.getReverse () ? "1" : "0"));
            addLine (channelProperties.getSampleStartMod () != defaultChannelProperties.getSampleStartMod (), Parameter::Channel::SampleStartModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getSampleStartMod (), 4));
            addLine (channelProperties.getSampleEndMod () != defaultChannelProperties.getSampleEndMod (), Parameter::Channel::SampleEndModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getSampleEndMod (), 4));
            addLine (channelProperties.getSpliceSmoothing () != defaultChannelProperties.getSpliceSmoothing (), Parameter::Channel::SpliceSmoothingId + " : " + (channelProperties.getSpliceSmoothing () ? "1" : "0"));
            addLine (channelProperties.getXfadeGroup () != defaultChannelProperties.getXfadeGroup (), Parameter::Channel::XfadeGroupId + " : " + channelProperties.getXfadeGroup ());
            addLine (channelProperties.getZonesCV () != defaultChannelProperties.getZonesCV (), Parameter::Channel::ZonesCVId + " : " + channelProperties.getZonesCV ());
            addLine (channelProperties.getZonesRT () != defaultChannelProperties.getZonesRT (), Parameter::Channel::ZonesRTId + " : " + juce::String (channelProperties.getZonesRT ()));

            channelProperties.forEachZone ([this, &indentAmount, &addLine, &defaultZoneProperties] (juce::ValueTree zoneVT)
            {
                ZoneProperties zoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                if (! zoneProperties.getSample().isEmpty ())
                {
                    addLine (true, Section::ZoneId + " " + juce::String (zoneProperties.getId ()) + " :");
                    ++indentAmount;
                    addLine (zoneProperties.getLevelOffset () != defaultZoneProperties.getLevelOffset (), Parameter::Zone::LevelOffsetId + " : " + juce::String (zoneProperties.getLevelOffset ()));
                    addLine (zoneProperties.getLoopLength ().has_value () && zoneProperties.getLoopLength () != defaultZoneProperties.getLoopLength (),
                             Parameter::Zone::LoopLengthId + " : " + juce::String (zoneProperties.getLoopLength ().value_or (0)));
                    addLine (zoneProperties.getLoopStart ().has_value () && zoneProperties.getLoopStart () != defaultZoneProperties.getLoopStart (),
                             Parameter::Zone::LoopStartId + " : " + juce::String (zoneProperties.getLoopStart ().value_or (0)));
                    addLine (zoneProperties.getMinVoltage () != defaultZoneProperties.getMinVoltage (), Parameter::Zone::MinVoltageId + " : " + juce::String (zoneProperties.getMinVoltage ()));
                    addLine (zoneProperties.getPitchOffset () != defaultZoneProperties.getPitchOffset (), Parameter::Zone::PitchOffsetId + " : " + juce::String (zoneProperties.getPitchOffset ()));
                    addLine (zoneProperties.getSample () != defaultZoneProperties.getSample (), Parameter::Zone::SampleId + " : " + zoneProperties.getSample ());
                    addLine (zoneProperties.getSampleStart ().has_value () && zoneProperties.getSampleStart () != defaultZoneProperties.getSampleStart (),
                             Parameter::Zone::SampleStartId + " : " + juce::String (zoneProperties.getSampleStart ().value_or (0)));
                    addLine (zoneProperties.getSampleEnd ().has_value () && zoneProperties.getSampleEnd () != defaultZoneProperties.getSampleEnd (),
                             Parameter::Zone::SampleEndId + " : " + juce::String (zoneProperties.getSampleEnd ().value_or (0)));
                    addLine (zoneProperties.getSide () != defaultZoneProperties.getSide (), Parameter::Zone::SideId + " : " + juce::String (zoneProperties.getSide ()));
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
    jassert (presetProperties.isValid ());
    PresetProperties::copyTreeProperties (ParameterPresetsSingleton::getInstance ()->getParameterPresetListProperties ().getParameterPreset (ParameterPresetListProperties::DefaultParameterPresetType),
                                          presetProperties.getValueTree ());
    parseErrorList.removeAllChildren (nullptr);
    parseErrorList.removeAllProperties (nullptr);

    auto scopeDepth { 0 };

    curActions = &globalActions;
    curPresetSection = {};
    channelProperties = {};
    curChannelSection = {};
    zoneProperties = {};
    curZoneSection = {};

    for (const auto& presetLine : presetLines)
    {
        const auto indent { presetLine.initialSectionContainingOnly (" ") };
        const auto previousScopeDepth { scopeDepth };
        scopeDepth = indent.length () / 2;
        const auto scopeDifference { scopeDepth - previousScopeDepth };
        // check if we are exiting scope(s)
        if (scopeDifference < 0)
        {
            // for each scope we are exiting, reset the appropriate parent objects
            for (auto remainingScopes { scopeDifference * -1 }; remainingScopes > 0; --remainingScopes)
            {
                // undoActionsStack should have items to reset
                jassert (! undoActionsStack.empty ());
                auto undoAction = undoActionsStack.top ();
                undoActionsStack.pop ();
                undoAction ();
            }
        }

        LogParsing (juce::String (scopeDepth) + "-" + presetLine.trimStart ());
        if (presetLine.trim ().isEmpty ())
            continue;

        key = presetLine.upToFirstOccurrenceOf (":", false, false).trim ();
        value = presetLine.fromFirstOccurrenceOf (":", false, false).trim ();
        const auto paramName { key.upToFirstOccurrenceOf (" ", false, false) };
        if (const auto action { curActions->find (paramName) }; action != curActions->end ())
        {
            action->second ();
        }
        else
        {
            const auto unknownParameterError { "unknown " + getSectionName () + " parameter: " + key };
            LogParsing (unknownParameterError);
            juce::ValueTree newParseError ("ParseError");
            newParseError.setProperty ("type", "UnknownParameterError", nullptr);
            newParseError.setProperty ("description", unknownParameterError, nullptr);
            parseErrorList.addChild (newParseError, -1, nullptr);
        }
    }
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
    if (delimiterLocation == 0)
    {
        const auto parameterFormatError { juce::String ("value '") + theValue + "' for parameter '" + theKey + "' - incorrect format" };
        LogParsing (unknownParameterError);
        juce::ValueTree newParseError ("ParseError");
        newParseError.setProperty ("type", "ParameterFormatError", nullptr);
        newParseError.setProperty ("description", parameterFormatError, nullptr);
        parseErrorList.addChild (newParseError, -1, nullptr);
        jassertfalse;
    }
};

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
    
    auto undoAction = [this, setActions] (ActionMap* newActions, juce::ValueTree & sectionToRevert)
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
