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
using namespace chip::app::Clusters::Channel;

std::list<chip::app::Clusters::Channel::Structs::ChannelInfo::Type> ChannelManager::HandleGetChannelList()
{
    std::list<Structs::ChannelInfo::Type> list;
    // TODO: Insert code here
    int maximumVectorSize = 2;

    for (int i = 0; i < maximumVectorSize; ++i)
    {
        chip::app::Clusters::Channel::Structs::ChannelInfo::Type channelInfo;
        channelInfo.affiliateCallSign = chip::CharSpan("exampleASign", strlen("exampleASign"));
        channelInfo.callSign          = chip::CharSpan("exampleCSign", strlen("exampleCSign"));
        channelInfo.name              = chip::CharSpan("exampleName", strlen("exampleName"));
        channelInfo.majorNumber       = static_cast<uint8_t>(1 + i);
        channelInfo.minorNumber       = static_cast<uint16_t>(2 + i);
        list.push_back(channelInfo);
    }
    return list;
}

chip::app::Clusters::Channel::Structs::LineupInfo::Type ChannelManager::HandleGetLineup()
{
    chip::app::Clusters::Channel::Structs::LineupInfo::Type lineup;
    lineup.operatorName   = chip::CharSpan("operatorName", strlen("operatorName"));
    lineup.lineupName     = chip::CharSpan("lineupName", strlen("lineupName"));
    lineup.postalCode     = chip::CharSpan("postalCode", strlen("postalCode"));
    lineup.lineupInfoType = chip::app::Clusters::Channel::LineupInfoTypeEnum::kMso;
    return lineup;
}

chip::app::Clusters::Channel::Structs::ChannelInfo::Type ChannelManager::HandleGetCurrentChannel()
{
    chip::app::Clusters::Channel::Structs::ChannelInfo::Type currentChannel;
    currentChannel.affiliateCallSign = chip::CharSpan("exampleASign", strlen("exampleASign"));
    currentChannel.callSign          = chip::CharSpan("exampleCSign", strlen("exampleCSign"));
    currentChannel.name              = chip::CharSpan("exampleName", strlen("exampleName"));
    currentChannel.majorNumber       = 1;
    currentChannel.minorNumber       = 0;
    return currentChannel;
}

Commands::ChangeChannelResponse::Type ChannelManager::HandleChangeChannel(const chip::CharSpan & match)
{
    Commands::ChangeChannelResponse::Type response;
    response.channelMatch.majorNumber       = 1;
    response.channelMatch.minorNumber       = 0;
    response.channelMatch.name              = chip::CharSpan("name", strlen("name"));
    response.channelMatch.callSign          = chip::CharSpan("callSign", strlen("callSign"));
    response.channelMatch.affiliateCallSign = chip::CharSpan("affiliateCallSign", strlen("affiliateCallSign"));
    response.errorType                      = chip::app::Clusters::Channel::ErrorTypeEnum::kMultipleMatches;
    return response;
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
