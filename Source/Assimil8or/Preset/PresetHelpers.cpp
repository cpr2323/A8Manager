#include "PresetHelpers.h"

namespace PresetHelpers
{
    void setCvInputAndAmount (CvInputAndAmount cvInputAndAmount, std::function<void (juce::String, double)> setter)
    {
        jassert (setter != nullptr);
        const auto& [cvInput, value] { cvInputAndAmount };
        setter (cvInput, value);
    };

    juce::StringArray getCvInputList (bool includeZero)
    {
        return {};
    }

    int getCvInputIndex (juce::String cvInput)
    {
        if (cvInput == "Off")
            return 0;
        jassert (cvInput.length () == 2);
        jassert (cvInput [0] >= '0' && cvInput [0] <= '8');
        jassert (cvInput [1] >= 'A' && cvInput [1] <= 'C');
        return 1 + ((cvInput [0] - '0') * 3) + cvInput [1] - 'A';
    }

    juce::String getCvInputString (int cvInputIndex)
    {
        if (cvInputIndex == 0)
            return "Off";
        const auto adjustedIndex {cvInputIndex - 1};
        jassert (adjustedIndex >= 0 && adjustedIndex <= 27);
        return juce::String::charToString('0' + adjustedIndex / 3) + juce::String::charToString ('A' + (adjustedIndex % 3));
    }

