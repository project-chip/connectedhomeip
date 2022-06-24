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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace Channel {

/** @brief
 *    Defines methods for implementing application-specific logic for the Channel Cluster.
 */
class Delegate
{
public:
    virtual CHIP_ERROR HandleGetChannelList(app::AttributeValueEncoder & aEncoder)    = 0;
    virtual CHIP_ERROR HandleGetLineup(app::AttributeValueEncoder & aEncoder)         = 0;
    virtual CHIP_ERROR HandleGetCurrentChannel(app::AttributeValueEncoder & aEncoder) = 0;

    virtual void HandleChangeChannel(CommandResponseHelper<Commands::ChangeChannelResponse::Type> & helper,
                                     const chip::CharSpan & match)                                       = 0;
    virtual bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) = 0;
    virtual bool HandleSkipChannel(const uint16_t & count)                                               = 0;

    bool HasFeature(chip::EndpointId endpoint, ChannelFeature feature);
    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip
