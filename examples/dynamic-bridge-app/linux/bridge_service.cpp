/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bridge_service.h"

#include "Backend.h"
#include "main.h"

namespace chip {
namespace rpc {

::pw::Status Bridge::Add(const ::chip_rpc_bridge_AddDevice & request, ::chip_rpc_bridge_AddDeviceResponse & response)
{
    std::unique_ptr<DynamicDevice> pending;

    pending = std::make_unique<DynamicDevice>();
    pending->SetParentEndpointId(request.parent_endpoint);

    for (pb_size_t i = 0; i < request.clusters_count; i++)
    {
        const chip_rpc_bridge_Cluster & c = request.clusters[i];

        auto cluster = CreateCluster(c.cluster_id);
        if (!cluster)
        {
            return pw::Status::InvalidArgument();
        }

        pending->AddCluster(std::make_unique<DynamicCluster>(std::move(cluster)));
    }

    for (pb_size_t i = 0; i < request.device_types_count; i++)
    {
        EmberAfDeviceType devType = { (uint16_t) request.device_types[i].id, (uint8_t) request.device_types[i].version };
        pending->AddDeviceType(devType);
    }

    int ret = AddDevice(std::move(pending));
    if (ret < 0)
    {
        return pw::Status::Aborted();
    }
    response.id = ret;
    return pw::OkStatus();
}

::pw::Status Bridge::Remove(const ::chip_rpc_bridge_RemoveDevice & request, ::chip_rpc_bridge_Empty & response)
{
    if (!RemoveDeviceAt(request.id))
    {
        return pw::Status::NotFound();
    }

    return pw::OkStatus();
}

} // namespace rpc
} // namespace chip
