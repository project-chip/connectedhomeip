/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
