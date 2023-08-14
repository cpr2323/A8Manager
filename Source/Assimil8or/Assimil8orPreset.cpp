#include "Assimil8orPreset.h"
#include "Assimil8orValidator.h"
#include "FileTypeHelpers.h"
#include "Preset/ParameterNames.h"

#define LOG_PARSING 0
#if LOG_PARSING
#define LogParsing(text) juce::Logger::outputDebugString (text);
#else
#define LogParsing(text) ;
#endif

void Assimil8orPreset::write (juce::File presetFile, juce::ValueTree presetPropertiesVT)
{
    jassert (presetPropertiesVT.isValid ());

    PresetProperties presetPropertiesToWrite (presetPropertiesVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
    if (FileTypeHelpers::isPresetFile (presetFile))
    {
        if (const auto presetNumber { FileTypeHelpers::getPresetNumberFromName (presetFile) }; presetNumber != FileTypeHelpers::kBadPresetNumber)
            presetPropertiesToWrite.setIndex (presetNumber, false);
    }

    auto indentAmount { 0 };
    juce::StringArray lines;
    auto hasContent = [] (juce::ValueTree vt)
    {
        auto doChildrenHaveContent = [&vt] ()
        {
            auto contentFound { false };
            ValueTreeHelpers::forEachChild (vt, [&contentFound] (juce::ValueTree child)
            {
                if (child.getNumProperties () != 0)
                {
                    contentFound = true;
                    return false;
                }
                return true;
            });
            return contentFound;
        };
        if (vt.getNumProperties () > 1)
            return true;
        if (vt.getNumChildren () == 0)
            return false;
        return doChildrenHaveContent ();
    };
    auto addLine = [this, &indentAmount, &lines] (bool doAdd, juce::String lineToAdd)
    {
        if (doAdd)
            lines.add (juce::String (lineToAdd).paddedLeft (' ', lineToAdd.length () + indentAmount * 2));
    };

    addLine (true, Section::PresetId + " " + juce::String (presetPropertiesToWrite.getIndex ()) + " :");
    ++indentAmount;
    addLine (true, Parameter::Preset::NameId + " : " + presetPropertiesToWrite.getName ());
    addLine (presetPropertiesToWrite.getData2AsCV () != presetPropertiesToWrite.getData2AsCVDefault(), Parameter::Preset::Data2asCVId+ " : " + presetPropertiesToWrite.getData2AsCV ());
    addLine (presetPropertiesToWrite.getXfadeACV () != presetPropertiesToWrite.getXfadeACVDefault (), Parameter::Preset::XfadeACVId + " : " + presetPropertiesToWrite.getXfadeACV ());
    addLine (presetPropertiesToWrite.getXfadeAWidth () != presetPropertiesToWrite.getXfadeAWidthDefault (), Parameter::Preset::XfadeAWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeAWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeBCV () != presetPropertiesToWrite.getXfadeBCVDefault (), Parameter::Preset::XfadeBCVId + " : " + presetPropertiesToWrite.getXfadeBCV ());
    addLine (presetPropertiesToWrite.getXfadeBWidth () != presetPropertiesToWrite.getXfadeBWidthDefault (), Parameter::Preset::XfadeBWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeBWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeCCV () != presetPropertiesToWrite.getXfadeCCVDefault (), Parameter::Preset::XfadeCCVId + " : " + presetPropertiesToWrite.getXfadeCCV ());
    addLine (presetPropertiesToWrite.getXfadeCWidth () != presetPropertiesToWrite.getXfadeCWidthDefault (), Parameter::Preset::XfadeCWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeCWidth (), 2));
    addLine (presetPropertiesToWrite.getXfadeDCV () != presetPropertiesToWrite.getXfadeDCVDefault (), Parameter::Preset::XfadeDCVId + " : " + presetPropertiesToWrite.getXfadeDCV ());
    addLine (presetPropertiesToWrite.getXfadeDWidth () != presetPropertiesToWrite.getXfadeDWidthDefault (), Parameter::Preset::XfadeDWidthId + " : " + juce::String (presetPropertiesToWrite.getXfadeDWidth (), 2));

    presetPropertiesToWrite.forEachChannel ([this, &hasContent, &addLine, &indentAmount, &presetPropertiesToWrite] (juce::ValueTree channelVT)
    {
        ChannelProperties channelProperties (channelVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        if (hasContent (channelVT))
        {
            const auto channelPropertiesVT { channelProperties.getValueTree () };
            addLine (true, Section::ChannelId + " " + juce::String (channelProperties.getIndex ()) + " :");
            ++indentAmount;
            addLine (channelProperties.getAliasing () != channelProperties.getAliasingDefault (), Parameter::Channel::AliasingId + " : " + juce::String (channelProperties.getAliasing ()));
            addLine (channelProperties.getAliasingMod () != channelProperties.getAliasingModDefault (), Parameter::Channel::AliasingModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getAliasingMod (), 4));
            addLine (channelProperties.getAttack () != channelProperties.getAttackDefault (), Parameter::Channel::AttackId + " : " + juce::String (channelProperties.getAttack ()));
            addLine (channelProperties.getAttackFromCurrent () != channelProperties.getAttackFromCurrentDefault (), Parameter::Channel::AttackFromCurrentId + " : " + (channelProperties.getAttackFromCurrent () ? "1" : "0"));
            addLine (channelProperties.getAttackMod () != channelProperties.getAttackModDefault (), Parameter::Channel::AttackModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getAttackMod (), 4));
            addLine (channelProperties.getAutoTrigger () != channelProperties.getAutoTriggerDefault (), Parameter::Channel::AutoTriggerId + " : " + (channelProperties.getAutoTrigger () ? "1" : "0"));
            addLine (channelProperties.getBits () != channelProperties.getBitsDefault (), Parameter::Channel::BitsId + " : " + juce::String (channelProperties.getBits (), 1));
            addLine (channelProperties.getBitsMod () != channelProperties.getBitsModDefault (), Parameter::Channel::BitsModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getBitsMod (), 4));
            addLine (channelProperties.getChannelMode () != channelProperties.getChannelModeDefault (), Parameter::Channel::ChannelModeId + " : " + juce::String (channelProperties.getChannelMode ()));
            addLine (channelProperties.getExpAM () != channelProperties.getExpAMDefault (), Parameter::Channel::ExpAMId + " : " + juce::String (channelProperties.getExpAM ()));
            addLine (channelProperties.getExpFM () != channelProperties.getExpFMDefault (), Parameter::Channel::ExpFMId + " : " + juce::String (channelProperties.getExpFM ()));
            addLine (channelProperties.getLevel () != channelProperties.getLevelDefault (), Parameter::Channel::LevelId + " : " + juce::String (channelProperties.getLevel ()));
            addLine (channelProperties.getLinAM () != channelProperties.getLinAMDefault (), Parameter::Channel::LinAMId + " : " + juce::String (channelProperties.getLinAM ()));
            addLine (channelProperties.getLinAMisExtEnv () != channelProperties.getLinAMisExtEnvDefault (), Parameter::Channel::LinAMisExtEnvId + " : " + (channelProperties.getLinAMisExtEnv () ? "1" : "0"));
            addLine (channelProperties.getLinFM () != channelProperties.getLinFMDefault (), Parameter::Channel::LinFMId + " : " + juce::String (channelProperties.getLinFM ()));
            addLine (channelProperties.getLoopLengthMod () != channelProperties.getLoopLengthModDefault (), Parameter::Channel::LoopLengthModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLoopLengthMod (), 4));
            addLine (channelProperties.getLoopMode () != channelProperties.getLoopModeDefault (), Parameter::Channel::LoopModeId + " : " + juce::String (channelProperties.getLoopMode ()));
            addLine (channelProperties.getLoopStartMod () != channelProperties.getLoopStartModDefault (), Parameter::Channel::LoopStartModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getLoopStartMod (), 4));
            addLine (channelProperties.getMixLevel () != channelProperties.getMixLevelDefault (), Parameter::Channel::MixLevelId + " : " + juce::String (channelProperties.getMixLevel ()));
            addLine (channelProperties.getMixMod () != channelProperties.getMixModDefault (), Parameter::Channel::MixModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getMixMod (), 4));
            addLine (channelProperties.getMixModIsFader () != channelProperties.getMixModIsFaderDefault (), Parameter::Channel::MixModIsFaderId + " : " + (channelProperties.getMixModIsFader () ? "1" : "0"));
            addLine (channelProperties.getPan () != channelProperties.getPanDefault (), Parameter::Channel::PanId + " : " + juce::String (channelProperties.getPan ()));
            addLine (channelProperties.getPanMod () != channelProperties.getPanModDefault (), Parameter::Channel::PanModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPanMod (), 4));
            addLine (channelProperties.getPhaseCV () != channelProperties.getPhaseCVDefault (), Parameter::Channel::PhaseCVId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPhaseCV (), 4));
            addLine (channelProperties.getPitch () != channelProperties.getPitchDefault (), Parameter::Channel::PitchId + " : " + juce::String (channelProperties.getPitch ()));
            addLine (channelProperties.getPitchCV () != channelProperties.getPitchCVDefault (), Parameter::Channel::PitchCVId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPitchCV (), 4));
            addLine (channelProperties.getPlayMode () != channelProperties.getPlayModeDefault (), Parameter::Channel::PlayModeId + " : " + juce::String (channelProperties.getPlayMode ()));
            addLine (channelProperties.getPMIndex () != channelProperties.getPMIndexDefault (), Parameter::Channel::PMIndexId + " : " + juce::String (channelProperties.getPMIndex ()));
            addLine (channelProperties.getPMIndexMod () != channelProperties.getPMIndexModDefault (), Parameter::Channel::PMIndexModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getPMIndexMod (), 4));
            addLine (channelProperties.getPMSource () != channelProperties.getPMSourceDefault (), Parameter::Channel::PMSourceId + " : " + juce::String (channelProperties.getPMSource ()));
            addLine (channelProperties.getRelease () != channelProperties.getReleaseDefault (), Parameter::Channel::ReleaseId + " : " + juce::String (channelProperties.getRelease ()));
            addLine (channelProperties.getReleaseMod () != channelProperties.getReleaseModDefault (), Parameter::Channel::ReleaseModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getReleaseMod (), 4));
            addLine (channelProperties.getReverse () != channelProperties.getReverseDefault (), Parameter::Channel::ReverseId + " : " + (channelProperties.getReverse () ? "1" : "0"));
            addLine (channelProperties.getSampleStartMod () != channelProperties.getSampleStartModDefault (), Parameter::Channel::SampleStartModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getSampleStartMod (), 4));
            addLine (channelProperties.getSampleEndMod () != channelProperties.getSampleEndModDefault (), Parameter::Channel::SampleEndModId + " : " + ChannelProperties::getCvInputAndValueString (channelProperties.getSampleEndMod (), 4));
            addLine (channelProperties.getSpliceSmoothing () != channelProperties.getSpliceSmoothingDefault (), Parameter::Channel::SpliceSmoothingId + " : " + (channelProperties.getSpliceSmoothing () ? "1" : "0"));
            addLine (channelProperties.getXfadeGroup () != channelProperties.getXfadeGroupDefault (), Parameter::Channel::XfadeGroupId + " : " + channelProperties.getXfadeGroup ());
            addLine (channelProperties.getZonesCV () != channelProperties.getZonesCVDefault (), Parameter::Channel::ZonesCVId + " : " + channelProperties.getZonesCV ());
            addLine (channelProperties.getZonesRT () != channelProperties.getZonesRTDefault (), Parameter::Channel::ZonesRTId + " : " + juce::String (channelProperties.getZonesRT ()));
            channelProperties.forEachZone ([this, &hasContent, &indentAmount, &addLine] (juce::ValueTree zoneVT)
            {
                ZoneProperties zoneProperties (zoneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
                if (hasContent (zoneVT))
                {
                    const auto zonePropertiesVT { zoneProperties.getValueTree () };
                    addLine (true, Section::ZoneId + " " + juce::String (zoneProperties.getIndex ()) + " :");
                    ++indentAmount;
                    addLine (zoneProperties.getLevelOffset () != zoneProperties.getLevelOffsetDefault (), Parameter::Zone::LevelOffsetId + " : " + juce::String (zoneProperties.getLevelOffset ()));
                    addLine (zoneProperties.getLoopLength () != zoneProperties.getLoopLengthDefault (), Parameter::Zone::LoopLengthId + " : " + juce::String (zoneProperties.getLoopLength ()));
                    addLine (zoneProperties.getLoopStart () != zoneProperties.getLoopStartDefault (), Parameter::Zone::LoopStartId + " : " + juce::String (zoneProperties.getLoopStart ()));
                    addLine (zoneProperties.getMinVoltage () != zoneProperties.getMinVoltageDefault (), Parameter::Zone::MinVoltageId + " : " + juce::String (zoneProperties.getMinVoltage ()));
                    addLine (zoneProperties.getPitchOffset () != zoneProperties.getPitchOffsetDefault (), Parameter::Zone::PitchOffsetId + " : " + juce::String (zoneProperties.getPitchOffset ()));
                    addLine (zoneProperties.getSample () != zoneProperties.getSampleDefault (), Parameter::Zone::SampleId + " : " + zoneProperties.getSample ());
                    addLine (zoneProperties.getSampleStart () != zoneProperties.getSampleStartDefault (), Parameter::Zone::SampleStartId + " : " + juce::String (zoneProperties.getSampleStart ()));
                    addLine (zoneProperties.getSampleEnd () != zoneProperties.getSampleEndDefault (), Parameter::Zone::SampleEndId + " : " + juce::String (zoneProperties.getSampleEnd ()));
                    addLine (zoneProperties.getSide () != zoneProperties.getSideDefault (), Parameter::Zone::SideId + " : " + juce::String (zoneProperties.getSide ()));
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
    presetProperties.initToDefaults ();
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
                jassert(! undoActionsStack.empty ());
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

Assimil8orPreset::Assimil8orPreset ()
{
    auto getParameterIndex = [this] ()
    {
        const auto index { key.fromFirstOccurrenceOf (" ", false, false).getIntValue () };
        jassert (index > 0);
        return index;
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
        {Section::PresetId, [this, undoAction, setActions] () {
            curPresetSection = presetProperties.getValueTree ();
            setActions (&presetActions, [this, undoAction]{
                undoAction (&globalActions, curPresetSection);
            });
        }}
        });

    // Preset Actions
    presetActions.insert ({
        {Section::ChannelId, [this, getParameterIndex, undoAction, setActions] () {
            curChannelSection = presetProperties.addChannel (getParameterIndex ());
            channelProperties.wrap (curChannelSection, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
            setActions (&channelActions, [this, undoAction]{
                undoAction (&presetActions, curChannelSection);
            });
        }},
        {Parameter::Preset::NameId, [this] () {
            presetProperties.setName (value, false);
        }},
        {Parameter::Preset::Data2asCVId, [this] () {
            presetProperties.setData2AsCV (value, false);
        }},
        {Parameter::Preset::XfadeACVId, [this] () {
            presetProperties.setXfadeACV (value, false);
        }},
        {Parameter::Preset::XfadeAWidthId, [this] () {
            presetProperties.setXfadeAWidth (value.getDoubleValue (), false);
        }},
        {Parameter::Preset::XfadeBCVId, [this] () {
            presetProperties.setXfadeBCV (value, false);
        }},
        {Parameter::Preset::XfadeBWidthId, [this] () {
            presetProperties.setXfadeBWidth (value.getDoubleValue (), false);
        }},
        {Parameter::Preset::XfadeCCVId, [this] () {
            presetProperties.setXfadeCCV (value, false);
        }},
        {Parameter::Preset::XfadeCWidthId, [this] () {
            presetProperties.setXfadeCWidth (value.getDoubleValue (), false);
        }},
        {Parameter::Preset::XfadeDCVId, [this] () {
            presetProperties.setXfadeDCV (value, false);
        }},
        {Parameter::Preset::XfadeDWidthId, [this] () {
            presetProperties.setXfadeDWidth (value.getDoubleValue (), false);
        }}
        });

    // Channel Actions
    channelActions.insert ({
        {Section::ZoneId, [this, getParameterIndex, undoAction, setActions] () {
            // TODO - do we need to check for malformed data, ie more than 8 zones
            curZoneSection = channelProperties.addZone (getParameterIndex ());
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
            const auto [cvInput, attackModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setAttackMod (cvInput, attackModAmount, false);
        }},
        {Parameter::Channel::AliasingId, [this] () {
            channelProperties.setAliasing (value.getIntValue (), false);
        }},
        {Parameter::Channel::AliasingModId, [this] () {
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
            const auto [cvInput, bitsModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setBitsMod (cvInput, bitsModAmount, false);
        }},
        {Parameter::Channel::ChannelModeId, [this] () {
            channelProperties.setChannelMode (value.getIntValue (), false);
        }},
        {Parameter::Channel::ExpAMId, [this] () {
            channelProperties.setExpAM (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::ExpFMId, [this] () {
            channelProperties.setExpFM (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::LevelId, [this] () {
            channelProperties.setLevel (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::LinAMId, [this] () {
            channelProperties.setLinAM (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::LinAMisExtEnvId, [this] () {
            channelProperties.setLinAMisExtEnv (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::LinFMId, [this] () {
            channelProperties.setLinFM (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::LoopLengthModId, [this] () {
            const auto [cvInput, loopLengthModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setLoopLengthMod (cvInput, loopLengthModAmount, false);
        }},
        {Parameter::Channel::LoopModeId, [this] () {
            channelProperties.setLoopMode (value.getIntValue (), false);
        }},
        {Parameter::Channel::LoopStartModId, [this] () {
            const auto [cvInput, loopStartModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setLoopStartMod (cvInput, loopStartModAmount, false);
        }},
        {Parameter::Channel::MixLevelId, [this] () {
            channelProperties.setMixLevel (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::MixModId, [this] () {
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
            const auto [cvInput, panModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setPanMod (cvInput, panModAmount, false);
        }},
        {Parameter::Channel::PhaseCVId, [this] () {
            const auto [cvInput, phaseCvAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setPhaseCV (cvInput, phaseCvAmount, false);
        }},
        {Parameter::Channel::PitchId, [this] () {
            channelProperties.setPitch (value.getDoubleValue (), false);
        }},
        {Parameter::Channel::PitchCVId, [this] () {
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
            const auto [cvInput, releaseModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setReleaseMod (cvInput, releaseModAmount, false);
        }},
        {Parameter::Channel::ReverseId, [this] () {
            channelProperties.setReverse (value.getIntValue () == 1, false);
        }},
        {Parameter::Channel::SampleEndModId, [this] () {
            const auto [cvInput, sampleEndModAmount] { ChannelProperties::getCvInputAndValueFromString (value) };
            channelProperties.setSampleEndMod (cvInput, sampleEndModAmount, false);
        }},
        { Parameter::Channel::SampleStartModId, [this] () {
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
