/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "locking_service/locking_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Locking final : public pw_rpc::nanopb::Locking::Service<Locking>
{
public:
    virtual ~Locking() = default;

    virtual pw::Status Set(const chip_rpc_LockingState & request, pw_protobuf_Empty & response)
    {
        bool locked = request.locked;
        DeviceLayer::StackLock lock;
        RETURN_STATUS_IF_NOT_OK(app::Clusters::OnOff::Attributes::OnOff::Set(kEndpoint, locked));
        return pw::OkStatus();
    }

    virtual pw::Status Get(const pw_protobuf_Empty & request, chip_rpc_LockingState & response)
    {
        bool locked;
        DeviceLayer::StackLock lock;
        RETURN_STATUS_IF_NOT_OK(app::Clusters::OnOff::Attributes::OnOff::Get(kEndpoint, &locked));
        response.locked = locked;
        return pw::OkStatus();
    }

private:
    static constexpr EndpointId kEndpoint = 1;
};

} // namespace rpc
} // namespace chip
