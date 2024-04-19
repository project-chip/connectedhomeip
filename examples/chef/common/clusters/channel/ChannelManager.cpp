/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/util/config.h>
#ifdef MATTER_DM_PLUGIN_CHANNEL_SERVER
#include "ChannelManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Channel;
using namespace chip::Uint8;

ChannelManager::ChannelManager()
{
    ChannelInfoType abc;
    abc.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("KAAL"));
    abc.callSign          = MakeOptional(chip::CharSpan::fromCharString("KAAL-TV"));
    abc.name              = MakeOptional(chip::CharSpan::fromCharString("ABC"));
    abc.majorNumber       = static_cast<uint8_t>(6);
    abc.minorNumber       = static_cast<uint16_t>(0);
    mChannels.push_back(abc);

    ChannelInfoType pbs;
    pbs.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("KCTS"));
    pbs.callSign          = MakeOptional(chip::CharSpan::fromCharString("KCTS-TV"));
    pbs.name              = MakeOptional(chip::CharSpan::fromCharString("PBS"));
    pbs.majorNumber       = static_cast<uint8_t>(9);
    pbs.minorNumber       = static_cast<uint16_t>(1);
    mChannels.push_back(pbs);

    ChannelInfoType pbsKids;
    pbsKids.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("KCTS"));
    pbsKids.callSign          = MakeOptional(chip::CharSpan::fromCharString("KCTS-TV"));
    pbsKids.name              = MakeOptional(chip::CharSpan::fromCharString("PBS Kids"));
    pbsKids.majorNumber       = static_cast<uint8_t>(9);
    pbsKids.minorNumber       = static_cast<uint16_t>(2);
    mChannels.push_back(pbsKids);

    ChannelInfoType worldChannel;
    worldChannel.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("KCTS"));
    worldChannel.callSign          = MakeOptional(chip::CharSpan::fromCharString("KCTS-TV"));
    worldChannel.name              = MakeOptional(chip::CharSpan::fromCharString("World Channel"));
    worldChannel.majorNumber       = static_cast<uint8_t>(9);
    worldChannel.minorNumber       = static_cast<uint16_t>(3);
    mChannels.push_back(worldChannel);

    mCurrentChannelIndex = 0;
    mCurrentChannel      = mChannels[mCurrentChannelIndex];

    ProgramType program1;
    program1.identifier = chip::CharSpan::fromCharString("progid-abc1");
    program1.channel    = abc;
    program1.title      = chip::CharSpan::fromCharString("ABC Title1");
    program1.subtitle   = MakeOptional(chip::CharSpan::fromCharString("My Program Subtitle1"));
    program1.startTime  = 0;
    program1.endTime    = 30 * 60;

    mPrograms.push_back(program1);

    ProgramType program_pbs1;
    program_pbs1.identifier = chip::CharSpan::fromCharString("progid-pbs1");
    program_pbs1.channel    = pbs;
    program_pbs1.title      = chip::CharSpan::fromCharString("PBS Title1");
    program_pbs1.subtitle   = MakeOptional(chip::CharSpan::fromCharString("My Program Subtitle1"));
    program_pbs1.startTime  = 0;
    program_pbs1.endTime    = 30 * 60;

    mPrograms.push_back(program_pbs1);

    ProgramType program2;
    program2.identifier = chip::CharSpan::fromCharString("progid-abc2");
    program2.channel    = abc;
    program2.title      = chip::CharSpan::fromCharString("My Program Title2");
    program2.subtitle   = MakeOptional(chip::CharSpan::fromCharString("My Program Subtitle2"));
    program2.startTime  = 30 * 60;
    program2.endTime    = 60 * 60;

    mPrograms.push_back(program2);

    ProgramType program3;
    program3.identifier = chip::CharSpan::fromCharString("progid-abc3");
    program3.channel    = abc;
    program3.title      = chip::CharSpan::fromCharString("My Program Title3");
    program3.subtitle   = MakeOptional(chip::CharSpan::fromCharString("My Program Subtitle3"));
    program3.startTime  = 0;
    program3.endTime    = 60 * 60;

    mPrograms.push_back(program3);
}

