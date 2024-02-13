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

#include "../../java/ContentAppAttributeDelegate.h"
#include <app/clusters/target-navigator-server/target-navigator-server.h>

using chip::CharSpan;
using chip::EndpointId;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using TargetNavigatorDelegate     = chip::app::Clusters::TargetNavigator::Delegate;
using TargetInfoType              = chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::Type;
using NavigateTargetResponseType  = chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Type;
using ContentAppAttributeDelegate = chip::AppPlatform::ContentAppAttributeDelegate;

class TargetNavigatorManager : public TargetNavigatorDelegate
{
public:
    TargetNavigatorManager() : TargetNavigatorManager(nullptr, { "exampleName", "exampleName" }, kNoCurrentTarget){};
    TargetNavigatorManager(ContentAppAttributeDelegate * attributeDelegate, std::list<std::string> targets, uint8_t currentTarget);

    CHIP_ERROR HandleGetTargetList(AttributeValueEncoder & aEncoder) override;
    uint8_t HandleGetCurrentTarget() override;
    void HandleNavigateTarget(CommandResponseHelper<NavigateTargetResponseType> & responser, const uint64_t & target,
                              const CharSpan & data) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

    void SetEndpointId(EndpointId epId) { mEndpointId = epId; };

protected:
    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    static const uint8_t kNoCurrentTarget = 0;
    std::list<std::string> mTargets;
    uint8_t mCurrentTarget;

    EndpointId mEndpointId;

    ContentAppAttributeDelegate * mAttributeDelegate;

private:
    // TODO: set this based upon meta data from app
    static constexpr uint16_t kClusterRevision = 2;
};
