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