CHIP_ERROR ChannelManager::HandleGetChannelList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (auto const & channel : ChannelManager().mChannels)
        {
            ReturnErrorOnFailure(encoder.Encode(channel));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ChannelManager::HandleGetLineup(AttributeValueEncoder & aEncoder)
{
    LineupInfoType lineup;
    lineup.operatorName   = chip::CharSpan::fromCharString("Comcast");
    lineup.lineupName     = MakeOptional(chip::CharSpan::fromCharString("Comcast King County"));
    lineup.postalCode     = MakeOptional(chip::CharSpan::fromCharString("98052"));
    lineup.lineupInfoType = LineupInfoTypeEnum::kMso;

    return aEncoder.Encode(lineup);
}

CHIP_ERROR ChannelManager::HandleGetCurrentChannel(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mCurrentChannel);
}

bool ChannelManager::isChannelMatched(const ChannelInfoType & channel, const chip::CharSpan & match)
{
    StringBuilder<16> channelNum;
    channelNum.AddFormat("%d.%d", channel.majorNumber, channel.minorNumber);

    auto isMatch = [&match](const Optional<chip::CharSpan> & a) { return a.HasValue() && a.Value().data_equal(match); };

    return isMatch(channel.name) || isMatch(channel.affiliateCallSign) || isMatch(channel.callSign) ||
        match.data_equal(chip::CharSpan::fromCharString(channelNum.c_str()));
}

void ChannelManager::HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match)
{
    int iMatchedChannel = -1;
    ChangeChannelResponseType response;

    for (uint16_t i = 0; i < mChannels.size(); i++)
    {
        // verify if CharSpan matches channel name
        // or callSign or affiliateCallSign or majorNumber.minorNumber
        if (isChannelMatched(mChannels[i], match))
        {
            if (iMatchedChannel != -1)
            {
                // Error: Found multiple matches
                response.status = StatusEnum::kMultipleMatches;
                helper.Success(response);
                return;
            }
            iMatchedChannel = i;
        }
    }

    if (iMatchedChannel == -1)
    {
        // Error: Found no match
        response.status = StatusEnum::kNoMatches;
        helper.Success(response);
    }
    else
    {
        response.status      = StatusEnum::kSuccess;
        response.data        = chip::MakeOptional(CharSpan::fromCharString("data response"));
        mCurrentChannel      = mChannels[iMatchedChannel];
        mCurrentChannelIndex = iMatchedChannel;
        helper.Success(response);
    }
}

bool ChannelManager::HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber)
{
    uint16_t index = 0;
    for (auto const & channel : mChannels)
    {
        // verify if major & minor matches one of the channel from the list
        if (channel.minorNumber == minorNumber && channel.majorNumber == majorNumber)
        {
            // verify if channel changed by comparing values of current channel with the requested channel
            if (channel.minorNumber != mCurrentChannel.minorNumber || channel.majorNumber != mCurrentChannel.majorNumber)
            {
                mCurrentChannelIndex = index;
                mCurrentChannel      = channel;
                return true;
            }
        }
        index++;
    }
    return false;
}

bool ChannelManager::HandleSkipChannel(const int16_t & count)
{
    int32_t newChannelIndex = static_cast<int32_t>(count) + static_cast<int32_t>(mCurrentChannelIndex);
    uint16_t channelsSize   = static_cast<uint16_t>(mChannels.size());

    // handle newChannelIndex out of range.
    newChannelIndex = newChannelIndex % channelsSize;

    if (newChannelIndex < 0)
    {
        newChannelIndex = newChannelIndex + channelsSize;
    }

    mCurrentChannelIndex = static_cast<uint16_t>(newChannelIndex);
    mCurrentChannel      = mChannels[mCurrentChannelIndex];
    return true;
}

