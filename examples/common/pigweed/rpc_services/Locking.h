/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
