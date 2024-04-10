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
#include "event_service/event_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Event final : public pw_rpc::nanopb::Event::Service<Event>
{
public:
    virtual ~Event() = default;

    virtual pw::Status Set(const chip_rpc_EventSetRequest & request, chip_rpc_EventSetResponse & response)
    {
        EndpointId endpointId = request.endpoint_id;
        uint8_t newPosition   = 1 ; // to be parsed from request.event_payload

        EventNumber eventNumber;
        {
            DeviceLayer::StackLock lock;

            // Update attribute first, then emit SwitchLatched event only on success.
            RETURN_STATUS_IF_NOT_OK(app::Clusters::Switch::Attributes::CurrentPosition::Set(endpointId, newPosition));

            chip::app::Clusters::Switch::Events::SwitchLatched::Type event{ newPosition };
            RETURN_STATUS_IF_NOT_OK(app::LogEvent(event, endpointId, eventNumber));
        }

        response.event_number = static_cast<uint64_t>(eventNumber);
        return pw::OkStatus();
    }

    virtual pw::Status Get(const chip_rpc_EventGetRequest & request, chip_rpc_EventGetResponse & response)
    {
        EndpointId endpointId = request.endpoint_id;
        uint32 event_id = 1; // TBD

        {
            DeviceLayer::StackLock lock;
            // TBD: Get latest event number
            // RETURN_STATUS_IF_NOT_OK(app::Clusters::Switch::Attributes::StateValue::Get(endpointId, &state_value));
        }

        response.state.event_id = event_id;
        return pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
