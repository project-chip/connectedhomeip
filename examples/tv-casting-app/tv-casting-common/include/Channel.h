/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaCommandBase.h"
#include "MediaSubscriptionBase.h"

#include <controller/CHIPCluster.h>
#include <functional>

#include <app-common/zap-generated/cluster-objects.h>

// COMMAND CLASSES
class ChangeChannelCommand : public MediaCommandBase<chip::app::Clusters::Channel::Commands::ChangeChannel::Type,
                                                     chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType>
{
public:
    ChangeChannelCommand() : MediaCommandBase(chip::app::Clusters::Channel::Id) {}

    CHIP_ERROR Invoke(const chip::CharSpan & match, std::function<void(CHIP_ERROR)> responseCallback);
};

// SUBSCRIBER CLASSES
class LineupSubscriber : public MediaSubscriptionBase<chip::app::Clusters::Channel::Attributes::Lineup::TypeInfo>
{
public:
    LineupSubscriber() : MediaSubscriptionBase(chip::app::Clusters::Channel::Id) {}
};
