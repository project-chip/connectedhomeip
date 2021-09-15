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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>

namespace chip {
namespace rpc {

class Locking final : public generated::Locking<Locking>
{
public:
    virtual ~Locking() = default;

    virtual pw::Status Set(ServerContext &, const chip_rpc_LockingState & request, pw_protobuf_Empty & response)
    {
        uint8_t locked = request.locked;
        RETURN_STATUS_IF_NOT_OK(emberAfWriteServerAttribute(kEndpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, &locked,
                                                            ZCL_BOOLEAN_ATTRIBUTE_TYPE));
        return pw::OkStatus();
    }

    virtual pw::Status Get(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_LockingState & response)
    {
        uint8_t locked;
        RETURN_STATUS_IF_NOT_OK(
            emberAfReadServerAttribute(kEndpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, &locked, sizeof(locked)));
        response.locked = locked;
        return pw::OkStatus();
    }

private:
    static constexpr EndpointId kEndpoint = 1;
};

} // namespace rpc
} // namespace chip
