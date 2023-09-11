/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
