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

SamplePool::SampleData SamplePool::useSample (juce::String fileName)
{
    jassert (parentFolder.exists ());
    const auto sampleDataIter { sampleList.find (fileName) };
    if (sampleDataIter == sampleList.end ())
    {
        return loadSample (fileName);
    }
    sampleDataIter->second.useCount++;
    return { sampleDataIter->second.bitsPerSample, sampleDataIter->second.numChannels,
             sampleDataIter->second.lengthInSamples, sampleDataIter->second.audioBuffer };
}

SamplePool::SampleData SamplePool::loadSample (juce::String fileName)
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

        const auto sdi { sampleList [fileName] };
        return { sdi.bitsPerSample, sdi.numChannels,
                 sdi.lengthInSamples, sdi.audioBuffer };
    }
    else
    {
        // TODO - handle error
        jassertfalse;
    }
    return {};
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
