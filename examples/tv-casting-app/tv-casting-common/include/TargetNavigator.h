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
class NavigateTargetCommand
    : public MediaCommandBase<chip::app::Clusters::TargetNavigator::Commands::NavigateTarget::Type,
                              chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType>
{
public:
    NavigateTargetCommand() : MediaCommandBase(chip::app::Clusters::TargetNavigator::Id) {}

    CHIP_ERROR Invoke(const uint8_t target, const chip::Optional<chip::CharSpan> data,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

// SUBSCRIBER CLASSES
class TargetListSubscriber : public MediaSubscriptionBase<chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo>
{
public:
    TargetListSubscriber() : MediaSubscriptionBase(chip::app::Clusters::TargetNavigator::Id) {}
};

class CurrentTargetSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo>
{
public:
    CurrentTargetSubscriber() : MediaSubscriptionBase(chip::app::Clusters::TargetNavigator::Id) {}
};
