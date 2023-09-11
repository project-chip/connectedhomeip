/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/target-navigator-server/target-navigator-server.h>

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using TargetNavigatorDelegate    = chip::app::Clusters::TargetNavigator::Delegate;
using TargetInfoType             = chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::Type;
using NavigateTargetResponseType = chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Type;

class TargetNavigatorManager : public TargetNavigatorDelegate
{
public:
    TargetNavigatorManager() : TargetNavigatorManager({ "exampleName", "exampleName" }, kNoCurrentTarget){};
    TargetNavigatorManager(std::list<std::string> targets, uint8_t currentTarget);

    CHIP_ERROR HandleGetTargetList(AttributeValueEncoder & aEncoder) override;
    uint8_t HandleGetCurrentTarget() override;
    void HandleNavigateTarget(CommandResponseHelper<NavigateTargetResponseType> & responser, const uint64_t & target,
                              const CharSpan & data) override;

protected:
    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    static const uint8_t kNoCurrentTarget = 0;
    std::list<std::string> mTargets;
    uint8_t mCurrentTarget;
};
