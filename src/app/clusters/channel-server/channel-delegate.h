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
    virtual std::list<chip::app::Clusters::Channel::Structs::ChannelInfo::Type> HandleGetChannelList() = 0;
    virtual chip::app::Clusters::Channel::Structs::LineupInfo::Type HandleGetLineup()                  = 0;
    virtual chip::app::Clusters::Channel::Structs::ChannelInfo::Type HandleGetCurrentChannel()         = 0;

    virtual Commands::ChangeChannelResponse::Type HandleChangeChannel(const chip::CharSpan & match)      = 0;
    virtual bool HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber) = 0;
    virtual bool HandleSkipChannel(const uint16_t & count)                                               = 0;

    virtual ~Delegate() = default;
};

} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip
