/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/channel-server/channel-server.h>
#include <vector>

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ChannelDelegate           = chip::app::Clusters::Channel::Delegate;
using ChangeChannelResponseType = chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type;
using ChannelInfoType           = chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type;
using LineupInfoType            = chip::app::Clusters::Channel::Structs::LineupInfoStruct::Type;

class ChannelManager : public ChannelDelegate
{
public:
    ChannelManager();

    CHIP_ERROR HandleGetChannelList(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetLineup(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetCurrentChannel(AttributeValueEncoder & aEncoder) override;

    void HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match) override;
    bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) override;
    bool HandleSkipChannel(const int16_t & count) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

protected:
    uint16_t mCurrentChannelIndex;
    ChannelInfoType mCurrentChannel;
    std::vector<ChannelInfoType> mChannels;

private:
    // TODO: set this based upon meta data from app
    uint32_t mDynamicEndpointFeatureMap = 3;
};
