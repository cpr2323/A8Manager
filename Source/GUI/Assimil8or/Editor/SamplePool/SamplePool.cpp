#include "SamplePool.h"

SamplePool::SamplePool ()
{
    audioFormatManager.registerBasicFormats ();
}

void SamplePool::setParentFolder (juce::File theParentFolder)
{
    jassert (sampleList.empty ());
    parentFolder = theParentFolder;
}

SampleData SamplePool::useSample (juce::String fileName)
{
    jassert (parentFolder.exists ());
    const auto sampleDataIter { sampleList.find (fileName) };
    if (sampleDataIter == sampleList.end ())
    {
        return loadSample (fileName);
    }
    sampleDataIter->second.useCount++;
    return { &sampleDataIter->second.exists, &sampleDataIter->second.bitsPerSample, &sampleDataIter->second.numChannels,
             &sampleDataIter->second.lengthInSamples, &sampleDataIter->second.audioBuffer };
}

SampleData SamplePool::loadSample (juce::String fileName)
{
    juce::File fullPath { parentFolder.getChildFile (fileName) };
    if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (fullPath) }; sampleFileReader != nullptr)
    {
        SampleDataInternal newSampleDataInternal;
        // cache sample attributes
        newSampleDataInternal.bitsPerSample = sampleFileReader->bitsPerSample;
        newSampleDataInternal.numChannels = sampleFileReader->numChannels;
        newSampleDataInternal.lengthInSamples = sampleFileReader->lengthInSamples;

        // read in audio data
        newSampleDataInternal.audioBuffer.setSize (newSampleDataInternal.numChannels, static_cast<int> (newSampleDataInternal.lengthInSamples), false, true, false);
        sampleFileReader->read (&newSampleDataInternal.audioBuffer, 0, static_cast<int> (newSampleDataInternal.lengthInSamples), 0, true, false);

        sampleList [fileName] = std::move (newSampleDataInternal);

        auto& sdi { sampleList [fileName] };
        return { &sdi.exists, &sdi.bitsPerSample, &sdi.numChannels,
                 &sdi.lengthInSamples, &sdi.audioBuffer };
    }
    else
    {
        // TODO - handle error
        jassertfalse;
    }
    return { &errorSampleData.exists, &errorSampleData.bitsPerSample, &errorSampleData.numChannels,
             &errorSampleData.lengthInSamples, &errorSampleData.audioBuffer };
}

void SamplePool::clear ()
{
}

void SamplePool::unUseSample (juce::String fileName)
{
    const auto sampleDataIter { sampleList.find (fileName) };
    jassert (sampleDataIter != sampleList.end ());
    jassert (sampleList [fileName].useCount != 0);
    --sampleList [fileName].useCount;
    if (sampleList [fileName].useCount == 0)
        sampleList.erase (fileName);
}
