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
#include "fabric_admin_service/fabric_admin_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class FabricAdmin : public pw_rpc::nanopb::FabricAdmin::Service<FabricAdmin>
{
public:
    virtual ~FabricAdmin() = default;

    virtual pw::Status OpenCommissioningWindow(const chip_rpc_DeviceCommissioningWindowInfo & request,
                                               chip_rpc_OperationStatus & response)
    {
        return pw::Status::Unimplemented();
    }

    virtual pw::Status CommissionNode(const chip_rpc_DeviceCommissioningInfo & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    virtual pw::Status KeepActive(const chip_rpc_KeepActiveParameters & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }
};

} // namespace rpc
} // namespace chip
