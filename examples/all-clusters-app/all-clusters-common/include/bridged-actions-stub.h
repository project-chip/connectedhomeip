/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/clusters/actions-server/actions-server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {
const uint16_t kActionListSize   = 3;
const uint16_t kEndpointListSize = 3;
class ActionsDelegateImpl : public Delegate
{
private:
    using ActionListStructType   = Structs::ActionStruct::Type;
    using EndpointListStructType = Structs::EndpointListStruct::Type;

    const ActionListStructType kActionList[kActionListSize] = {
        ActionListStructType{ .actionID          = 0,
                              .name              = CharSpan::fromCharString("TurnOnLight"),
                              .type              = ActionTypeEnum::kScene,
                              .endpointListID    = 0,
                              .supportedCommands = 0,
                              .state             = ActionStateEnum::kInactive },

        ActionListStructType{ .actionID          = 1,
                              .name              = CharSpan::fromCharString("TurnOffLight"),
                              .type              = ActionTypeEnum::kScene,
                              .endpointListID    = 1,
                              .supportedCommands = 0,
                              .state             = ActionStateEnum::kInactive },

        ActionListStructType{ .actionID          = 2,
                              .name              = CharSpan::fromCharString("ToggleLight"),
                              .type              = ActionTypeEnum::kScene,
                              .endpointListID    = 2,
                              .supportedCommands = 0,
                              .state             = ActionStateEnum::kInactive },
    };

    // Dummy endpoint list.
    const EndpointId firstEpList[1]  = { 0 };
    const EndpointId secondEpList[1] = { 0 };
    const EndpointId thirdEpList[1]  = { 0 };

    const EndpointListStructType kEndpointList[kEndpointListSize] = {
        EndpointListStructType{ .endpointListID = 0,
                                .name           = CharSpan::fromCharString("On"),
                                .type           = EndpointListTypeEnum::kOther,
                                .endpoints      = DataModel::List<const EndpointId>(firstEpList) },

        EndpointListStructType{ .endpointListID = 1,
                                .name           = CharSpan::fromCharString("Off"),
                                .type           = EndpointListTypeEnum::kOther,
                                .endpoints      = DataModel::List<const EndpointId>(secondEpList) },

        EndpointListStructType{ .endpointListID = 2,
                                .name           = CharSpan::fromCharString("Toggle"),
                                .type           = EndpointListTypeEnum::kOther,
                                .endpoints      = DataModel::List<const EndpointId>(thirdEpList) },
    };
    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionListStructType & action) override;
    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStructType & epList) override;
    CHIP_ERROR FindActionIdInActionList(uint16_t actionId) override;

    Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime, Optional<uint32_t> invokeId) override;
    Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
    Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
    Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
    Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
};
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