void ChannelManager::HandleGetProgramGuide(CommandResponseHelper<ProgramGuideResponseType> & helper,
                                           const chip::Optional<uint32_t> & startTime, const chip::Optional<uint32_t> & endTime,
                                           const chip::Optional<DataModel::DecodableList<ChannelInfoType>> & channelList,
                                           const chip::Optional<PageTokenType> & pageToken,
                                           const chip::Optional<chip::BitMask<RecordingFlagBitmap>> & recordingFlag,
                                           const chip::Optional<DataModel::DecodableList<AdditionalInfoType>> & externalIdList,
                                           const chip::Optional<chip::ByteSpan> & data)
{

    // 1. Decode received parameters
    // 2. Perform search
    // 3. Return results

    // PageTokenType paging;
    // paging.limit  = MakeOptional(static_cast<uint16_t>(10));
    // paging.after  = MakeOptional(chip::CharSpan::fromCharString("after-token"));
    // paging.before = MakeOptional(chip::CharSpan::fromCharString("before-token"));

    // ChannelPagingStructType channelPaging;
    // channelPaging.nextToken = MakeOptional<DataModel::Nullable<Structs::PageTokenStruct::Type>>(paging);

    std::vector<ProgramType> matches;
    for (auto const & program : mPrograms)
    {
        if (startTime.ValueOr(0) > program.startTime)
        {
            continue;
        }
        if (endTime.ValueOr(std::numeric_limits<uint32_t>::max()) < program.endTime)
        {
            continue;
        }
        if (channelList.HasValue())
        {
            auto iter     = channelList.Value().begin();
            bool match    = false;
            int listCount = 0;
            while (iter.Next() && !match)
            {
                listCount++;
                auto & channel = iter.GetValue();
                if (channel.minorNumber != program.channel.minorNumber || channel.majorNumber != program.channel.majorNumber)
                {
                    continue;
                }
                // this sample code does not currently check OTT
                match = true;
            }
            if (!match && listCount > 0)
            {
                continue;
            }
        }
        // this sample code does not currently filter on external id list
        matches.push_back(program);
    }

    ProgramGuideResponseType response;
    response.programList = DataModel::List<const ProgramType>(matches.data(), matches.size());
    helper.Success(response);
}

bool ChannelManager::HandleRecordProgram(const chip::CharSpan & programIdentifier, bool shouldRecordSeries,
                                         const DataModel::DecodableList<AdditionalInfo> & externalIdList,
                                         const chip::ByteSpan & data)
{
    // Start recording
    std::string idString(programIdentifier.data(), programIdentifier.size());
    for (auto & program : mPrograms)
    {
        std::string nextIdString(program.identifier.data(), program.identifier.size());
        if (nextIdString == idString)
        {
            program.recordingFlag = MakeOptional(chip::BitMask<RecordingFlagBitmap>(
                shouldRecordSeries ? RecordingFlagBitmap::kRecordSeries : RecordingFlagBitmap::kScheduled));
        }
    }

    return true;
}

bool ChannelManager::HandleCancelRecordProgram(const chip::CharSpan & programIdentifier, bool shouldRecordSeries,
                                               const DataModel::DecodableList<AdditionalInfo> & externalIdList,
                                               const chip::ByteSpan & data)
{
    // Cancel recording
    std::string idString(programIdentifier.data(), programIdentifier.size());
    for (auto & program : mPrograms)
    {
        std::string nextIdString(program.identifier.data(), program.identifier.size());
        if (nextIdString == idString)
        {
            program.recordingFlag = MakeOptional<chip::BitMask<RecordingFlagBitmap>>(0);
        }
    }
    return true;
}

uint32_t ChannelManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

uint16_t ChannelManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "ChannelManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
#endif // MATTER_DM_PLUGIN_CHANNEL_SERVER
