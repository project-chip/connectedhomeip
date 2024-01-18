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

class ChefChannelManager : public chip::app::Clusters::Channel::Delegate
{

public:
    ChefChannelManager();

    CHIP_ERROR HandleGetChannelList(chip::app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleGetLineup(chip::app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleGetCurrentChannel(chip::app::AttributeValueEncoder & aEncoder);

    void HandleChangeChannel(
        chip::app::CommandResponseHelper<chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type> & helper,
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
    static constexpr size_t kMaxChannels = 10;
    uint16_t mCurrentChannelIndex{ 0 };
    uint16_t mTotalChannels{ 0 };
    std::array<chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type, kMaxChannels> mChannels;
};
