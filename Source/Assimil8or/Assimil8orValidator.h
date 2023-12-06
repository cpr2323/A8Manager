#pragma once

#include <JuceHeader.h>
#include "Validator/ValidatorProperties.h"
#include "../AppProperties.h"
#include "../Utility/DirectoryDataProperties.h"
#include "../Utility/LambdaThread.h"

// validate
//  SD Card (max folders unknown)
//      Folder (31 character len) (max 199 presets) (no nested folders)
//          Preset (prst000.yml)
//          Preset
//          Audio (47 character len) (arbitrary name)
//          Audio
class Assimil8orValidator : public juce::Thread
{
public:
    Assimil8orValidator ();
    ~Assimil8orValidator ();

    void init (juce::ValueTree rootPropertiesVT);

private:
    enum class ValdatationState
    {
        idle,
        validating,
        restarting
    };
    AppProperties appProperties;
    ValidatorProperties validatorProperties;
    ValidatorResultListProperties validatorResultListProperties;
    DirectoryDataProperties directoryDataProperties;

    // TODO - my current thought is that the DirectoryValueTree class should perform all of the checks (files types, supported audio, etc), and store into in the valuetree
    juce::AudioFormatManager audioFormatManager;

    juce::int64 lastScanInProgressUpdate {};
    ValdatationState valdatationState { ValdatationState::idle };
    juce::CriticalSection threadManagmentLock;
    std::atomic<bool> cancelCurrentValidation { false };
    LambdaThread validateThread { "ValidateThread", 100 };

    void addResult (juce::String statusType, juce::String statusText);
    void addResult (juce::ValueTree validatorResultsVT);
    void doIfProgressTimeElapsed (std::function<void ()> functionToDo);
    void processFolder (juce::ValueTree folder);
    bool shouldCancelOperation ();
    void startValidation ();
    std::tuple<uint64_t, std::optional<std::map<juce::String, uint64_t>>> validateFile (juce::File file, juce::ValueTree validatorResultsVT);
    void validateFolder (juce::File folder, juce::ValueTree validatorResultsVT);
    void validateRootFolder ();

    void run () override;
};
