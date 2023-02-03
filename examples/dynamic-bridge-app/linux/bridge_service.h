/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "bridge_service/bridge_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Bridge : public bridge::pw_rpc::nanopb::Bridge::Service<Bridge>
{
public:
    Bridge() = default;

    virtual ~Bridge() = default;

    ::pw::Status Add(const ::chip_rpc_bridge_AddDevice & request, ::chip_rpc_bridge_AddDeviceResponse & response);

    ::pw::Status Remove(const ::chip_rpc_bridge_RemoveDevice & request, ::chip_rpc_bridge_Empty & response);
};

} // namespace rpc
} // namespace chip
