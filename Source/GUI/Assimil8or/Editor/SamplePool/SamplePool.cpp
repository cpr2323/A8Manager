#include "SamplePool.h"
#include "../../../../Utility/DebugLog.h"
#include "../../../../Utility/DumpStack.h"

SamplePool::SamplePool ()
{
    audioFormatManager.registerBasicFormats ();
}

void SamplePool::setFolder (juce::File theParentFolder)
{
    //jassert (sampleList.empty ());
    parentFolder = theParentFolder;
}

SampleData SamplePool::open (juce::String fileName)
{
    DebugLog("SamplePool", "open: " + fileName);
    jassert (fileName.isNotEmpty ());
    jassert (parentFolder.exists ());
    const auto sampleDataIter { sampleList.find (fileName) };
    if (sampleDataIter == sampleList.end ())
        return loadSample (fileName);

    sampleDataIter->second.useCount++;
    DebugLog ("SamplePool", "open: useCount:" + juce::String (sampleList [fileName].useCount));
    return { &sampleDataIter->second.status, &sampleDataIter->second.bitsPerSample, &sampleDataIter->second.numChannels,
             &sampleDataIter->second.lengthInSamples, &sampleDataIter->second.audioBuffer };
}

SampleData SamplePool::loadSample (juce::String fileName)
{
    DebugLog ("SamplePool", "loadSample: " + fileName);
    SampleDataInternal newSampleDataInternal;
    newSampleDataInternal.useCount = 1;
    updateSample (fileName, newSampleDataInternal);
    DebugLog ("SamplePool", "loadSample: useCount: 1");

    sampleList [fileName] = std::move (newSampleDataInternal);
    auto& sdi { sampleList [fileName] };
    return { &sdi.status, &sdi.bitsPerSample, &sdi.numChannels,
             &sdi.lengthInSamples, &sdi.audioBuffer };
}

void SamplePool::updateSample (juce::String fileName, SampleDataInternal& sampleData)
{
    juce::File fullPath { parentFolder.getChildFile (fileName) };
    if (fullPath.exists ())
    {
        DebugLog ("SamplePool", "updateSample: file exists");
        if (std::unique_ptr<juce::AudioFormatReader> sampleFileReader { audioFormatManager.createReaderFor (fullPath) }; sampleFileReader != nullptr)
        {
            // cache sample attributes
            sampleData.status = SampleData::SampleDataStatus::exists;
            sampleData.bitsPerSample = sampleFileReader->bitsPerSample;
            sampleData.numChannels = sampleFileReader->numChannels;
            sampleData.lengthInSamples = sampleFileReader->lengthInSamples;

            // read in audio data
            sampleData.audioBuffer.setSize (sampleData.numChannels, static_cast<int> (sampleData.lengthInSamples), false, true, false);
            sampleFileReader->read (&sampleData.audioBuffer, 0, static_cast<int> (sampleData.lengthInSamples), 0, true, false);
        }
        else
        {
            DebugLog ("SamplePool", "updateSample: wrong format");
            sampleData.status = SampleData::SampleDataStatus::wrongFormat;
        }
    }
    else
    {
        DebugLog ("SamplePool", "updateSample: does not exist");
        sampleData.status = SampleData::SampleDataStatus::doesNotExist;
    }
}

void SamplePool::clear ()
{
    sampleList.clear ();
}

void SamplePool::close (juce::String fileName)
{
    DebugLog ("SamplePool", "close: " + fileName);
    //dumpStacktrace (-1, [this] (juce::String logLine) { DebugLog ("SamplePool", logLine); });
    jassert (fileName.isNotEmpty ());
    const auto sampleDataIter { sampleList.find (fileName) };
    jassert (sampleDataIter != sampleList.end ());
    jassert (sampleList [fileName].useCount != 0);
    --sampleList [fileName].useCount;
    DebugLog ("SamplePool", "close: useCount:" + juce::String(sampleList [fileName].useCount));
    if (sampleList [fileName].useCount == 0)
    {
        sampleList.erase (fileName);
        DebugLog ("SamplePool", "close: deleting");
    }
}
