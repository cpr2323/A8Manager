#pragma once

#include <JuceHeader.h>

namespace Section
{
    static juce::String PresetId  { "Preset" };
    static juce::String ChannelId { "Channel" };
    static juce::String ZoneId    { "Zone" };
};

namespace Parameter
{
    namespace Preset
    {
        static juce::String NameId        { "Name" };
        static juce::String Data2asCVId   { "Data2asCV" };
        static juce::String XfadeACVId    { "XfadeACV" };
        static juce::String XfadeAWidthId { "XfadeAWidth" };
        static juce::String XfadeBCVId    { "XfadeBCV" };
        static juce::String XfadeBWidthId { "XfadeBWidth" };
        static juce::String XfadeCCVId    { "XfadeCCV" };
        static juce::String XfadeCWidthId { "XfadeCWidth" };
        static juce::String XfadeDCVId    { "XfadeDCV" };
        static juce::String XfadeDWidthId { "XfadeDWidth" };
    };
    namespace Channel
    {
        static juce::String AliasingId          { "Aliasing" };
        static juce::String AliasingModId       { "AliasingMod" };
        static juce::String AttackId            { "Attack" };
        static juce::String AttackFromCurrentId { "AttackFromCurrent" };
        static juce::String AttackModId         { "AttackMod" };
        static juce::String AutoTriggerId       { "AutoTrigger" };
        static juce::String BitsId              { "Bits" };
        static juce::String BitsModId           { "BitsMod" };
        static juce::String ChannelModeId       { "ChannelMode" };
        static juce::String ExpAMId             { "ExpAM" };
        static juce::String ExpFMId             { "ExpFM" };
        static juce::String LevelId             { "Level" };
        static juce::String LinAMId             { "LinAM" };
        static juce::String LinAMisExtEnvId     { "LinAMisExtEnv" };
        static juce::String LinFMId             { "LinFM" };
        static juce::String LoopLengthIsEndId   { "LoopLengthIsEnd" };
        static juce::String LoopLengthModId     { "LoopLengthMod" };
        static juce::String LoopModeId          { "LoopMode" };
        static juce::String LoopStartModId      { "LoopStartMod" };
        static juce::String MixLevelId          { "MixLevel" };
        static juce::String MixModId            { "MixMod" };
        static juce::String MixModIsFaderId     { "MixModIsFader" };
        static juce::String PanId               { "Pan" };
        static juce::String PanModId            { "PanMod" };
        static juce::String PhaseCVId           { "PhaseCV" };
        static juce::String PitchId             { "Pitch" };
        static juce::String PitchCVId           { "PitchCV" };
        static juce::String PlayModeId          { "PlayMode" };
        static juce::String PMIndexId           { "PMIndex" };
        static juce::String PMIndexModId        { "PMIndexMod" };
        static juce::String PMSourceId          { "PMSource" };
        static juce::String ReleaseId           { "Release" };
        static juce::String ReleaseModId        { "ReleaseMod" };
        static juce::String ReverseId           { "Reverse" };
        static juce::String SampleStartModId    { "SampleStartMod" };
        static juce::String SampleEndModId      { "SampleEndMod" };
        static juce::String SpliceSmoothingId   { "SpliceSmoothing" };
        static juce::String XfadeGroupId        { "XfadeGroup" };
        static juce::String ZonesCVId           { "ZonesCV" };
        static juce::String ZonesRTId           { "ZonesRT" };
    };
    namespace Zone
    {
        static juce::String LevelOffsetId { "LevelOffset" };
        static juce::String LoopLengthId  { "LoopLength" };
        static juce::String LoopStartId   { "LoopStart" };
        static juce::String MinVoltageId  { "MinVoltage" };
        static juce::String PitchOffsetId { "PitchOffset" };
        static juce::String SampleId      { "Sample" };
        static juce::String SampleStartId { "SampleStart" };
        static juce::String SampleEndId   { "SampleEnd" };
        static juce::String SideId        { "Side" };
    };
};