    // TODO - add options to check Preset Id as well
    bool areEntirePresetsEqual (juce::ValueTree presetOneVT, juce::ValueTree presetTwoVT)
    {
        auto presetsAreEqual { true };
        if (! arePresetsEqual (presetOneVT, presetTwoVT))
        {
            //juce::Logger::outputDebugString ("Preset mismatch");
            presetsAreEqual = false;
        }
        else
        {
            PresetProperties presetOne (presetOneVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
            PresetProperties presetTwo (presetTwoVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
            for (auto channelIndex { 0 }; channelIndex < 8 && presetsAreEqual; ++channelIndex)
            {
                if (! areChannelsEqual (presetOne.getChannelVT (channelIndex), presetTwo.getChannelVT (channelIndex)))
                {
                    //juce::Logger::outputDebugString ("Channel "+juce::String (channelIndex) + " mismatch");
                    presetsAreEqual = false;
                }

                ChannelProperties presetOneChannelProperties (presetOne.getChannelVT (channelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                ChannelProperties presetTwoChannelProperties (presetTwo.getChannelVT (channelIndex), ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
                for (auto zoneIndex { 0 }; zoneIndex < 8 && presetsAreEqual; ++zoneIndex)
                {
                    if (! areZonesEqual (presetOneChannelProperties.getZoneVT (zoneIndex), presetTwoChannelProperties.getZoneVT (zoneIndex)))
                    {
                        //juce::Logger::outputDebugString ("Zone " + juce::String (zoneIndex) + " mismatch");
                        //displayZoneDifferences (presetOneZoneProperties, presetTwoZoneProperties);
                        presetsAreEqual = false;
                    }
                }
            }
        }

        return presetsAreEqual;
    }

    bool arePresetsEqual (juce::ValueTree presetOneVT, juce::ValueTree presetTwoVT)
    {
        PresetProperties presetPropertiesOne (presetOneVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        PresetProperties presetPropertiesTwo (presetTwoVT, PresetProperties::WrapperType::client, PresetProperties::EnableCallbacks::no);
        return  presetPropertiesOne.getData2AsCV () == presetPropertiesTwo.getData2AsCV () &&
                presetPropertiesOne.getMidiSetup () == presetPropertiesTwo.getMidiSetup () &&
                presetPropertiesOne.getName () == presetPropertiesTwo.getName () &&
                presetPropertiesOne.getXfadeACV () == presetPropertiesTwo.getXfadeACV () &&
                presetPropertiesOne.getXfadeAWidth () == presetPropertiesTwo.getXfadeAWidth () &&
                presetPropertiesOne.getXfadeBCV () == presetPropertiesTwo.getXfadeBCV () &&
                presetPropertiesOne.getXfadeBWidth () == presetPropertiesTwo.getXfadeBWidth () &&
                presetPropertiesOne.getXfadeCCV () == presetPropertiesTwo.getXfadeCCV () &&
                presetPropertiesOne.getXfadeCWidth () == presetPropertiesTwo.getXfadeCWidth () &&
                presetPropertiesOne.getXfadeDCV () == presetPropertiesTwo.getXfadeDCV () &&
                presetPropertiesOne.getXfadeDWidth () == presetPropertiesTwo.getXfadeDWidth ();
    };

    bool areChannelsEqual (juce::ValueTree channelOneVT, juce::ValueTree channelTwoVT)
    {
        ChannelProperties channelPropertiesOne (channelOneVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        ChannelProperties channelPropertiesTwo (channelTwoVT, ChannelProperties::WrapperType::client, ChannelProperties::EnableCallbacks::no);
        return  channelPropertiesOne.getAliasing () == channelPropertiesTwo.getAliasing () &&
                channelPropertiesOne.getAliasingMod () == channelPropertiesTwo.getAliasingMod () &&
                channelPropertiesOne.getAttack () == channelPropertiesTwo.getAttack () &&
                channelPropertiesOne.getAttackFromCurrent () == channelPropertiesTwo.getAttackFromCurrent () &&
                channelPropertiesOne.getAttackMod () == channelPropertiesTwo.getAttackMod () &&
                channelPropertiesOne.getAutoTrigger () == channelPropertiesTwo.getAutoTrigger () &&
                channelPropertiesOne.getBits () == channelPropertiesTwo.getBits () &&
                channelPropertiesOne.getBitsMod () == channelPropertiesTwo.getBitsMod () &&
                channelPropertiesOne.getChannelMode () == channelPropertiesTwo.getChannelMode () &&
                channelPropertiesOne.getExpAM () == channelPropertiesTwo.getExpAM () &&
                channelPropertiesOne.getExpFM () == channelPropertiesTwo.getExpFM () &&
                channelPropertiesOne.getLevel () == channelPropertiesTwo.getLevel () &&
                channelPropertiesOne.getLinAM () == channelPropertiesTwo.getLinAM () &&
                channelPropertiesOne.getLinAMisExtEnv () == channelPropertiesTwo.getLinAMisExtEnv () &&
                channelPropertiesOne.getLinFM () == channelPropertiesTwo.getLinFM () &&
                channelPropertiesOne.getLoopLengthIsEnd () == channelPropertiesTwo.getLoopLengthIsEnd () &&
                channelPropertiesOne.getLoopLengthMod () == channelPropertiesTwo.getLoopLengthMod () &&
                channelPropertiesOne.getLoopMode () == channelPropertiesTwo.getLoopMode () &&
                channelPropertiesOne.getLoopStartMod () == channelPropertiesTwo.getLoopStartMod () &&
                channelPropertiesOne.getMixLevel () == channelPropertiesTwo.getMixLevel () &&
                channelPropertiesOne.getMixMod () == channelPropertiesTwo.getMixMod () &&
                channelPropertiesOne.getMixModIsFader () == channelPropertiesTwo.getMixModIsFader () &&
                channelPropertiesOne.getPan () == channelPropertiesTwo.getPan () &&
                channelPropertiesOne.getPanMod () == channelPropertiesTwo.getPanMod () &&
                channelPropertiesOne.getPhaseCV () == channelPropertiesTwo.getPhaseCV () &&
                channelPropertiesOne.getPitch () == channelPropertiesTwo.getPitch () &&
                channelPropertiesOne.getPitchCV () == channelPropertiesTwo.getPitchCV () &&
                channelPropertiesOne.getPlayMode () == channelPropertiesTwo.getPlayMode () &&
                channelPropertiesOne.getPMIndex () == channelPropertiesTwo.getPMIndex () &&
                channelPropertiesOne.getPMIndexMod () == channelPropertiesTwo.getPMIndexMod () &&
                channelPropertiesOne.getPMSource () == channelPropertiesTwo.getPMSource () &&
                channelPropertiesOne.getRelease () == channelPropertiesTwo.getRelease () &&
                channelPropertiesOne.getReleaseMod () == channelPropertiesTwo.getReleaseMod () &&
                channelPropertiesOne.getReverse () == channelPropertiesTwo.getReverse () &&
                channelPropertiesOne.getSampleStartMod () == channelPropertiesTwo.getSampleStartMod () &&
                channelPropertiesOne.getSampleEndMod () == channelPropertiesTwo.getSampleEndMod () &&
                channelPropertiesOne.getSpliceSmoothing () == channelPropertiesTwo.getSpliceSmoothing () &&
                channelPropertiesOne.getXfadeGroup () == channelPropertiesTwo.getXfadeGroup () &&
                channelPropertiesOne.getZonesCV () == channelPropertiesTwo.getZonesCV () &&
                channelPropertiesOne.getZonesRT () == channelPropertiesTwo.getZonesRT ();
    };
    bool areZonesEqual (juce::ValueTree zoneOneVT, juce::ValueTree zoneTwoVT)
    {
        ZoneProperties zonePropertiesOne (zoneOneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        ZoneProperties zonePropertiesTwo (zoneTwoVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        return  zonePropertiesOne.getLevelOffset () == zonePropertiesTwo.getLevelOffset () &&
                zonePropertiesOne.getLoopLength ().value_or (0.0) == zonePropertiesTwo.getLoopLength ().value_or (0.0) &&
                zonePropertiesOne.getLoopStart ().value_or (0) == zonePropertiesTwo.getLoopStart ().value_or (0) &&
                zonePropertiesOne.getMinVoltage () == zonePropertiesTwo.getMinVoltage () &&
                zonePropertiesOne.getPitchOffset () == zonePropertiesTwo.getPitchOffset () &&
                zonePropertiesOne.getSample () == zonePropertiesTwo.getSample () &&
                zonePropertiesOne.getSampleStart ().value_or (0) == zonePropertiesTwo.getSampleStart ().value_or (0) &&
                zonePropertiesOne.getSampleEnd ().value_or (0) == zonePropertiesTwo.getSampleEnd ().value_or (0) &&
                zonePropertiesOne.getSide () == zonePropertiesTwo.getSide ();
    };

    void displayZoneDifferences (juce::ValueTree zonePropertiesOneVT, juce::ValueTree zonePropertiesTwoVT)
    {
        auto displayIfDiff = [] (auto value1, auto value2, juce::String parameterName)
        {
            if (value1 != value2)
                juce::Logger::outputDebugString (parameterName + " mismatch. " + juce::String (value1) + " != " + juce::String (value2));
        };

        ZoneProperties zonePropertiesOne (zonePropertiesOneVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);
        ZoneProperties zonePropertiesTwo (zonePropertiesTwoVT, ZoneProperties::WrapperType::client, ZoneProperties::EnableCallbacks::no);

        displayIfDiff (zonePropertiesOne.getLevelOffset (), zonePropertiesTwo.getLevelOffset (), "LevelOffset");
        displayIfDiff (zonePropertiesOne.getLoopLength ().value_or (99999999), zonePropertiesTwo.getLoopLength ().value_or (99999999), "LoopLength");
        displayIfDiff (zonePropertiesOne.getLoopStart ().value_or (99999999), zonePropertiesTwo.getLoopStart ().value_or (99999999), "LoopStart");
        displayIfDiff (zonePropertiesOne.getMinVoltage (), zonePropertiesTwo.getMinVoltage (), "MinVoltage");
        displayIfDiff (zonePropertiesOne.getPitchOffset (), zonePropertiesTwo.getPitchOffset (), "PitchOffset");
        displayIfDiff (zonePropertiesOne.getSample (), zonePropertiesTwo.getSample (), "Sample");
        displayIfDiff (zonePropertiesOne.getSampleStart ().value_or (99999999), zonePropertiesTwo.getSampleStart ().value_or (99999999), "SampleStart");
        displayIfDiff (zonePropertiesOne.getSampleEnd ().value_or (99999999), zonePropertiesTwo.getSampleEnd ().value_or (99999999), "SampleEnd");
        displayIfDiff (zonePropertiesOne.getSide (), zonePropertiesTwo.getSide (), "Side");
    };

    // TODO - this should be done in the Directory scan, and stored in a property
    bool isSupportedAudioFile (juce::File file)
    {
        juce::AudioFormatManager audioFormatManager;
        audioFormatManager.registerBasicFormats ();

        if (file.isDirectory () || file.getFileExtension () != ".wav")
            return false;
        std::unique_ptr<juce::AudioFormatReader> reader (audioFormatManager.createReaderFor (file));
        if (reader == nullptr)
            return false;
        // check for any format settings that are unsupported
        if ((reader->usesFloatingPointData == true) || (reader->bitsPerSample < 8 || reader->bitsPerSample > 32) || (reader->numChannels == 0 || reader->numChannels > 2) || (reader->sampleRate > 192000))
            return false;

        return true;
    }

};