#include "SampleManagerProperties.h"
#include "SampleProperties.h"
#include "../../../../Utility/ValueTreeHelpers.h"

void SampleManagerProperties::initValueTree ()
{
    for (auto channelIndex { 0 }; channelIndex < 8; ++channelIndex)
        for (auto zoneIndex { 0 }; zoneIndex < 8; ++zoneIndex)
        {
            SampleProperties sampleProperties ({}, SampleProperties::WrapperType::owner, SampleProperties::EnableCallbacks::no);
            data.addChild (sampleProperties.getValueTree (), -1, nullptr);
        }
}

juce::ValueTree SampleManagerProperties::getSamplePropertiesVT (int channelIndex, int zoneIndex)
{
    jassert (channelIndex >= 0 && channelIndex < 8);
    jassert (zoneIndex >= 0 && zoneIndex < 8);
    auto curChannelIndex { 0 };
    auto curZoneIndex { 0 };
    SampleProperties requestedSampleProperties;
    ValueTreeHelpers::forEachChild (data, [this, channelIndex, zoneIndex, &curChannelIndex, &curZoneIndex, &requestedSampleProperties] (juce::ValueTree samplePropertiesVT)
    {
        if (curChannelIndex == channelIndex && curZoneIndex == zoneIndex)
        {
            requestedSampleProperties.wrap (samplePropertiesVT, SampleProperties::WrapperType::client, SampleProperties::EnableCallbacks::no);
            return false;
        }
        ++curZoneIndex;
        if (curZoneIndex > 7)
        {
            curZoneIndex = 0;
            ++curChannelIndex;
            jassert (curChannelIndex < 8);
        }
        return true;
    });
    jassert (requestedSampleProperties.isValid ());
    return requestedSampleProperties.getValueTree ();
}
