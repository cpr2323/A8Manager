#pragma once

#include <JuceHeader.h>
#include "SampleStatus.h"
#include "../../../../Utility/ValueTreeWrapper.h"

using AudioBufferType = juce::AudioBuffer<float>;

class SampleProperties : public ValueTreeWrapper<SampleProperties>
{
public:
    SampleProperties () noexcept : ValueTreeWrapper<SampleProperties> (SamplePropertiesTypeId)
    {
    }
    SampleProperties (juce::ValueTree vt, WrapperType wrapperType, EnableCallbacks shouldEnableCallbacks) noexcept
        : ValueTreeWrapper<SampleProperties> (SamplePropertiesTypeId, vt, wrapperType, shouldEnableCallbacks)
    {
    }

    void setName (juce::String name, bool includeSelfCallback);
    void setBitsPerSample (int bitsPerSample, bool includeSelfCallback);
    void setSampleRate (double sampleRate, bool includeSelfCallback);
    void setNumChannels (int numChannels, bool includeSelfCallback);
    void setLengthInSamples (juce::int64, bool includeSelfCallback);
    void setAudioBufferPtr (AudioBufferType* audioBufferPtr, bool includeSelfCallback);
    void setStatus (SampleStatus status, bool includeSelfCallback);

    juce::String getName ();
    int getBitsPerSample ();
    double getSampleRate ();
    int getNumChannels ();
    juce::int64 getLengthInSamples ();
    AudioBufferType* getAudioBufferPtr ();
    SampleStatus getStatus ();

    std::function<void (juce::String name)> onNameChange;
    std::function<void (int bitsPerSample)> onBitsPerSampleChange;
    std::function<void (double sampleRate)> onSampleRateChange;
    std::function<void (int numChannels)> onNumChannelsChange;
    std::function<void (juce::int64 lengthInSamples)> onLengthInSamplesChange;
    std::function<void (AudioBufferType* audioBufferPtr)> onAudioBufferPtrChange;
    std::function<void (SampleStatus status)> onStatusChange;

    static inline const juce::Identifier SamplePropertiesTypeId { "Sample" };
    static inline const juce::Identifier NamePropertyId            { "name" };
    static inline const juce::Identifier BitsPerSamplePropertyId   { "bitsPerSample"};
    static inline const juce::Identifier SampleRatePropertyId      { "sampleRate" };
    static inline const juce::Identifier NumChannelsPropertyId     { "numChannels" };
    static inline const juce::Identifier LengthInSamplesPropertyId { "lengthInSamples" };
    static inline const juce::Identifier AudioBufferPtrPropertyId  { "audioBufferPtr" };
    static inline const juce::Identifier StatusPropertyId          { "status" };

    void initValueTree ();
    void processValueTree () {}

private:
    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& property) override;
};
