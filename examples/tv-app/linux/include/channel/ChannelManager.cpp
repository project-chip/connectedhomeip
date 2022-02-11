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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Channel;

CHIP_ERROR ChannelManager::HandleGetChannelList(AttributeValueEncoder & aEncoder)
{
    // TODO: Insert code here
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        int maximumVectorSize = 2;

        for (int i = 0; i < maximumVectorSize; ++i)
        {
            chip::app::Clusters::Channel::Structs::ChannelInfo::Type channelInfo;
            channelInfo.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("exampleASign"));
            channelInfo.callSign          = MakeOptional(chip::CharSpan::fromCharString("exampleCSign"));
            channelInfo.name              = MakeOptional(chip::CharSpan::fromCharString("exampleName"));
            channelInfo.majorNumber       = static_cast<uint8_t>(1 + i);
            channelInfo.minorNumber       = static_cast<uint16_t>(2 + i);

            ReturnErrorOnFailure(encoder.Encode(channelInfo));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ChannelManager::HandleGetLineup(AttributeValueEncoder & aEncoder)
{
    chip::app::Clusters::Channel::Structs::LineupInfo::Type lineup;
    lineup.operatorName   = chip::CharSpan::fromCharString("operatorName");
    lineup.lineupName     = MakeOptional(chip::CharSpan::fromCharString("lineupName"));
    lineup.postalCode     = MakeOptional(chip::CharSpan::fromCharString("postalCode"));
    lineup.lineupInfoType = chip::app::Clusters::Channel::LineupInfoTypeEnum::kMso;

    return aEncoder.Encode(lineup);
}

CHIP_ERROR ChannelManager::HandleGetCurrentChannel(AttributeValueEncoder & aEncoder)
{
    chip::app::Clusters::Channel::Structs::ChannelInfo::Type currentChannel;
    currentChannel.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("exampleASign"));
    currentChannel.callSign          = MakeOptional(chip::CharSpan::fromCharString("exampleCSign"));
    currentChannel.name              = MakeOptional(chip::CharSpan::fromCharString("exampleName"));
    currentChannel.majorNumber       = 1;
    currentChannel.minorNumber       = 0;

    return aEncoder.Encode(currentChannel);
}

void ChannelManager::HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match)
{
    ChangeChannelResponseType response;
    response.channelMatch.majorNumber       = 1;
    response.channelMatch.minorNumber       = 0;
    response.channelMatch.name              = MakeOptional(chip::CharSpan::fromCharString("name"));
    response.channelMatch.callSign          = MakeOptional(chip::CharSpan::fromCharString("callSign"));
    response.channelMatch.affiliateCallSign = MakeOptional(chip::CharSpan::fromCharString("affiliateCallSign"));
    response.errorType                      = chip::app::Clusters::Channel::ErrorTypeEnum::kMultipleMatches;

    helper.Success(response);
}

bool ChannelManager::HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber)
{
    // TODO: Insert code here
    return true;
}
bool ChannelManager::HandleSkipChannel(const uint16_t & count)
{
    // TODO: Insert code here
    return true;
}
