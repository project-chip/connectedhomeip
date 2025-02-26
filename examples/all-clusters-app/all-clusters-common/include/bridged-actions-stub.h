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
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {
class ActionsDelegateImpl : public Delegate
{
private:
    std::vector<ActionStructStorage> kActionList = {
        ActionStructStorage(0, CharSpan::fromCharString("TurnOnLight"), ActionTypeEnum::kScene, 0, 0, ActionStateEnum::kInactive),
        ActionStructStorage(1, CharSpan::fromCharString("TurnOffLight"), ActionTypeEnum::kScene, 1, 0, ActionStateEnum::kInactive),
        ActionStructStorage(2, CharSpan::fromCharString("ToggleLight"), ActionTypeEnum::kScene, 2, 0, ActionStateEnum::kInactive)
    };

    std::vector<EndpointId> firstEpList  = { 0 };
    std::vector<EndpointId> secondEpList = { 0, 1 };
    std::vector<EndpointId> thirdEpList  = { 1, 2, 3 };

    std::vector<EndpointListStorage> kEndpointList = {
        EndpointListStorage(0, CharSpan::fromCharString("On"), EndpointListTypeEnum::kOther,
                            DataModel::List<const EndpointId>(firstEpList.data(), firstEpList.size())),
        EndpointListStorage(1, CharSpan::fromCharString("Off"), EndpointListTypeEnum::kOther,
                            DataModel::List<const EndpointId>(secondEpList.data(), secondEpList.size())),
        EndpointListStorage(2, CharSpan::fromCharString("Toggle"), EndpointListTypeEnum::kOther,
                            DataModel::List<const EndpointId>(thirdEpList.data(), thirdEpList.size()))
    };

    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override;
    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override;
    bool HaveActionWithId(uint16_t actionId, uint16_t & actionIndex) override;

    Protocols::InteractionModel::Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                                          Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                       Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Protocols::InteractionModel::Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                        Optional<uint32_t> invokeId) override;
};
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
