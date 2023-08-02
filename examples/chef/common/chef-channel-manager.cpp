/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/util/config.h>

using ChangeChannelResponseType = chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type;
using ChannelInfoType           = chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type;
using LineupInfoType            = chip::app::Clusters::Channel::Structs::LineupInfoStruct::Type;
// Include Channel Cluster Server callbacks only when the server is enabled
#ifdef EMBER_AF_PLUGIN_CHANNEL_SERVER
#include <chef-channel-manager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Channel;
using namespace chip::Uint8;

ChefChannelManager::ChefChannelManager()
{
    ChannelInfoType abc;
    abc.affiliateCallSign       = MakeOptional(chip::CharSpan::fromCharString("KAAL"));
    abc.callSign                = MakeOptional(chip::CharSpan::fromCharString("KAAL-TV"));
    abc.name                    = MakeOptional(chip::CharSpan::fromCharString("ABC"));
    abc.majorNumber             = static_cast<uint8_t>(6);
    abc.minorNumber             = static_cast<uint16_t>(0);
    mChannels[mTotalChannels++] = abc;

    ChannelInfoType pbs;
    pbs.affiliateCallSign       = MakeOptional(chip::CharSpan::fromCharString("KCTS"));
    pbs.callSign                = MakeOptional(chip::CharSpan::fromCharString("KCTS-TV"));
    pbs.name                    = MakeOptional(chip::CharSpan::fromCharString("PBS"));
    pbs.majorNumber             = static_cast<uint8_t>(9);
    pbs.minorNumber             = static_cast<uint16_t>(1);
    mChannels[mTotalChannels++] = pbs;

    ChannelInfoType pbsKids;
    pbsKids.affiliateCallSign   = MakeOptional(chip::CharSpan::fromCharString("KCTS"));
    pbsKids.callSign            = MakeOptional(chip::CharSpan::fromCharString("KCTS-TV"));
    pbsKids.name                = MakeOptional(chip::CharSpan::fromCharString("PBS Kids"));
    pbsKids.majorNumber         = static_cast<uint8_t>(9);
    pbsKids.minorNumber         = static_cast<uint16_t>(2);
    mChannels[mTotalChannels++] = pbsKids;

    ChannelInfoType worldChannel;
    worldChannel.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("KCTS"));
    worldChannel.callSign          = MakeOptional(chip::CharSpan::fromCharString("KCTS-TV"));
    worldChannel.name              = MakeOptional(chip::CharSpan::fromCharString("World Channel"));
    worldChannel.majorNumber       = static_cast<uint8_t>(9);
    worldChannel.minorNumber       = static_cast<uint16_t>(3);
    mChannels[mTotalChannels++]    = worldChannel;
}

static bool isChannelMatched(const ChannelInfoType & channel, const CharSpan & match)
{
    if (channel.name.HasValue() && channel.name.Value().data_equal(match))
    {
        return true;
    }

    if (channel.affiliateCallSign.HasValue() && channel.affiliateCallSign.Value().data_equal(match))
    {
        return true;
    }

    if (channel.callSign.HasValue() && channel.callSign.Value().data_equal(match))
    {
        return true;
    }

    StringBuilder<32> nr;
    nr.AddFormat("%d.%d", channel.majorNumber, channel.minorNumber);
    return match.data_equal(CharSpan::fromCharString(nr.c_str()));
}

CHIP_ERROR ChefChannelManager::HandleGetChannelList(app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        int index = 0;
        for (auto const & channel : ChefChannelManager().mChannels)
        {
            ReturnErrorOnFailure(encoder.Encode(channel));
            index++;
            if (index >= ChefChannelManager().mTotalChannels)
                break;
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ChefChannelManager::HandleGetLineup(app::AttributeValueEncoder & aEncoder)
{
    LineupInfoType lineup;
    lineup.operatorName   = chip::CharSpan::fromCharString("Comcast");
    lineup.lineupName     = MakeOptional(chip::CharSpan::fromCharString("Comcast King County"));
    lineup.postalCode     = MakeOptional(chip::CharSpan::fromCharString("98052"));
    lineup.lineupInfoType = chip::app::Clusters::Channel::LineupInfoTypeEnum::kMso;

    return aEncoder.Encode(lineup);
}

CHIP_ERROR ChefChannelManager::HandleGetCurrentChannel(app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mChannels[mCurrentChannelIndex]);
}

void ChefChannelManager::HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper,
                                             const chip::CharSpan & match)
{
    std::array<ChannelInfoType, kMaxChannels> matchedChannels;

    uint16_t index                = 0;
    uint16_t totalMatchedChannels = 0;
    for (auto const & channel : mChannels)
    {
        // verify if CharSpan matches channel name
        // or callSign or affiliateCallSign or majorNumber.minorNumber
        if (isChannelMatched(channel, match))
        {
            matchedChannels[totalMatchedChannels++] = (channel);
        }
        else if (totalMatchedChannels == 0)
        {
            // "index" is only used when we end up with totalMatchedChannels == 1.
            // In that case, we want it to be the number of non-matching channels we saw before
            // the matching one.
            index++;
        }
    }

    ChangeChannelResponseType response;

    // Error: Found multiple matches
    if (totalMatchedChannels > 1)
    {
        response.status = chip::app::Clusters::Channel::ChannelStatusEnum::kMultipleMatches;
        helper.Success(response);
    }
    else if (totalMatchedChannels == 0)
    {
        // Error: Found no match
        response.status = chip::app::Clusters::Channel::ChannelStatusEnum::kNoMatches;
        helper.Success(response);
    }
    else
    {
        response.status      = chip::app::Clusters::Channel::ChannelStatusEnum::kSuccess;
        response.data        = chip::MakeOptional(CharSpan::fromCharString("data response"));
        mCurrentChannelIndex = index;
        helper.Success(response);
    }
}

bool ChefChannelManager::HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber)
{
    bool channelChanged = false;
    uint16_t index      = 0;
    for (auto const & channel : mChannels)
    {

        // verify if major & minor matches one of the channel from the list
        if (channel.minorNumber == minorNumber && channel.majorNumber == majorNumber)
        {
            // verify if channel changed by comparing values of current channel with the requested channel
            if (channel.minorNumber != mChannels[mCurrentChannelIndex].minorNumber ||
                channel.majorNumber != mChannels[mCurrentChannelIndex].majorNumber)
            {
                channelChanged       = true;
                mCurrentChannelIndex = index;
            }

            // return since we've already found the unique matched channel
            return channelChanged;
        }
        index++;
        if (index >= mTotalChannels)
            break;
    }
    return channelChanged;
}

bool ChefChannelManager::HandleSkipChannel(const int16_t & count)
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
    return true;
}

uint32_t ChefChannelManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint > EMBER_AF_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return 0;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

#endif /* EMBER_AF_PLUGIN_CHANNEL_SERVER */
