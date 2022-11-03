/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app/util/attribute-storage.h"
#include "bridge_service/bridge_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
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
