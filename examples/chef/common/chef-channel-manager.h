/*
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

#pragma once

#include <app/clusters/channel-server/channel-server.h>
#include <vector>

using namespace chip;
using namespace chip::app;

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ChannelDelegate           = chip::app::Clusters::Channel::Delegate;
using ChangeChannelResponseType = chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type;
using ChannelInfoType           = chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type;
using LineupInfoType            = chip::app::Clusters::Channel::Structs::LineupInfoStruct::Type;

class ChefChannelManager : public ChannelDelegate
{

public:
    ChefChannelManager();

    CHIP_ERROR HandleGetChannelList(app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleGetLineup(app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleGetCurrentChannel(app::AttributeValueEncoder & aEncoder);

    void HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper,
                                     const chip::CharSpan & match);
    bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber);
    bool HandleSkipChannel(const int16_t & count);

    static ChefChannelManager & Instance()
    {
        static ChefChannelManager instance;
        return instance;
    }

    // bool HasFeature(chip::EndpointId endpoint, Feature feature);
    uint32_t GetFeatureMap(chip::EndpointId endpoint);

    ~ChefChannelManager() = default;

protected:
    uint16_t mCurrentChannelIndex;
    ChannelInfoType mCurrentChannel;
    std::vector<ChannelInfoType> mChannels;

};