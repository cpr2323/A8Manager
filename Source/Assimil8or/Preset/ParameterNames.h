#pragma once

#include <JuceHeader.h>

namespace Section
{
    juce::String PresetId  { "Preset" };
    juce::String ChannelId { "Channel" };
    juce::String ZoneId    { "Zone" };
};

namespace Parameter
{
    namespace Preset
    {
        juce::String NameId        { "Name" };
        juce::String Data2asCVId   { "Data2asCV" };
        juce::String XfadeACVId    { "XfadeACV" };
        juce::String XfadeAWidthId { "XfadeAWidth" };
        juce::String XfadeBCVId    { "XfadeBCV" };
        juce::String XfadeBWidthId { "XfadeBWidth" };
        juce::String XfadeCCVId    { "XfadeCCV" };
        juce::String XfadeCWidthId { "XfadeCWidth" };
        juce::String XfadeDCVId    { "XfadeDCV" };
        juce::String XfadeDWidthId { "XfadeDWidth" };
    };
    namespace Channel
    {
        juce::String AliasingId          { "Aliasing" };
        juce::String AliasingModId       { "AliasingMod" };
        juce::String AttackId            { "Attack" };
        juce::String AttackFromCurrentId { "AttackFromCurrent" };
        juce::String AttackModId         { "AttackMod" };
        juce::String AutoTriggerId       { "AutoTrigger" };
        juce::String BitsId              { "Bits" };
        juce::String BitsModId           { "BitsMod" };
        juce::String ChannelModeId       { "ChannelMode" };
        juce::String ExpAMId             { "ExpAM" };
        juce::String ExpFMId             { "ExpFM" };
        juce::String LevelId             { "Level" };
        juce::String LinAMId             { "LinAM" };
        juce::String LinAMisExtEnvId     { "LinAMisExtEnv" };
        juce::String LinFMId             { "LinFM" };
        juce::String LoopLengthModId     { "LoopLengthMod" };
        juce::String LoopModeId          { "LoopMode" };
        juce::String LoopStartModId      { "LoopStartMod" };
        juce::String MixLevelId          { "MixLevel" };
        juce::String MixModId            { "MixMod" };
        juce::String MixModIsFaderId     { "MixModIsFader" };
        juce::String PanId               { "Pan" };
        juce::String PanModId            { "PanMod" };
        juce::String PhaseCVId           { "PhaseCV" };
        juce::String PitchId             { "Pitch" };
        juce::String PitchCVId           { "PitchCV" };
        juce::String PlayModeId          { "PlayMode" };
        juce::String PMIndexId           { "PMIndex" };
        juce::String PMIndexModId        { "PMIndexMod" };
        juce::String PMSourceId          { "PMSource" };
        juce::String ReleaseId           { "Release" };
        juce::String ReleaseModId        { "ReleaseMod" };
        juce::String ReverseId           { "Reverse" };
        juce::String SampleStartModId    { "SampleStartMod" };
        juce::String SampleEndModId      { "SampleEndMod" };
        juce::String SpliceSmoothingId   { "SpliceSmoothing" };
        juce::String XfadeGroupId        { "XfadeGroup" };
        juce::String ZonesCVId           { "ZonesCV" };
        juce::String ZonesRTId           { "ZonesRT" };
    };
    namespace Zone
    {
        juce::String LevelOffsetId { "LevelOffset" };
        juce::String LoopLengthId  { "LoopLength" };
        juce::String LoopStartId   { "LoopStart" };
        juce::String MinVoltageId  { "MinVoltage" };
        juce::String PitchOffsetId { "PitchOffset" };
        juce::String SampleId      { "Sample" };
        juce::String SampleStartId { "SampleStart" };
        juce::String SampleEndId   { "SampleEnd" };
        juce::String SideId        { "Side" };
    };
};
