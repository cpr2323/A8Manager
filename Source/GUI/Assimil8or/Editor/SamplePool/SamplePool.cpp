#include "SamplePool.h"
#include "../../../../Utility/DebugLog.h"

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
    DebugLog("SamplePool", "SamplePool::useSample: " + fileName);
    jassert (fileName.isNotEmpty ());
    jassert (parentFolder.exists ());
    const auto sampleDataIter { sampleList.find (fileName) };
    if (sampleDataIter == sampleList.end ())
        return loadSample (fileName);

    sampleDataIter->second.useCount++;
    return { &sampleDataIter->second.status, &sampleDataIter->second.bitsPerSample, &sampleDataIter->second.numChannels,
             &sampleDataIter->second.lengthInSamples, &sampleDataIter->second.audioBuffer };
}

SampleData SamplePool::loadSample (juce::String fileName)
{
    SampleDataInternal newSampleDataInternal;

    juce::File fullPath { parentFolder.getChildFile (fileName) };
    newSampleDataInternal.useCount = 1;
    if (fullPath.exists ())
    {
        if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (fullPath) }; sampleFileReader != nullptr)
        {
            // cache sample attributes
            newSampleDataInternal.status = SampleData::SampleDataStatus::exists;
            newSampleDataInternal.bitsPerSample = sampleFileReader->bitsPerSample;
            newSampleDataInternal.numChannels = sampleFileReader->numChannels;
            newSampleDataInternal.lengthInSamples = sampleFileReader->lengthInSamples;

            // read in audio data
            newSampleDataInternal.audioBuffer.setSize (newSampleDataInternal.numChannels, static_cast<int> (newSampleDataInternal.lengthInSamples), false, true, false);
            sampleFileReader->read (&newSampleDataInternal.audioBuffer, 0, static_cast<int> (newSampleDataInternal.lengthInSamples), 0, true, false);
        }
        else
        {
            newSampleDataInternal.status = SampleData::SampleDataStatus::wrongFormat;
        }
    }
    else
    {
        newSampleDataInternal.status = SampleData::SampleDataStatus::doesNotExist;
    }

    sampleList [fileName] = std::move (newSampleDataInternal);
    auto& sdi { sampleList [fileName] };
    return { &sdi.status, &sdi.bitsPerSample, &sdi.numChannels,
             &sdi.lengthInSamples, &sdi.audioBuffer };
}

void SamplePool::updateSample (juce::String fileName)
{
    // TODO - update the information stored for this sample
    jassertfalse;
}

void SamplePool::clear ()
{
    sampleList.clear ();
}

void SamplePool::unUseSample (juce::String fileName)
{
    DebugLog ("SamplePool", "SamplePool::unUseSample: " + fileName);
    jassert (fileName.isNotEmpty ());
    const auto sampleDataIter { sampleList.find (fileName) };
    jassert (sampleDataIter != sampleList.end ());
    jassert (sampleList [fileName].useCount != 0);
    --sampleList [fileName].useCount;
    if (sampleList [fileName].useCount == 0)
        sampleList.erase (fileName);
}
