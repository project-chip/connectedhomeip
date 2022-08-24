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

#include "bridge_service.h"

#include "Backend.h"
#include "main.h"

namespace chip {
namespace rpc {

namespace {
const clusters::ClusterInfo * FindCluster(uint32_t id)
{
    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (id == cluster.id)
            return &cluster;
    }
    return nullptr;
}
} // namespace

::pw::Status Bridge::Add(const ::chip_rpc_bridge_AddDevice & request, ::chip_rpc_bridge_AddDeviceResponse & response)
{
    std::unique_ptr<DynamicDeviceImpl> pending;

    pending = std::make_unique<DynamicDeviceImpl>();
    pending->SetParentEndpointId(request.parent_endpoint);

    for (pb_size_t i = 0; i < request.clusters_count; i++)
    {
        const chip_rpc_bridge_Cluster & c = request.clusters[i];

        auto cluster = FindCluster(c.cluster_id);
        if (!cluster)
        {
            return pw::Status::InvalidArgument();
        }

        std::unique_ptr<CommonCluster> obj(cluster->ctor(::operator new(cluster->size)));
        DynamicAttributeList dynamic_attrs;
        auto attrs = obj->GetAllAttributes();
        for (auto & attr : attrs)
            dynamic_attrs.Add(attr);
        pending->AddCluster(std::move(obj), dynamic_attrs, nullptr, nullptr);
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
