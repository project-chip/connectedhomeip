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
#include <jni.h>

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ChannelDelegate           = chip::app::Clusters::Channel::Delegate;
using ChangeChannelResponseType = chip::app::Clusters::Channel::Commands::ChangeChannelResponse::Type;

class ChannelManager : public ChannelDelegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    CHIP_ERROR HandleGetChannelList(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetLineup(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetCurrentChannel(AttributeValueEncoder & aEncoder) override;

    void HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match) override;
    bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) override;
    bool HandleSkipChannel(const uint16_t & count) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

private:
    jobject mChannelManagerObject      = nullptr;
    jmethodID mGetChannelListMethod    = nullptr;
    jmethodID mGetLineupMethod         = nullptr;
    jmethodID mGetCurrentChannelMethod = nullptr;

    jmethodID mChangeChannelMethod         = nullptr;
    jmethodID mChangeChannelByNumberMethod = nullptr;
    jmethodID mSkipChannelMethod           = nullptr;

    // TODO: set this based upon meta data from app
    uint32_t mDynamicEndpointFeatureMap = 3;
};
