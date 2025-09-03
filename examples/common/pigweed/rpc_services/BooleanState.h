/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "app/util/attribute-storage.h"
#include "boolean_state_service/boolean_state_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/PlatformManager.h>

// TODO: Ideally we should not depend on the codegen integration
// It would be best if we could use generic cluster API instead
#include <app/clusters/boolean-state-server/CodegenIntegration.h>

namespace chip {
namespace rpc {

class BooleanState : public pw_rpc::nanopb::BooleanState::Service<BooleanState>
{
public:
    virtual ~BooleanState() = default;

    virtual pw::Status Set(const chip_rpc_BooleanStateSetRequest & request, chip_rpc_BooleanStateSetResponse & response)
    {
        EndpointId endpointId = request.endpoint_id;
        bool newState         = request.state_value;

        EventNumber eventNumber;
        {
            DeviceLayer::StackLock lock;

            auto booleanState = app::Clusters::BooleanState::GetClusterForEndpointIndex(endpointId);
            VerifyOrReturnError(booleanState != nullptr, pw::Status::InvalidArgument());
            RETURN_STATUS_IF_NOT_OK(booleanState->SetStateValue(newState, eventNumber));
        }

        response.event_number = static_cast<uint64_t>(eventNumber);
        return pw::OkStatus();
    }

    virtual pw::Status Get(const chip_rpc_BooleanStateGetRequest & request, chip_rpc_BooleanStateGetResponse & response)
    {
        EndpointId endpointId = request.endpoint_id;
        bool state_value{ false };

        {
            DeviceLayer::StackLock lock;

            auto booleanState = app::Clusters::BooleanState::GetClusterForEndpointIndex(endpointId);
            VerifyOrReturnError(booleanState != nullptr, pw::Status::InvalidArgument());
            state_value = booleanState->GetStateValue();
        }

        response.state.state_value = state_value;
        return pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
