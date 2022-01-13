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

#pragma once

#include <app/clusters/channel-server/channel-server.h>

class ChannelManager : public chip::app::Clusters::Channel::Delegate
{
public:
    virtual CHIP_ERROR HandleGetChannelList(chip::app::AttributeValueEncoder & aEncoder) override;
    virtual CHIP_ERROR HandleGetLineup(chip::app::AttributeValueEncoder & aEncoder) override;
    virtual CHIP_ERROR HandleGetCurrentChannel(chip::app::AttributeValueEncoder & aEncoder) override;

    virtual void HandleChangeChannel(
        const chip::CharSpan & match,
        chip::app::CommandResponseHelper<chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type> & responser) override;
    bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) override;
    bool HandleSkipChannel(const uint16_t & count) override;
};
