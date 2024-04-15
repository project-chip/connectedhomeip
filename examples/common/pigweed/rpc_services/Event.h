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
    ::pw::Status Set( const ::chip_rpc_EventSetRequest& request, ::chip_rpc_EventSetResponse& response)
    {
printf("\033[41m %s, %d, request.endpoint_id=%d, request.cluster_id=%d, request.event_playload=%s \033[0m \n", __func__, __LINE__, request.endpoint_id, request.cluster_id, request.event_payload);

        mEventsSubscriber(request.endpoint_id, request.cluster_id, request.event_payload);

        return pw::OkStatus();
    }

    ::pw::Status Get( const ::chip_rpc_EventGetRequest& request, ::chip_rpc_EventGetResponse& response)
    {
        EndpointId endpointId = request.endpoint_id;
        // uint32_t event_id = 1; // TBD

        {
            DeviceLayer::StackLock lock;
            // TBD: Get latest event number
            // RETURN_STATUS_IF_NOT_OK(app::Clusters::Switch::Attributes::StateValue::Get(endpointId, &state_value));
        }

        response.event_id = endpointId;
        return pw::OkStatus();
    }

    using EventsSubscriber = void (*)(EndpointId endpointId, ClusterId clusterId, std::string events); 

    void RegisterEventsSubscriber(EventsSubscriber subscriber) { mEventsSubscriber = subscriber; }

private:
    EventsSubscriber mEventsSubscriber;

};

} // namespace rpc
} // namespace chip
