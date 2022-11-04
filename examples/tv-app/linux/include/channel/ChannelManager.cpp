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

#include "ChannelManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
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
}

CHIP_ERROR ChannelManager::HandleGetChannelList(AttributeValueEncoder & aEncoder)
{
    // TODO: Insert code here
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
    lineup.lineupInfoType = chip::app::Clusters::Channel::LineupInfoTypeEnum::kMso;

    return aEncoder.Encode(lineup);
}

CHIP_ERROR ChannelManager::HandleGetCurrentChannel(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mCurrentChannel);
}

bool isChannelMatched(const ChannelInfoType & channel, const CharSpan & match)
{
    char number[32];
    sprintf(number, "%d.%d", channel.majorNumber, channel.minorNumber);
    bool nameMatch = channel.name.HasValue() ? channel.name.Value().data_equal(match) : false;
    bool affiliateCallSignMatch =
        channel.affiliateCallSign.HasValue() ? channel.affiliateCallSign.Value().data_equal(match) : false;
    bool callSignMatch = channel.callSign.HasValue() ? channel.callSign.Value().data_equal(match) : false;
    bool numberMatch   = match.data_equal(chip::CharSpan::fromCharString(number));

    return affiliateCallSignMatch || callSignMatch || nameMatch || numberMatch;
}

void ChannelManager::HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match)
{
    std::vector<ChannelInfoType> matchedChannels;
    uint16_t index = 0;
    for (auto const & channel : mChannels)
    {
        index++;
        // verify if CharSpan matches channel name
        // or callSign or affiliateCallSign or majorNumber.minorNumber
        if (isChannelMatched(channel, match))
        {
            matchedChannels.push_back(channel);
        }
    }

    ChangeChannelResponseType response;

    // Error: Found multiple matches
    if (matchedChannels.size() > 1)
    {
        response.status = chip::app::Clusters::Channel::ChannelStatusEnum::kMultipleMatches;
        helper.Success(response);
    }
    else if (matchedChannels.size() == 0)
    {
        // Error: Found no match
        response.status = chip::app::Clusters::Channel::ChannelStatusEnum::kNoMatches;
        helper.Success(response);
    }
    else
    {
        response.status      = chip::app::Clusters::Channel::ChannelStatusEnum::kSuccess;
        response.data        = chip::MakeOptional(CharSpan::fromCharString("data response"));
        mCurrentChannel      = matchedChannels[0];
        mCurrentChannelIndex = index;
        helper.Success(response);
    }
}

bool ChannelManager::HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber)
{
    // TODO: Insert code here
    bool channelChanged = false;
    uint16_t index      = 0;
    for (auto const & channel : mChannels)
    {
        index++;
        // verify if major & minor matches one of the channel from the list
        if (channel.minorNumber == minorNumber && channel.majorNumber == majorNumber)
        {
            // verify if channel changed by comparing values of current channel with the requested channel
            if (channel.minorNumber != mCurrentChannel.minorNumber || channel.majorNumber != mCurrentChannel.majorNumber)
            {
                channelChanged       = true;
                mCurrentChannelIndex = index;
                mCurrentChannel      = channel;
            }
        }
    }
    return channelChanged;
}

bool ChannelManager::HandleSkipChannel(const uint16_t & count)
{
    // TODO: Insert code here
    uint16_t newChannelIndex = static_cast<uint16_t>((count + mCurrentChannelIndex) % mChannels.size());
    mCurrentChannelIndex     = newChannelIndex;
    mCurrentChannel          = mChannels[mCurrentChannelIndex];
    return true;
}

uint32_t ChannelManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= EMBER_AF_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
